#pragma once

#include <surrealgpu/vulkanobjects.h>

class VulkanRenderDevice;

class CommandBufferManager
{
public:
	CommandBufferManager(VulkanRenderDevice* renderer);
	~CommandBufferManager();

	void WaitForTransfer();
	// deferWait: the frame is submitted and its end (the wait for the GPU and everything that waits on it: command pool, delete
	// list, descriptor sets, upload and vertex buffers) is left to FinishFrame, which the first thing of the next frame that
	// touches those calls (GetDrawCommands, GetTransferCommands, ReserveVertices, a texture upload). The CPU then prepares the
	// next frame while the GPU draws this one, instead of idling until it is done.
	void SubmitCommands(bool present, int presentWidth, int presentHeight, bool presentFullscreen, bool deferWait = false);
	void FinishFrame();
	VulkanCommandBuffer* GetTransferCommands();
	VulkanCommandBuffer* GetDrawCommands();
	VulkanCommandBuffer* GetDrawCommandsNoBegin() { return DrawCommands.get(); }
	void DeleteFrameObjects();

	struct DeleteList
	{
		std::vector<std::unique_ptr<VulkanImage>> images;
		std::vector<std::unique_ptr<VulkanImageView>> imageViews;
		std::vector<std::unique_ptr<VulkanBuffer>> buffers;
		std::vector<std::unique_ptr<VulkanDescriptorSet>> descriptors;
	};
	std::unique_ptr<DeleteList> FrameDeleteList;

	std::shared_ptr<VulkanSwapChain> SwapChain;
	int PresentImageIndex = -1;
	bool UsingVsync = false;
	bool UsingHdr = false;

private:
	VulkanRenderDevice* renderer = nullptr;

	std::unique_ptr<VulkanSemaphore> ImageAvailableSemaphore;
	std::vector<std::unique_ptr<VulkanSemaphore>> RenderFinishedSemaphores;
	std::unique_ptr<VulkanSemaphore> TransferSemaphore;
	std::unique_ptr<VulkanFence> RenderFinishedFence;
	std::unique_ptr<VulkanCommandPool> CommandPool;
	std::unique_ptr<VulkanCommandBuffer> DrawCommands;
	std::unique_ptr<VulkanCommandBuffer> TransferCommands;

	bool FramePending = false;
	std::unique_ptr<VulkanCommandBuffer> PendingDrawCommands, PendingTransferCommands;
	std::unique_ptr<DeleteList> PendingDeleteList;
};
