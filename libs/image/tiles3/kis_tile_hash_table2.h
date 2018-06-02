#ifndef KIS_TILEHASHTABLE_2_H
#define KIS_TILEHASHTABLE_2_H

#include "kis_shared.h"
#include "kis_shared_ptr.h"
#include "3rdparty/lock_free_map/concurrent_map.h"

template <class T>
class KisTileHashTableTraits2
{
    static constexpr bool isInherited = std::is_convertible<T*, KisShared*>::value;
    Q_STATIC_ASSERT_X(isInherited, "Template must inherit KisShared");

public:
    typedef T TileType;
    typedef KisSharedPtr<T> TileTypeSP;
    typedef KisWeakSharedPtr<T> TileTypeWSP;

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
        m_rawPointerUsers.fetchAndAddOrdered(1);
        TileTypeSP::ref(&value, value.data());
        TileType *result = m_map.assign(key, value.data());

        if (result) {
            MemoryReclaimer *tmp = new MemoryReclaimer(result);
            QSBR::instance().enqueue(&MemoryReclaimer::destroy, tmp);
        }

        TileTypeSP ptr(result);
        m_rawPointerUsers.fetchAndSubOrdered(1);
        return ptr;
    }

    TileTypeSP erase(qint32 key)
    {
        m_rawPointerUsers.fetchAndAddOrdered(1);
        TileType *result = m_map.erase(key);
        TileTypeSP ptr(result);

        if (result) {
            MemoryReclaimer *tmp = new MemoryReclaimer(result);
            QSBR::instance().enqueue(&MemoryReclaimer::destroy, tmp);
        }

        if (m_rawPointerUsers == 1) {
            QSBR::instance().update(m_context);
        }

        m_rawPointerUsers.fetchAndSubOrdered(1);
        return ptr;
    }

    TileTypeSP get(qint32 key)
    {
        m_rawPointerUsers.fetchAndAddOrdered(1);
        TileTypeSP result(m_map.get(key));
        m_rawPointerUsers.fetchAndSubOrdered(1);
        return result;
    }

    TileTypeSP getLazy(qint32 key)
    {
        m_rawPointerUsers.fetchAndAddOrdered(1);
        typename ConcurrentMap<qint32, TileType *>::Mutator iter = m_map.insertOrFind(key);

        if (!iter.getValue()) {
            TileTypeSP value(new TileType);
            TileTypeSP::ref(&value, value.data());

            if (iter.exchangeValue(value.data()) == value.data()) {
                TileTypeSP::deref(&value, value.data());
            }
        }

        TileTypeSP result(iter.getValue());
        m_rawPointerUsers.fetchAndSubOrdered(1);
        return result;
    }

private:
    struct MemoryReclaimer {
        MemoryReclaimer(TileType *data) : d(data) {}
        ~MemoryReclaimer() = default;

        void destroy()
        {
            TileTypeSP::deref(reinterpret_cast<TileTypeSP *>(this), d);
            this->MemoryReclaimer::~MemoryReclaimer();
            delete this;
        }

    private:
        TileType *d;
    };

    ConcurrentMap<qint32, TileType *> m_map;
    QSBR::Context m_context;
    QAtomicInt m_rawPointerUsers;
};

#endif // KIS_TILEHASHTABLE_2_H
