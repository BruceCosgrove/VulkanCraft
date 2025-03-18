#pragma once

#include "Engine/Core/Attributes.hpp"
#include <atomic>
#include <memory>

namespace eng
{
    // Holds a resource that can be reloaded on another thread.
    // Call Get to get the current resource. After the resource
    // has been loaded, the subsequent call to Get will also
    // return the old resource, which you may use how you like.
    // During the time after the resource had been loaded and
    // before the next call to Get, the resource will simply
    // not attempt to load again, no matter how many times Load
    // is called.
    template <class Resource>
    class DynamicResource
    {
    public:
        DynamicResource() = default;
        DynamicResource(Resource&& resource)
            : m_Current(std::move(resource)) {}

        // Call on the thread you want to access the resource from.
        // Sets the current resource and returns the old resource.
        // Only discard the old resource if you don't need it.
        ENG_NO_DISCARD Resource Exchange(Resource&& resource)
        {
            return std::exchange(m_Current, resource);
        }

        struct GetResult
        {
            Resource& Current;
            Resource Old;
        };

        // Call on the thread you want to access the resource from.
        ENG_NO_DISCARD GetResult Get()
        {
            Resource old;
            if (m_Loaded.exchange(false, std::memory_order_acquire))
            {
                old = std::move(m_Current);
                m_Current = std::move(m_Next);
                m_Loading.store(false, std::memory_order_release);
            }
            return {m_Current, std::move(old)};
        }

        // Call on the thread you want to asynchronously load the resource on, e.g. in a thread pool.
        template <typename LoadFunction>
        requires(requires(LoadFunction&& loadFunction) { { loadFunction() } -> std::same_as<Resource>; })
        void Load(LoadFunction&& loadFunction)
        {
            if (not m_Loading.exchange(true, std::memory_order_acquire))
            {
                m_Next = loadFunction();
                m_Loaded.store(true, std::memory_order_release);
            }
        }

        // Returns if the next version of the resource is currently loading.
        bool Loading() const
        {
            return m_Loading.load(std::memory_order_acquire);
        }
    private:
        Resource m_Current;
        Resource m_Next;
        std::atomic_bool m_Loading;
        std::atomic_bool m_Loaded;
    };
}
