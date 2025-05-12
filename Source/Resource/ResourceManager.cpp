#include "ResourceManager.h"

#if 0
const std::shared_ptr<GltfModel> ResourceManager::LoadModelResource(const char* const filename)
{
    const ModelMap::iterator it = models_.find(filename);

    if (it != models_.end())
    {
        if (it->second.expired() == false)
        {
            return it->second.lock();
        }
    }

    const std::shared_ptr<GltfModel> model = std::make_shared<GltfModel>(filename);

    models_[filename] = model;

    return model;
}
#endif