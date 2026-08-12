#pragma once

#include <cstdint>
#include <vector>
#include <stdexcept>
#include <generator>
#include <algorithm>

template <typename T>
class Registry
{
public:
    Registry(int initialAllocation = 32)
    {
        firstFree = -1;
        registry.reserve(initialAllocation);
    }

    ~Registry() = default;

    int16_t registerItem(const T &item)
    {

        if (firstFree != -1)
        {
            int16_t recycledIndex = firstFree;
            firstFree = registry[recycledIndex].nextFree;

            registry[recycledIndex].item = item;
            registry[recycledIndex].active = true;

            return recycledIndex;
        }

        int16_t index = static_cast<int16_t>(registry.size());
        registry.push_back({item,
                            -1,
                            true});
        
        this->allIds.push_back(index); 
        std::sort(allIds.begin(), allIds.end());
        
        return index;
    }

    T &getItem(int16_t id)
    {

        if (id < 0 || static_cast<size_t>(id) >= registry.size() || !registry[id].active)
        {
            throw std::runtime_error("Cannot fetch invalid or inactive item.");
        }

        return registry[id].item;
    }

    int16_t removeItem(int16_t id)
    {
        if (id < 0 || id >= registry.size() || !registry[id].active)
        {
            throw std::runtime_error("Cannot remove invalid or inactive item.");
        }

        registry[id].active = false;
        registry[id].nextFree = firstFree;
        firstFree = id;

        std::erase(allIds, id);

        return id;
    }

    std::generator<T &> items()
    {
        for (auto &slot : registry)
        {
            if (slot.active)
            {
                co_yield slot.item;
            }
        }
    }

    bool itemExists(int16_t id) {
        if (id < 0 || id >= registry.size())
        {
            return false;
        }

        return registry[id].active;
        
    }

    const std::vector<int16_t> &ids() {
        return this->allIds;        
    }

    int16_t getFirst() {
        return this->allIds[0];
    }

    int16_t getLast() {
        return this->allIds[this->allIds.size() - 1];
    }

private:
    struct Slot
    {
        T item;
        int16_t nextFree;
        bool active;
    };

    std::vector<Slot> registry;
    std::vector<int16_t> allIds;
    int16_t firstFree;
};