/*
 *  SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISROTATINGCACHE_H
#define KISROTATINGCACHE_H

#include <optional>
#include <utility>

#include <boost/container/static_vector.hpp>


/**
 */
template <typename Key, typename Value, size_t CacheSize>
class KisRotatingCache
{
public:
    using CacheItem = std::pair<Key, Value>;
public:
    void push(Key key, Value value) {
        if (m_storage.size() == m_storage.max_size()) {
            m_storage.erase(m_storage.begin());
        }
        m_storage.emplace_back(std::move(key), std::move(value));
    }

    inline std::optional<Value> find(const Key &key) const {
        for (auto it = m_storage.begin(); it != m_storage.end(); ++it) {
            if (it->first == key) {
                return it->second;
            }
        }
        return std::nullopt;
    }

    void clear() {
        m_storage.clear();
    }

private:
    boost::container::static_vector<CacheItem, CacheSize> m_storage;
};

#endif // KISROTATINGCACHE_H
