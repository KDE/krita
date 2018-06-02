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
        qint32 currentThreads = m_rawPointerUsers.fetchAndAddRelaxed(1);
        TileType *result = m_map.erase(key);
        TileTypeSP ptr(result);

        if (result) {
            MemoryReclaimer *tmp = new MemoryReclaimer(result);
            QSBR::instance().enqueue(&MemoryReclaimer::destroy, tmp);
        }

        if (m_rawPointerUsers.testAndSetRelaxed(1, currentThreads)) {
            QSBR::instance().update(m_context);
        }

        m_rawPointerUsers.fetchAndSubRelaxed(1);
        return ptr;
    }

    TileTypeSP get(qint32 key)
    {
        m_rawPointerUsers.fetchAndAddRelaxed(1);
        TileTypeSP result(m_map.get(key));
        m_rawPointerUsers.fetchAndSubRelaxed(1);
        return result;
    }

    TileTypeSP getLazy(qint32 key)
    {
        m_rawPointerUsers.fetchAndAddRelaxed(1);
        typename ConcurrentMap<qint32, TileType *>::Mutator iter = m_map.insertOrFind(key);
        TileTypeSP value(iter.getValue());
        m_rawPointerUsers.fetchAndSubRelaxed(1);

        if (!value.data()) {
            value.attach(new TileType);
            TileTypeSP::ref(&value, value.data());
            iter.exchangeValue(value.data());
        }

        return value;
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
    QAtomicInt m_rawPointerUsers;
};

#endif // KIS_TILEHASHTABLE_2_H
