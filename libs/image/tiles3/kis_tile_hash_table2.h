#ifndef KIS_TILEHASHTABLE_2_H
#define KIS_TILEHASHTABLE_2_H

#include "kis_shared.h"
#include "kis_shared_ptr.h"
#include "3rdparty/lock_free_map/concurrent_map.h"

#include "kis_memento_manager.h"

template <class T>
class KisTileHashTableTraits2
{
    static constexpr bool isInherited = std::is_convertible<T*, KisShared*>::value;
    Q_STATIC_ASSERT_X(isInherited, "Template must inherit KisShared");

public:
    typedef T TileType;
    typedef KisSharedPtr<T> TileTypeSP;
    typedef KisWeakSharedPtr<T> TileTypeWSP;

    KisTileHashTableTraits2();
    KisTileHashTableTraits2(KisMementoManager *mm);
    KisTileHashTableTraits2(const KisTileHashTableTraits2<T> &ht, KisMementoManager *mm);
    ~KisTileHashTableTraits2();

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

    bool tileExists(qint32 key);
    TileTypeSP getExistingTile(qint32 key);
    TileTypeSP getTileLazy(qint32 key);
    TileTypeSP getReadOnlyTileLazy(qint32 key, bool &existingTile);

    void addTile(qint32 key, TileTypeSP value);
    bool deleteTile(qint32 key);

    void setDefaultTileDataImp(KisTileData *defaultTileData);
    KisTileData* defaultTileDataImp() const;

    void debugPrintInfo();
    void debugMaxListLength(qint32 &min, qint32 &max);

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

    KisTileData *m_defaultTileData;
    KisMementoManager *m_mementoManager;
};

template <class T>
KisTileHashTableTraits2<T>::KisTileHashTableTraits2()
    : m_context(QSBR::instance().createContext()), m_rawPointerUsers(0),
      m_defaultTileData(0), m_mementoManager(0)
{
}

template <class T>
KisTileHashTableTraits2<T>::KisTileHashTableTraits2(KisMementoManager *mm)
    : KisTileHashTableTraits2()
{
    m_mementoManager = mm;
}

template <class T>
KisTileHashTableTraits2<T>::KisTileHashTableTraits2(const KisTileHashTableTraits2<T> &ht, KisMementoManager *mm)
{
}

template <class T>
KisTileHashTableTraits2<T>::~KisTileHashTableTraits2()
{
    QSBR::instance().destroyContext(m_context);
}

template<class T>
bool KisTileHashTableTraits2<T>::tileExists(qint32 key)
{
    return get(key) != nullptr;
}

template <class T>
typename KisTileHashTableTraits2<T>::TileTypeSP KisTileHashTableTraits2<T>::getExistingTile(qint32 key)
{
    return get(key);
}

template <class T>
typename KisTileHashTableTraits2<T>::TileTypeSP KisTileHashTableTraits2<T>::getTileLazy(qint32 key)
{
    return getLazy(key);
}

template <class T>
typename KisTileHashTableTraits2<T>::TileTypeSP KisTileHashTableTraits2<T>::getReadOnlyTileLazy(qint32 key, bool &existingTile)
{
    m_rawPointerUsers.fetchAndAddOrdered(1);
    TileTypeSP tile(m_map.get(key));
    existingTile = tile;

    if (!existingTile) {
        tile = new TileType;
    }

    m_rawPointerUsers.fetchAndSubOrdered(1);
    return tile;
}

template <class T>
void KisTileHashTableTraits2<T>::addTile(qint32 key, TileTypeSP value)
{
    insert(key, value);
}

template <class T>
bool KisTileHashTableTraits2<T>::deleteTile(qint32 key)
{
    return erase(key) != nullptr;
}

template<class T>
inline void KisTileHashTableTraits2<T>::setDefaultTileDataImp(KisTileData *defaultTileData)
{
    if (m_defaultTileData) {
        m_defaultTileData->release();
        m_defaultTileData = 0;
    }

    if (defaultTileData) {
        defaultTileData->acquire();
        m_defaultTileData = defaultTileData;
    }
}

template<class T>
inline KisTileData* KisTileHashTableTraits2<T>::defaultTileDataImp() const
{
    return m_defaultTileData;
}

template <class T>
void KisTileHashTableTraits2<T>::debugPrintInfo()
{
}

template <class T>
void KisTileHashTableTraits2<T>::debugMaxListLength(qint32 &min, qint32 &max)
{
}

#endif // KIS_TILEHASHTABLE_2_H
