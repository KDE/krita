#ifndef KIS_TILEHASHTABLE_2_H
#define KIS_TILEHASHTABLE_2_H

#include "kis_shared.h"
#include "kis_shared_ptr.h"
#include "3rdparty/lock_free_map/concurrent_map.h"

#include "kis_tile.h"

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
        m_rawPointerUsers.fetchAndAddRelaxed(1);
        TileTypeSP::ref(&value, value.data());
        TileType *result = m_map.assign(key, value.data());

        if (result) {
            MemoryReclaimer *tmp = new MemoryReclaimer(result);
            QSBR::instance().enqueue(&MemoryReclaimer::destroy, tmp);
        } else {
            m_numTiles.fetchAndAddRelaxed(1);
        }

        TileTypeSP ptr(result);
        m_rawPointerUsers.fetchAndSubRelaxed(1);
        return ptr;
    }

    TileTypeSP erase(qint32 key)
    {
        m_rawPointerUsers.fetchAndAddRelaxed(1);
        TileType *result = m_map.erase(key);
        TileTypeSP ptr(result);

        if (result) {
            m_numTiles.fetchAndSubRelaxed(1);
            MemoryReclaimer *tmp = new MemoryReclaimer(result);
            QSBR::instance().enqueue(&MemoryReclaimer::destroy, tmp);
        }

        if (m_rawPointerUsers == 1) {
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

    TileTypeSP getLazy(qint32 key, TileTypeSP value, bool &newTile)
    {
        m_rawPointerUsers.fetchAndAddRelaxed(1);
        newTile = false;
        typename ConcurrentMap<qint32, TileType *>::Mutator iter = m_map.insertOrFind(key);

        if (!iter.getValue()) {
            TileTypeSP::ref(&value, value.data());

            if (iter.exchangeValue(value.data()) == value.data()) {
                TileTypeSP::deref(&value, value.data());
            } else {
                m_numTiles.fetchAndAddRelaxed(1);
                newTile = true;
            }
        }

        TileTypeSP result(iter.getValue());
        m_rawPointerUsers.fetchAndSubRelaxed(1);
        return result;
    }

    bool isEmpty()
    {
        return !m_numTiles;
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
    KisTileData* defaultTileData() const;

    qint32 numTiles()
    {
        return m_numTiles;
    }

    void debugPrintInfo();
    void debugMaxListLength(qint32 &min, qint32 &max);

    typename ConcurrentMap<qint32, TileType*>::Iterator iterator()
    {
        typename ConcurrentMap<qint32, TileType*>::Iterator iter(m_map);
        return iter;
    }

private:
    static inline quint32 calculateHash(qint32 col, qint32 row);

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

private:
    ConcurrentMap<qint32, TileType*> m_map;
    QSBR::Context m_context;
    QAtomicInt m_rawPointerUsers;
    QAtomicInt m_numTiles;

    KisTileData *m_defaultTileData;
    KisMementoManager *m_mementoManager;
};

template <class T>
class KisTileHashTableIteratorTraits2
{
public:
    typedef T TileType;
    typedef KisSharedPtr<T> TileTypeSP;

    KisTileHashTableIteratorTraits2(KisTileHashTableTraits2<T> *ht) : m_ht(ht)
    {
    }

    void next()
    {
        m_ht->iterator().next();
    }

    TileTypeSP tile() const
    {
        return TileTypeSP(m_ht->iterator().getValue());
    }

    bool isDone() const
    {
        return m_ht->iterator().isValid();
    }

    void deleteCurrent()
    {
        m_ht->erase(m_ht->iterator().getKey());
        next();
    }

    void moveCurrentToHashTable(KisTileHashTableTraits2<T> *newHashTable)
    {
        TileTypeSP tile = m_ht->iterator().getValue();
        next();
        m_ht->deleteTile(tile);
        newHashTable->addTile(tile);
    }

private:
    KisTileHashTableTraits2<T> *m_ht;
};

template <class T>
KisTileHashTableTraits2<T>::KisTileHashTableTraits2()
    : m_context(QSBR::instance().createContext()), m_rawPointerUsers(0), m_numTiles(0),
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
    : KisTileHashTableTraits2(mm)
{
    setDefaultTileData(ht.m_defaultTileData);
    typename ConcurrentMap<qint32, TileType*>::Iterator iter(const_cast<ConcurrentMap<qint32, TileType*> &>(ht.m_map));
    while (iter.isValid()) {
        insert(iter.getKey(), iter.getValue());
        iter.next();
    }
}

template <class T>
KisTileHashTableTraits2<T>::~KisTileHashTableTraits2()
{
    clear();
    QSBR::instance().destroyContext(m_context);
}

template<class T>
bool KisTileHashTableTraits2<T>::tileExists(qint32 col, qint32 row)
{
    return get(calculateHash(col, row)) != nullptr;
}

template <class T>
typename KisTileHashTableTraits2<T>::TileTypeSP KisTileHashTableTraits2<T>::getExistingTile(qint32 col, qint32 row)
{
    return get(calculateHash(col, row));
}

template <class T>
typename KisTileHashTableTraits2<T>::TileTypeSP KisTileHashTableTraits2<T>::getTileLazy(qint32 col, qint32 row, bool &newTile)
{
    TileTypeSP tile(new TileType(col, row, m_defaultTileData, m_mementoManager));
    return getLazy(calculateHash(col, row), tile, newTile);
}

template <class T>
typename KisTileHashTableTraits2<T>::TileTypeSP KisTileHashTableTraits2<T>::getReadOnlyTileLazy(qint32 col, qint32 row, bool &existingTile)
{
    m_rawPointerUsers.fetchAndAddRelaxed(1);
    TileTypeSP tile(m_map.get(calculateHash(col, row)));
    existingTile = tile;

    if (!existingTile) {
        tile = new TileType(col, row, m_defaultTileData, 0);
    }

    m_rawPointerUsers.fetchAndSubRelaxed(1);
    return tile;
}

template <class T>
void KisTileHashTableTraits2<T>::addTile(TileTypeSP value)
{
    insert(m_numTiles.load() + 1, value);
}

template <class T>
bool KisTileHashTableTraits2<T>::deleteTile(TileTypeSP tile)
{
    return deleteTile(tile->col(), tile->row());
}

template <class T>
bool KisTileHashTableTraits2<T>::deleteTile(qint32 col, qint32 row)
{
    return erase(calculateHash(col, row)) != nullptr;
}

template<class T>
void KisTileHashTableTraits2<T>::clear()
{
    typename ConcurrentMap<qint32, TileType*>::Iterator iter(m_map);
    while (iter.isValid()) {
        erase(iter.getKey());
        iter.next();
    }
}

template <class T>
inline void KisTileHashTableTraits2<T>::setDefaultTileData(KisTileData *defaultTileData)
{
    m_rawPointerUsers.fetchAndAddRelaxed(1);

    if (m_defaultTileData) {
        m_defaultTileData->release();
        m_defaultTileData = 0;
    }

    if (defaultTileData) {
        defaultTileData->acquire();
        m_defaultTileData = defaultTileData;
    }

    m_rawPointerUsers.fetchAndSubRelaxed(1);
}

template <class T>
inline KisTileData* KisTileHashTableTraits2<T>::defaultTileData() const
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

template <class T>
quint32 KisTileHashTableTraits2<T>::calculateHash(qint32 col, qint32 row)
{
    return ((row << 5) + (col & 0x1F)) & 0x3FF;
}

typedef KisTileHashTableTraits2<KisTile> KisTileHashTable;
typedef KisTileHashTableIteratorTraits2<KisTile> KisTileHashTableIterator;
typedef KisTileHashTableIteratorTraits2<KisTile> KisTileHashTableConstIterator;

#endif // KIS_TILEHASHTABLE_2_H
