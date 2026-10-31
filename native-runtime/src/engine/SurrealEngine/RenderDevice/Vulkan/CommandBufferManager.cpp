
#include "Precomp.h"
#include "CommandBufferManager.h"
#include "VulkanRenderDevice.h"
#include <surrealgpu/vulkanswapchain.h>

CommandBufferManager::CommandBufferManager(VulkanRenderDevice* renderer) : renderer(renderer)
{
	SwapChain = VulkanSwapChainBuilder()
		.Create(renderer->Device.get());

	ImageAvailableSemaphore = SemaphoreBuilder()
		.DebugName("ImageAvailableSemaphore")
		.Create(renderer->Device.get());

	RenderFinishedFence = FenceBuilder()
		.DebugName("RenderFinishedFence")
		.Create(renderer->Device.get());

	TransferSemaphore.reset(new VulkanSemaphore(renderer->Device.get()));

	CommandPool = CommandPoolBuilder()
		.QueueFamily(renderer->Device.get()->GraphicsFamily)
		.DebugName("CommandPool")
		.Create(renderer->Device.get());

	FrameDeleteList = std::make_unique<DeleteList>();
}

CommandBufferManager::~CommandBufferManager()
{
	FinishFrame();
	DeleteFrameObjects();
}

void CommandBufferManager::WaitForTransfer()
{
	FinishFrame();
	renderer->Uploads->SubmitUploads();

	if (TransferCommands)
	{
		TransferCommands->end();

		QueueSubmit()
			.AddCommandBuffer(TransferCommands.get())
			.Execute(renderer->Device.get(), renderer->Device.get()->GraphicsQueue, RenderFinishedFence.get());

		VkResult result = vkWaitForFences(renderer->Device.get()->device, 1, &RenderFinishedFence->fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
		if (result != VK_SUCCESS)
			throw std::runtime_error("vkWaitForFences failed");
		result = vkResetFences(renderer->Device.get()->device, 1, &RenderFinishedFence->fence);
		if (result != VK_SUCCESS)
			throw std::runtime_error("vkResetFences failed");

		TransferCommands.reset();
	}
}

void CommandBufferManager::SubmitCommands(bool present, int presentWidth, int presentHeight, bool presentFullscreen, bool deferWait)
{
	FinishFrame();
	renderer->Uploads->SubmitUploads();

	if (present)
	{
		if (SwapChain->Lost() || SwapChain->Width() != presentWidth || SwapChain->Height() != presentHeight || UsingVsync != renderer->UseVSync || UsingHdr != renderer->Hdr)
		{
			UsingVsync = renderer->UseVSync;
			UsingHdr = renderer->Hdr;
			renderer->Framebuffers->DestroySwapChainFramebuffers();
			if (SwapChain->SurfaceLost())
			{
				// The window was destroyed (Android: the app went to the background) and a new one exists now. While there
				// is no window yet (still in the background, or the screen being rebuilt) this frame is not shown: making a
				// surface for a window that has no native window crashed in the Vulkan loader (Redmi Note 10S, Mali-G76).
				SwapChain->ReleaseSwapchain();
				if (!renderer->RecreateSurface())
					present = false;
			}
			if (present)
			{
				SwapChain->Create(presentWidth, presentHeight, renderer->UseVSync ? 2 : 3, renderer->UseVSync, renderer->Hdr);
				renderer->Framebuffers->CreateSwapChainFramebuffers();

				RenderFinishedSemaphores.clear();
				for (int i = 0; i < SwapChain->ImageCount(); i++)
				{
					RenderFinishedSemaphores.push_back(SemaphoreBuilder()
						.DebugName("RenderFinishedSemaphore")
						.Create(renderer->Device.get()));
				}
			}
		}

		if (present)
		{
			PresentImageIndex = SwapChain->AcquireImage(ImageAvailableSemaphore.get());
			if (PresentImageIndex != -1)
			{
				renderer->DrawPresentTexture(presentWidth, presentHeight);
			}
		}
	}

	if (TransferCommands)
	{
		TransferCommands->end();

		QueueSubmit()
			.AddCommandBuffer(TransferCommands.get())
			.AddSignal(TransferSemaphore.get())
			.Execute(renderer->Device.get(), renderer->Device.get()->GraphicsQueue);
	}

	if (DrawCommands)
		DrawCommands->end();

	QueueSubmit submit;
	if (DrawCommands)
	{
		submit.AddCommandBuffer(DrawCommands.get());
	}
	if (TransferCommands)
	{
		submit.AddWait(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, TransferSemaphore.get());
	}
	if (present && PresentImageIndex != -1)
	{
		submit.AddWait(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, ImageAvailableSemaphore.get());
		submit.AddSignal(RenderFinishedSemaphores[PresentImageIndex].get());
	}
	submit.Execute(renderer->Device.get(), renderer->Device.get()->GraphicsQueue, RenderFinishedFence.get());

	if (present && PresentImageIndex != -1)
	{
		SwapChain->QueuePresent(PresentImageIndex, RenderFinishedSemaphores[PresentImageIndex].get());
	}

	if (deferWait)
	{
		// What the GPU is using stays alive until FinishFrame: the buffers, what is on the delete list
		PendingDrawCommands = std::move(DrawCommands);
		PendingTransferCommands = std::move(TransferCommands);
		PendingDeleteList = std::move(FrameDeleteList);
		FrameDeleteList = std::make_unique<DeleteList>();
		FramePending = true;
		return;
	}

	VkResult result = vkWaitForFences(renderer->Device.get()->device, 1, &RenderFinishedFence->fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
	if (result != VK_SUCCESS)
		throw std::runtime_error("vkWaitForFences failed");
	result = vkResetFences(renderer->Device.get()->device, 1, &RenderFinishedFence->fence);
	if (result != VK_SUCCESS)
		throw std::runtime_error("vkResetFences failed");

	DrawCommands.reset();
	TransferCommands.reset();
	DeleteFrameObjects();
	// Spike: Mali keeps command-stream memory until the pool is reset; recycle it every frame.
	vkResetCommandPool(renderer->Device.get()->device, CommandPool->pool, 0);
}

// The end of a frame submitted with deferWait: wait for the GPU, then let go of what it was using
void CommandBufferManager::FinishFrame()
{
	if (!FramePending)
		return;
	FramePending = false;

	VkResult result = vkWaitForFences(renderer->Device.get()->device, 1, &RenderFinishedFence->fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
	if (result != VK_SUCCESS)
		throw std::runtime_error("vkWaitForFences failed");
	result = vkResetFences(renderer->Device.get()->device, 1, &RenderFinishedFence->fence);
	if (result != VK_SUCCESS)
		throw std::runtime_error("vkResetFences failed");

	PendingDrawCommands.reset();
	PendingTransferCommands.reset();
	PendingDeleteList.reset();
	vkResetCommandPool(renderer->Device.get()->device, CommandPool->pool, 0);
	if (!renderer->Bindless)
		renderer->DescriptorSets->ResetFixedTextureSets();
}

VulkanCommandBuffer* CommandBufferManager::GetTransferCommands()
{
	FinishFrame();
	if (!TransferCommands)
	{
		TransferCommands = CommandPool->createBuffer();
		TransferCommands->begin();
	}
	return TransferCommands.get();
}

VulkanCommandBuffer* CommandBufferManager::GetDrawCommands()
{
	if (!DrawCommands)
	{
		FinishFrame();
		DrawCommands = CommandPool->createBuffer();
		DrawCommands->begin();
		// The frame's start was left for the first thing that records into it (VulkanRenderDevice::Lock)
		renderer->BeginSceneCommands();
	}
	return DrawCommands.get();
}

void CommandBufferManager::DeleteFrameObjects()
{
	FrameDeleteList = std::make_unique<DeleteList>();
}
