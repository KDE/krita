#ifndef KIS_TILEHASHTABLE_2_H
#define KIS_TILEHASHTABLE_2_H

#include "kis_shared.h"
#include "kis_shared_ptr.h"
#include "3rdparty/lock_free_map/concurrent_map.h"

#include "kis_lockless_stack.h"
#include "kis_tile.h"
#include <boost/functional/hash.hpp>

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

    TileTypeSP insert(quint32 key, TileTypeSP value)
    {
        TileTypeSP::ref(&value, value.data());
        TileType *result = m_map.assign(key, value.data());

        if (result) {
            m_map.getGC().enqueue(new MemoryReclaimer(result));
        } else {
            m_numTiles.fetchAndAddRelaxed(1);
        }

//        m_map.getGC().update();
        return TileTypeSP(result);
    }

    TileTypeSP erase(quint32 key)
    {
        TileType *result = m_map.erase(key);
        TileTypeSP ptr(result);

        if (result) {
            m_numTiles.fetchAndSubRelaxed(1);
            m_map.getGC().enqueue(new MemoryReclaimer(result));
        }

//        m_map.getGC().update();
        return ptr;
    }

    TileTypeSP get(quint32 key)
    {
        TileTypeSP result(m_map.get(key));
//        m_map.getGC().update();
        return result;
    }

    bool isEmpty()
    {
        return !m_numTiles.load();
    }

    bool tileExists(qint32 col, qint32 row);

    /**
     * Returns a tile in position (col,row). If no tile exists,
     * returns null.
     * \param col column of the tile
     * \param row row of the tile
     */
    TileTypeSP getExistingTile(qint32 col, qint32 row);

    /**
     * Returns a tile in position (col,row). If no tile exists,
     * creates a new one, attaches it to the list and returns.
     * \param col column of the tile
     * \param row row of the tile
     * \param newTile out-parameter, returns true if a new tile
     *                was created
     */
    TileTypeSP getTileLazy(qint32 col, qint32 row, bool& newTile);

    /**
     * Returns a tile in position (col,row). If no tile exists,
     * creates nothing, but returns shared default tile object
     * of the table. Be careful, this object has column and row
     * parameters set to (qint32_MIN, qint32_MIN).
     * \param col column of the tile
     * \param row row of the tile
     * \param existingTile returns true if the tile actually exists in the table
     *                     and it is not a lazily created default wrapper tile
     */
    TileTypeSP getReadOnlyTileLazy(qint32 col, qint32 row, bool &existingTile);
    void addTile(TileTypeSP tile);
    bool deleteTile(TileTypeSP tile);
    bool deleteTile(qint32 col, qint32 row);

    void clear();

    void setDefaultTileData(KisTileData *defaultTileData);
    KisTileData* defaultTileData();

    qint32 numTiles()
    {
        return m_numTiles.load();
    }

    void debugPrintInfo();
    void debugMaxListLength(qint32 &min, qint32 &max);

    ConcurrentMap<quint32, TileType*> &map()
    {
        return m_map;
    }

private:
    static inline quint32 calculateHash(qint32 col, qint32 row);

    struct MemoryReclaimer : public Property {
        MemoryReclaimer(TileType *data) : d(data) {}
        ~MemoryReclaimer()
        {
            TileTypeSP::deref(reinterpret_cast<TileTypeSP *>(this), d);
        }

    private:
        TileType *d;
    };

private:
    ConcurrentMap<quint32, TileType*> m_map;
    QAtomicInt m_numTiles;
    QReadWriteLock m_rwLock;
    KisLocklessStack<TileType*> m_stack;

    KisTileData *m_defaultTileData;
    KisMementoManager *m_mementoManager;
};

template <class T>
class KisTileHashTableIteratorTraits2
{
public:
    typedef T TileType;
    typedef KisSharedPtr<T> TileTypeSP;
    typedef typename ConcurrentMap<quint32, TileType*>::Iterator Iterator;

    KisTileHashTableIteratorTraits2(KisTileHashTableTraits2<T> *ht) : m_ht(ht)
    {
        m_iter.setMap(m_ht->map());
    }

    void next()
    {
        m_iter.next();
    }

    TileTypeSP tile() const
    {
        return TileTypeSP(m_iter.getValue());
    }

    bool isDone() const
    {
        return !m_iter.isValid();
    }

    void deleteCurrent()
    {
        m_ht->erase(m_iter.getKey());
        next();
    }

    void moveCurrentToHashTable(KisTileHashTableTraits2<T> *newHashTable)
    {
        TileTypeSP tile = m_iter.getValue();
        next();
        m_ht->deleteTile(tile);
        newHashTable->addTile(tile);
    }

private:
    KisTileHashTableTraits2<T> *m_ht;
    typename ConcurrentMap<quint32, TileType*>::Iterator m_iter;
};

