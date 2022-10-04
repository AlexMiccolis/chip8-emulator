#pragma once

#include <deque>
#include <unordered_map>

template <class K, class V>
class LRUCache
{
public:
    using key_type = K;
    using value_type = V;

    LRUCache(size_t capacity)
        : m_Capacity(capacity)
    { }

    bool Contains(const key_type& key) const
    {
        return m_Cache.contains(key);
    }

    const value_type& Get(const key_type& key)
    {
        auto it = std::find(m_Keys.cbegin(), m_Keys.cend(), key);
        m_Keys.erase(it);
        m_Keys.push_front(key);
        return m_Cache.at(key);
    }

    void Put(const key_type& key, value_type&& value)
    {
        if (!Contains(key))
        {
            if (m_Keys.size() >= m_Capacity)
            {
                m_Cache.erase(m_Keys.back());
                m_Keys.pop_back();
            }
        }
        else
        {
            auto it = std::find(m_Keys.cbegin(), m_Keys.cend(), key);
            m_Keys.erase(it);
            m_Cache.erase(key);
        }

        m_Keys.push_front(key);

        key_type keyCopy = key;
        m_Cache.emplace(std::move(keyCopy), std::move(value));
    }

    const value_type& operator[](const key_type& key) const
    {
        return Get(key);
    }
    
private:
    std::deque<key_type> m_Keys;
    std::unordered_map<key_type, value_type> m_Cache;
    size_t m_Capacity;
};
