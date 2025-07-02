#include "ResourceManager.h"

ResourceManager::~ResourceManager()
{
    gltfModels_.clear();
}

std::shared_ptr<GltfModel> ResourceManager::LoadGltfModel(const std::string& filename, const std::string& rootNodeName)
{
    // 既にロード済み
    auto it = gltfModels_.find(filename);
    if (it != gltfModels_.end()) return it->second;

    // ロードする
    std::shared_ptr<GltfModel> model = std::make_shared<GltfModel>(filename, rootNodeName);
    gltfModels_[filename] = model;
    gltfModelFilenames_.emplace_back(filename);

    return model;
}
