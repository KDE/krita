#ifndef KIS_TILEHASHTABLE_2_H
#define KIS_TILEHASHTABLE_2_H

#include "kis_shared.h"
#include "kis_shared_ptr.h"
#include "3rdparty/lock_free_map/concurrent_map.h"

template <class T>
class KisTileHashTableTraits2
{
    static_assert(std::is_convertible<T*, KisShared*>::value, "Template must inherit KisShared");

public:
    typedef T TileType;
    typedef KisSharedPtr<T> TileTypeSP;
    typedef KisWeakSharedPtr<T> TileTypeWSP;
    typedef typename ConcurrentMap<qint32, TileType *>::Iterator Iterator;

    KisTileHashTableTraits2()
        : m_rawPointerUsers(0)
    {
        m_context = QSBR::instance().createContext();
    }

    ~KisTileHashTableTraits2()
    {
        QSBR::instance().destroyContext(m_context);
    }

    TileTypeSP insert(qint32 key, TileTypeSP value)
    {
        TileTypeSP::ref(&value, value.data());
        TileType *result = m_map.assign(key, value.data());
        if (result) {
            MemoryReclaimer *tmp = new MemoryReclaimer(result);
            QSBR::instance().enqueue(&MemoryReclaimer::destroy, tmp);
        }
        return TileTypeSP(result);
    }

    TileTypeSP erase(qint32 key)
    {
        qint32 currentThreads = m_rawPointerUsers.fetchAdd(1, ConsumeRelease);

        TileType *result = m_map.erase(key);
        TileTypeSP ptr(result);
        if (result) {
            MemoryReclaimer *tmp = new MemoryReclaimer(result);
            QSBR::instance().enqueue(&MemoryReclaimer::destroy, tmp);
        }

        qint32 expected = 1;
        if (m_rawPointerUsers.compareExchangeStrong(expected, currentThreads, ConsumeRelease)) {
            QSBR::instance().update(m_context);
        }

        m_rawPointerUsers.fetchSub(1, ConsumeRelease);
        return ptr;
    }

    TileTypeSP get(qint32 key)
    {
        m_rawPointerUsers.fetchAdd(1, ConsumeRelease);
        TileTypeSP result(m_map.get(key));
        m_rawPointerUsers.fetchSub(1, ConsumeRelease);
        return result;
    }

    Iterator iterator()
    {
        return Iterator(m_map);
    }

private:
    struct MemoryReclaimer {
        MemoryReclaimer(TileType *data) : d(data) {}
        ~MemoryReclaimer() = default;

        void destroy()
        {
            TileTypeSP::deref(reinterpret_cast<TileTypeSP *>(this), d);
            this->MemoryReclaimer::~MemoryReclaimer();
            std::free(this);
        }

    private:
        TileType *d;
    };

    ConcurrentMap<qint32, TileType *> m_map;
    QSBR::Context m_context;
    Atomic<qint32> m_rawPointerUsers;
};

#endif // KIS_TILEHASHTABLE_2_H
