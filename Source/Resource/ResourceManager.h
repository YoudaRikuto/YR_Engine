#pragma once
#include <unordered_map>
#include <string>
#include <memory>

#include "Resource/GltfModel/GltfModel.h"

class ResourceManager
{
private:
    ResourceManager() {}
    ~ResourceManager();

public:
    static ResourceManager& Instance()
    {
        static ResourceManager instance;
        return instance;
    }

    std::shared_ptr<GltfModel> LoadGltfModel(const std::string& filename, const std::string& rootNodeName = "root");

    const std::vector<std::string> GetGltfModelFilenames() const { return gltfModelFilenames_; }
    const int GetGltfModelCount() const { return gltfModels_.size(); }

private:
    std::unordered_map<std::string, std::shared_ptr<GltfModel>> gltfModels_;
    std::vector<std::string> gltfModelFilenames_;
};