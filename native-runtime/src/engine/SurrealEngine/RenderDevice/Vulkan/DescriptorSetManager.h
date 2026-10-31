#pragma once

#include "SceneTextures.h"
#include <unordered_map>
#include <map>
#include <surrealgpu/vulkanbuilders.h>

class VulkanRenderDevice;
class CachedTexture;

struct TexDescriptorKey
{
	TexDescriptorKey(CachedTexture* tex, CachedTexture* lightmap, CachedTexture* detailtex, CachedTexture* macrotex, uint32_t sampler) : tex(tex), lightmap(lightmap), detailtex(detailtex), macrotex(macrotex), sampler(sampler) { }

	bool operator==(const TexDescriptorKey& other) const
	{
		return tex == other.tex && lightmap == other.lightmap && detailtex == other.detailtex && macrotex == other.macrotex && sampler == other.sampler;
	}

	bool operator<(const TexDescriptorKey& other) const
	{
		if (tex != other.tex)
			return tex < other.tex;
		else if (lightmap != other.lightmap)
			return lightmap < other.lightmap;
		else if (detailtex != other.detailtex)
			return detailtex < other.detailtex;
		else if (macrotex != other.macrotex)
			return macrotex < other.macrotex;
		else
			return sampler < other.sampler;
	}

	CachedTexture* tex;
	CachedTexture* lightmap;
	CachedTexture* detailtex;
	CachedTexture* macrotex;
	uint32_t sampler;
};

template<> struct std::hash<TexDescriptorKey>
{
	std::size_t operator()(const TexDescriptorKey& k) const
	{
		return (((std::size_t)k.tex ^ (std::size_t)k.lightmap));
	}
};

// Without bindless textures (GPUs that lack descriptor indexing, e.g. older Mali): the four textures of a draw call
// (texture, macro, detail, lightmap) with their samplers. A descriptor set per distinct key, made on demand each frame.
struct FixedTextureKey
{
	VkImageView view[4] = {};
	VkSampler sampler[4] = {};

	bool operator==(const FixedTextureKey& other) const
	{
		for (int i = 0; i < 4; i++)
			if (view[i] != other.view[i] || sampler[i] != other.sampler[i])
				return false;
		return true;
	}

	bool operator!=(const FixedTextureKey& other) const { return !(*this == other); }

	bool operator<(const FixedTextureKey& other) const
	{
		for (int i = 0; i < 4; i++)
		{
			if (view[i] != other.view[i])
				return view[i] < other.view[i];
			if (sampler[i] != other.sampler[i])
				return sampler[i] < other.sampler[i];
		}
		return false;
	}
};

class DescriptorSetManager
{
public:
	DescriptorSetManager(VulkanRenderDevice* renderer);
	~DescriptorSetManager();

	void ClearCache();

	bool IsTextureArrayFull() const { return Textures.NextBindlessIndex + 4 > MaxBindlessTextures; }
	int GetTextureArrayIndex(uint32_t PolyFlags, CachedTexture* tex, bool clamp = false);

	VulkanDescriptorSet* GetBindlessSet() { return Textures.BindlessSet.get(); }
	VulkanDescriptorSet* GetPresentSet() { return Present.Set.get(); }
	VulkanDescriptorSet* GetBloomPPImageSet() { return Bloom.PPImageSet.get(); }
	VulkanDescriptorSet* GetBloomVTextureSet(int level) { return Bloom.VTextureSets[level].get(); }
	VulkanDescriptorSet* GetBloomHTextureSet(int level) { return Bloom.HTextureSets[level].get(); }

	void UpdateBindlessSet();

	// No-bindless path
	VkDescriptorSet GetFixedTextureSet(const FixedTextureKey& key);
	void ResetFixedTextureSets();
	bool FixedTextureSetsFull() const { return FixedTextures.Count + 2 > MaxFixedTextureSets; }
	static const int MaxFixedTextureSets = 6000;
	VulkanDescriptorSetLayout* GetFixedTextureLayout() { return FixedTextures.Layout.get(); }
	void UpdateFrameDescriptors();

	static const int MaxBindlessTextures = 16536;

	VulkanDescriptorSetLayout* GetTextureBindlessLayout() { return Textures.BindlessLayout.get(); }
	VulkanDescriptorSetLayout* GetPresentLayout() { return Present.Layout.get(); }
	VulkanDescriptorSetLayout* GetBloomLayout() { return Bloom.Layout.get(); }

private:
	void CreateBindlessTextureSet();
	void CreateFixedTextureSets();
	void CreatePresentLayout();
	void CreatePresentSet();
	void CreateBloomLayout();
	void CreateBloomSets();

	VulkanRenderDevice* renderer = nullptr;

	struct
	{
		std::unique_ptr<VulkanDescriptorSetLayout> BindlessLayout;
		std::unique_ptr<VulkanDescriptorPool> BindlessPool;
		std::unique_ptr<VulkanDescriptorSet> BindlessSet;
		WriteDescriptors WriteBindless;
		int NextBindlessIndex = 0;

	} Textures;

	struct
	{
		std::unique_ptr<VulkanDescriptorSetLayout> Layout;
		VkDescriptorPool Pool = VK_NULL_HANDLE;
		std::map<FixedTextureKey, VkDescriptorSet> Cache;
		int Count = 0;
	} FixedTextures;

	struct
	{
		std::unique_ptr<VulkanDescriptorSetLayout> Layout;
		std::unique_ptr<VulkanDescriptorPool> Pool;
		std::unique_ptr<VulkanDescriptorSet> Set;
	} Present;

	struct
	{
		std::unique_ptr<VulkanDescriptorSetLayout> Layout;
		std::unique_ptr<VulkanDescriptorPool> Pool;
		std::unique_ptr<VulkanDescriptorSet> VTextureSets[NumBloomLevels];
		std::unique_ptr<VulkanDescriptorSet> HTextureSets[NumBloomLevels];
		std::unique_ptr<VulkanDescriptorSet> PPImageSet;
	} Bloom;
};
