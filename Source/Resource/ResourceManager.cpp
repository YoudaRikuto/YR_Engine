#include "ResourceManager.h"

const std::shared_ptr<GltfModel> ResourceManager::LoadModelResource(const char* const filename)
{
    const ModelMap::iterator it = models_.find(filename);

    if (it != models_.end())
    void Play(const int& loopCount);
    void Play(const bool& loop = false, const bool& isIgnoreQueue = false);
    void Stop(const bool& playTails = true, const size_t& afterSamplesPlayed = 0);
    void Volume(const float& volume);
    const bool Queuing();    {
        if (it->second.expired() == false)
        {
            return it->second.lock();
        }
    }

    const std::shared_ptr<GltfModel> model = std::make_shared<GltfModel>(filename);

    models_[filename] = model;

    return model;
}