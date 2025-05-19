#pragma once
#include <string>
#include <map>
#include <memory>

#include "Resource/Audio/Audio.h"
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

private:




    const std::shared_ptr<GltfModel> LoadModelResource(const char* const filename);

private:
    using ModelMap = std::map<const char*, std::weak_ptr<GltfModel>>;

    ModelMap models_ = {};
};