template <class T>
KisTileHashTableTraits2<T>::KisTileHashTableTraits2()
    : m_numTiles(0), m_rwLock(QReadWriteLock::NonRecursive), m_defaultTileData(0), m_mementoManager(0)
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
    : KisTileHashTableTraits2(mm)
{
    setDefaultTileData(ht.m_defaultTileData);
    typename ConcurrentMap<quint32, TileType*>::Iterator iter(const_cast<ConcurrentMap<quint32, TileType*> &>(ht.m_map));
    while (iter.isValid()) {
        insert(iter.getKey(), iter.getValue());
        iter.next();
    }
}

template <class T>
KisTileHashTableTraits2<T>::~KisTileHashTableTraits2()
{
//    clear();
}

template<class T>
bool KisTileHashTableTraits2<T>::tileExists(qint32 col, qint32 row)
{
    return get(calculateHash(col, row)) != nullptr;
}

template <class T>
typename KisTileHashTableTraits2<T>::TileTypeSP KisTileHashTableTraits2<T>::getExistingTile(qint32 col, qint32 row)
{
    quint32 idx = calculateHash(col, row);
    return get(idx);
}

template <class T>
typename KisTileHashTableTraits2<T>::TileTypeSP KisTileHashTableTraits2<T>::getTileLazy(qint32 col, qint32 row, bool &newTile)
{
    newTile = false;
    TileTypeSP tile;
    quint32 idx = calculateHash(col, row);
    typename ConcurrentMap<quint32, TileType*>::Mutator mutator = m_map.insertOrFind(idx);

    if (!mutator.getValue()) {
        {
            QReadLocker guard(&m_rwLock);
            tile = new TileType(col, row, m_defaultTileData, m_mementoManager);
        }
        TileTypeSP::ref(&tile, tile.data());
        TileType *result = mutator.exchangeValue(tile.data());

        if (result) {
            m_map.getGC().enqueue(new MemoryReclaimer(result));
        } else {
            m_numTiles.fetchAndAddRelaxed(1);
        }

        newTile = true;
        tile = m_map.get(idx);
    } else {
        tile = mutator.getValue();
    }

//    m_map.getGC().update();
    return tile;
}

template <class T>
typename KisTileHashTableTraits2<T>::TileTypeSP KisTileHashTableTraits2<T>::getReadOnlyTileLazy(qint32 col, qint32 row, bool &existingTile)
{
    quint32 idx = calculateHash(col, row);
    TileTypeSP tile(m_map.get(idx));
    existingTile = tile;

    if (!existingTile) {
        QReadLocker guard(&m_rwLock);
        tile = new TileType(col, row, m_defaultTileData, 0);
    }

//    m_map.getGC().update();
    return tile;
}

template <class T>
void KisTileHashTableTraits2<T>::addTile(TileTypeSP tile)
{
    quint32 idx = calculateHash(tile->col(), tile->row());
    insert(idx, tile);
}

template <class T>
bool KisTileHashTableTraits2<T>::deleteTile(TileTypeSP tile)
{
    return deleteTile(tile->col(), tile->row());
}

template <class T>
bool KisTileHashTableTraits2<T>::deleteTile(qint32 col, qint32 row)
{
    quint32 idx = calculateHash(col, row);
    return erase(idx) != 0;
}

template<class T>
void KisTileHashTableTraits2<T>::clear()
{
    typename ConcurrentMap<quint32, TileType*>::Iterator iter(m_map);
    while (iter.isValid()) {
        erase(iter.getKey());
        iter.next();
    }
}

template <class T>
inline void KisTileHashTableTraits2<T>::setDefaultTileData(KisTileData *defaultTileData)
{
    QWriteLocker guard(&m_rwLock);
    if (m_defaultTileData) {
        m_defaultTileData->release();
        m_defaultTileData = 0;
    }

    if (defaultTileData) {
        defaultTileData->acquire();
        m_defaultTileData = defaultTileData;
    }
}

template <class T>
inline KisTileData* KisTileHashTableTraits2<T>::defaultTileData()
{
    QReadLocker guard(&m_rwLock);
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

template <class T>
quint32 KisTileHashTableTraits2<T>::calculateHash(qint32 col, qint32 row)
{
    std::size_t seed = 0;
    boost::hash_combine(seed, col);
    boost::hash_combine(seed, row);
    return seed;
//    return (((row << 5) + (col & 0x1F)) & 0x3FF) + 1;
}

typedef KisTileHashTableTraits2<KisTile> KisTileHashTable;
typedef KisTileHashTableIteratorTraits2<KisTile> KisTileHashTableIterator;
typedef KisTileHashTableIteratorTraits2<KisTile> KisTileHashTableConstIterator;

#endif // KIS_TILEHASHTABLE_2_H
