/*------------------------------------------------------------------------
  Junction: Concurrent data structures in C++
  Copyright (c) 2016 Jeff Preshing
  Distributed under the Simplified BSD License.
  Original location: https://github.com/preshing/junction
  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the LICENSE file for more information.
------------------------------------------------------------------------*/

#ifndef LEAPFROG_H
#define LEAPFROG_H

#include "map_traits.h"
#include "simple_job_coordinator.h"
#include "kis_assert.h"

#define SANITY_CHECK

template <class Map>
struct Leapfrog {
    typedef typename Map::Hash Hash;
    typedef typename Map::Value Value;
    typedef typename Map::KeyTraits KeyTraits;
    typedef typename Map::ValueTraits ValueTraits;

    static const quint64 InitialSize = 8;
    static const quint64 TableMigrationUnitSize = 32;
    static const quint64 LinearSearchLimit = 128;
    static const quint64 CellsInUseSample = LinearSearchLimit;

    Q_STATIC_ASSERT(LinearSearchLimit > 0 && LinearSearchLimit < 256);              // Must fit in CellGroup::links
    Q_STATIC_ASSERT(CellsInUseSample > 0 && CellsInUseSample <= LinearSearchLimit); // Limit sample to failed search chain

    struct Cell {
        Atomic<Hash> hash;
        Atomic<Value> value;
    };

    struct CellGroup {
        // Every cell in the table actually represents a bucket of cells, all linked together in a probe chain.
        // Each cell in the probe chain is located within the table itself.
        // "deltas" determines the index of the next cell in the probe chain.
        // The first cell in the chain is the one that was hashed. It may or may not actually belong in the bucket.
        // The "second" cell in the chain is given by deltas 0 - 3. It's guaranteed to belong in the bucket.
        // All subsequent cells in the chain is given by deltas 4 - 7. Also guaranteed to belong in the bucket.
        Atomic<quint8> deltas[8];
        Cell cells[4];
    };

    struct Table {
        const quint64 sizeMask;                 // a power of two minus one
        QMutex mutex;                   // to DCLI the TableMigration (stored in the jobCoordinator)
        SimpleJobCoordinator jobCoordinator; // makes all blocked threads participate in the migration

        Table(quint64 sizeMask) : sizeMask(sizeMask)
        {
        }

        static Table* create(quint64 tableSize)
        {
#ifdef SANITY_CHECK
            KIS_ASSERT_RECOVER_NOOP(isPowerOf2(tableSize));
            KIS_ASSERT_RECOVER_NOOP(tableSize >= 4);
#endif // SANITY_CHECK
            quint64 numGroups = tableSize >> 2;
            Table* table = (Table*) std::malloc(sizeof(Table) + sizeof(CellGroup) * numGroups);
            new (table) Table(tableSize - 1);

            for (quint64 i = 0; i < numGroups; i++) {
                CellGroup* group = table->getCellGroups() + i;

                for (quint64 j = 0; j < 4; j++) {
                    group->deltas[j].storeNonatomic(0);
                    group->deltas[j + 4].storeNonatomic(0);
                    group->cells[j].hash.storeNonatomic(KeyTraits::NullHash);
                    group->cells[j].value.storeNonatomic(Value(ValueTraits::NullValue));
                }
            }
            return table;
        }

        void destroy()
        {
            this->Table::~Table();
            std::free(this);
        }

        CellGroup* getCellGroups() const
        {
            return (CellGroup*)(this + 1);
        }

        quint64 getNumMigrationUnits() const
        {
            return sizeMask / TableMigrationUnitSize + 1;
        }
    };

    class TableMigration : public SimpleJobCoordinator::Job
    {
    public:
        struct Source {
            Table* table;
            Atomic<quint64> sourceIndex;
        };

        Map& m_map;
        Table* m_destination;
        Atomic<quint64> m_workerStatus; // number of workers + end flag
        Atomic<bool> m_overflowed;
        Atomic<qint64> m_unitsRemaining;
        quint64 m_numSources;

        TableMigration(Map& map) : m_map(map)
        {
        }

        static TableMigration* create(Map& map, quint64 numSources)
        {
            TableMigration* migration =
                (TableMigration*) std::malloc(sizeof(TableMigration) + sizeof(TableMigration::Source) * numSources);
            new (migration) TableMigration(map);

            migration->m_workerStatus.storeNonatomic(0);
            migration->m_overflowed.storeNonatomic(false);
            migration->m_unitsRemaining.storeNonatomic(0);
            migration->m_numSources = numSources;
            // Caller is responsible for filling in sources & destination
            return migration;
        }

        virtual ~TableMigration() override
        {
        }

        void destroy()
        {
            // Destroy all source tables.
            for (quint64 i = 0; i < m_numSources; i++)
                if (getSources()[i].table)
                    getSources()[i].table->destroy();
            // Delete the migration object itself.
            this->TableMigration::~TableMigration();
            std::free(this);
        }

        Source* getSources() const
        {
            return (Source*)(this + 1);
        }

        bool migrateRange(Table* srcTable, quint64 startIdx);
        virtual void run() override;
    };

    static Cell* find(Hash hash, Table* table)
    {
#ifdef SANITY_CHECK
        KIS_ASSERT_RECOVER_NOOP(table);
        KIS_ASSERT_RECOVER_NOOP(hash != KeyTraits::NullHash);
#endif // SANITY_CHECK
        quint64 sizeMask = table->sizeMask;
        // Optimistically check hashed cell even though it might belong to another bucket
        quint64 idx = hash & sizeMask;
        CellGroup* group = table->getCellGroups() + (idx >> 2);
        Cell* cell = group->cells + (idx & 3);
        Hash probeHash = cell->hash.load(Relaxed);

        if (probeHash == hash) {
            return cell;
        } else if (probeHash == KeyTraits::NullHash) {
            return cell = NULL;
        }
        // Follow probe chain for our bucket
        quint8 delta = group->deltas[idx & 3].load(Relaxed);
        while (delta) {
            idx = (idx + delta) & sizeMask;
            group = table->getCellGroups() + (idx >> 2);
            cell = group->cells + (idx & 3);
            Hash probeHash = cell->hash.load(Relaxed);
            // Note: probeHash might actually be NULL due to memory reordering of a concurrent insert,
            // but we don't check for it. We just follow the probe chain.
            if (probeHash == hash) {
                return cell;
            }
            delta = group->deltas[(idx & 3) + 4].load(Relaxed);
        }
        // End of probe chain, not found
        return NULL;
    }

    // FIXME: Possible optimization: Dedicated insert for migration? It wouldn't check for InsertResult_AlreadyFound.
    enum InsertResult { InsertResult_AlreadyFound, InsertResult_InsertedNew, InsertResult_Overflow };
    static InsertResult insertOrFind(Hash hash, Table* table, Cell*& cell, quint64& overflowIdx)
    {
#ifdef SANITY_CHECK
        KIS_ASSERT_RECOVER_NOOP(table);
        KIS_ASSERT_RECOVER_NOOP(hash != KeyTraits::NullHash);
#endif // SANITY_CHECK
        quint64 sizeMask = table->sizeMask;
        quint64 idx = quint64(hash);

        // Check hashed cell first, though it may not even belong to the bucket.
        CellGroup* group = table->getCellGroups() + ((idx & sizeMask) >> 2);
        cell = group->cells + (idx & 3);
        Hash probeHash = cell->hash.load(Relaxed);

        if (probeHash == KeyTraits::NullHash) {
            if (cell->hash.compareExchangeStrong(probeHash, hash, Relaxed)) {
                // There are no links to set. We're done.
                return InsertResult_InsertedNew;
            } else {
                // Fall through to check if it was the same hash...
            }
        }

        if (probeHash == hash) {
            return InsertResult_AlreadyFound;
        }

        // Follow the link chain for this bucket.
        quint64 maxIdx = idx + sizeMask;
        quint64 linkLevel = 0;
        Atomic<quint8>* prevLink;
        for (;;) {
        followLink:
            prevLink = group->deltas + ((idx & 3) + linkLevel);
            linkLevel = 4;
            quint8 probeDelta = prevLink->load(Relaxed);

            if (probeDelta) {
                idx += probeDelta;
                // Check the hash for this cell.
                group = table->getCellGroups() + ((idx & sizeMask) >> 2);
                cell = group->cells + (idx & 3);
                probeHash = cell->hash.load(Relaxed);

                if (probeHash == KeyTraits::NullHash) {
                    // Cell was linked, but hash is not visible yet.
                    // We could avoid this case (and guarantee it's visible) using acquire & release, but instead,
                    // just poll until it becomes visible.
                    do {
                        probeHash = cell->hash.load(Acquire);
                    } while (probeHash == KeyTraits::NullHash);
                }

#ifdef SANITY_CHECK
                KIS_ASSERT_RECOVER_NOOP(((probeHash ^ hash) & sizeMask) == 0); // Only hashes in same bucket can be linked
#endif // SANITY_CHECK
                if (probeHash == hash) {
                    return InsertResult_AlreadyFound;
                }
            } else {
                // Reached the end of the link chain for this bucket.
                // Switch to linear probing until we reserve a new cell or find a late-arriving cell in the same bucket.
                quint64 prevLinkIdx = idx;
#ifdef SANITY_CHECK
                KIS_ASSERT_RECOVER_NOOP(qint64(maxIdx - idx) >= 0); // Nobody would have linked an idx that's out of range.
#endif // SANITY_CHECK
                quint64 linearProbesRemaining = qMin(maxIdx - idx, quint64(LinearSearchLimit));

                while (linearProbesRemaining-- > 0) {
                    idx++;
                    group = table->getCellGroups() + ((idx & sizeMask) >> 2);
                    cell = group->cells + (idx & 3);
                    probeHash = cell->hash.load(Relaxed);

                    if (probeHash == KeyTraits::NullHash) {
                        // It's an empty cell. Try to reserve it.
                        if (cell->hash.compareExchangeStrong(probeHash, hash, Relaxed)) {
                            // Success. We've reserved the cell. Link it to previous cell in same bucket.
#ifdef SANITY_CHECK
                            KIS_ASSERT_RECOVER_NOOP(probeDelta == 0);
#endif // SANITY_CHECK
                            quint8 desiredDelta = idx - prevLinkIdx;
                            prevLink->store(desiredDelta, Relaxed);
                            return InsertResult_InsertedNew;
                        } else {
                            // Fall through to check if it's the same hash...
                        }
                    }
                    Hash x = (probeHash ^ hash);
                    // Check for same hash.
                    if (!x) {
                        return InsertResult_AlreadyFound;
                    }
                    // Check for same bucket.
                    if ((x & sizeMask) == 0) {
                        // Attempt to set the link on behalf of the late-arriving cell.
                        // This is usually redundant, but if we don't attempt to set the late-arriving cell's link here,
                        // there's no guarantee that our own link chain will be well-formed by the time this function returns.
                        // (Indeed, subsequent lookups sometimes failed during testing, for this exact reason.)
                        quint8 desiredDelta = idx - prevLinkIdx;
                        prevLink->store(desiredDelta, Relaxed);
                        goto followLink; // Try to follow link chain for the bucket again.
                    }
                    // Continue linear search...
                }
                // Table is too full to insert.
                overflowIdx = idx + 1;
                return InsertResult_Overflow;
            }
        }
    }

    static void beginTableMigrationToSize(Map& map, Table* table, quint64 nextTableSize)
    {
        // Create new migration by DCLI.
        SimpleJobCoordinator::Job* job = table->jobCoordinator.loadConsume();
        if (job) {
            // new migration already exists
        } else {
            QMutexLocker guard(&table->mutex);
            job = table->jobCoordinator.loadConsume(); // Non-atomic would be sufficient, but that's OK.

            if (job) {
                // new migration already exists (double-checked)
            } else {
                // Create new migration.
                TableMigration* migration = TableMigration::create(map, 1);
                migration->m_unitsRemaining.storeNonatomic(table->getNumMigrationUnits());
                migration->getSources()[0].table = table;
                migration->getSources()[0].sourceIndex.storeNonatomic(0);
                migration->m_destination = Table::create(nextTableSize);
                // Publish the new migration.
                table->jobCoordinator.storeRelease(migration, &map.getGC());
            }
        }
    }

    static void beginTableMigration(Map& map, Table* table, quint64 overflowIdx)
    {
        // Estimate number of cells in use based on a small sample.
        quint64 sizeMask = table->sizeMask;
        quint64 idx = overflowIdx - CellsInUseSample;
        quint64 inUseCells = 0;
        for (quint64 linearProbesRemaining = CellsInUseSample; linearProbesRemaining > 0; linearProbesRemaining--) {
            CellGroup* group = table->getCellGroups() + ((idx & sizeMask) >> 2);
            Cell* cell = group->cells + (idx & 3);
            Value value = cell->value.load(Relaxed);
            if (value == Value(ValueTraits::Redirect)) {
                // Another thread kicked off the jobCoordinator. The caller will participate upon return.
                return;
            }
            if (value != Value(ValueTraits::NullValue))
                inUseCells++;
            idx++;
        }
        float inUseRatio = float(inUseCells) / CellsInUseSample;
        float estimatedInUse = (sizeMask + 1) * inUseRatio;
        quint64 nextTableSize = qMax(quint64(InitialSize), roundUpPowerOf2(quint64(estimatedInUse * 2)));
        beginTableMigrationToSize(map, table, nextTableSize);
    }
}; // Leapfrog

template <class Map>
bool Leapfrog<Map>::TableMigration::migrateRange(Table* srcTable, quint64 startIdx)
{
    quint64 srcSizeMask = srcTable->sizeMask;
    quint64 endIdx = qMin(startIdx + TableMigrationUnitSize, srcSizeMask + 1);
    // Iterate over source range.
    for (quint64 srcIdx = startIdx; srcIdx < endIdx; srcIdx++) {
        CellGroup* srcGroup = srcTable->getCellGroups() + ((srcIdx & srcSizeMask) >> 2);
        Cell* srcCell = srcGroup->cells + (srcIdx & 3);
        Hash srcHash;
        Value srcValue;
        // Fetch the srcHash and srcValue.
        for (;;) {
            srcHash = srcCell->hash.load(Relaxed);
            if (srcHash == KeyTraits::NullHash) {
                // An unused cell. Try to put a Redirect marker in its value.
                srcValue =
                    srcCell->value.compareExchange(Value(ValueTraits::NullValue), Value(ValueTraits::Redirect), Relaxed);
                if (srcValue == Value(ValueTraits::Redirect)) {
                    // srcValue is already marked Redirect due to previous incomplete migration.
                    break;
                }
                if (srcValue == Value(ValueTraits::NullValue)) {
                    break; // Redirect has been placed. Break inner loop, continue outer loop.
                }
                // Otherwise, somebody just claimed the cell. Read srcHash again...
            } else {
                // Check for deleted/uninitialized value.
                srcValue = srcCell->value.load(Relaxed);
                if (srcValue == Value(ValueTraits::NullValue)) {
                    // Try to put a Redirect marker.
                    if (srcCell->value.compareExchangeStrong(srcValue, Value(ValueTraits::Redirect), Relaxed)) {
                        break; // Redirect has been placed. Break inner loop, continue outer loop.
                    }

                    if (srcValue == Value(ValueTraits::Redirect)) {
                        // FIXME: I don't think this will happen. Investigate & change to assert
                        break;
                    }
                } else if (srcValue == Value(ValueTraits::Redirect)) {
                    // srcValue is already marked Redirect due to previous incomplete migration.
                    break;
                }

                // We've got a key/value pair to migrate.
                // Reserve a destination cell in the destination.
#ifdef SANITY_CHECK
                KIS_ASSERT_RECOVER_NOOP(srcHash != KeyTraits::NullHash);
                KIS_ASSERT_RECOVER_NOOP(srcValue != Value(ValueTraits::NullValue));
                KIS_ASSERT_RECOVER_NOOP(srcValue != Value(ValueTraits::Redirect));
#endif // SANITY_CHECK
                Cell* dstCell;
                quint64 overflowIdx;
                InsertResult result = insertOrFind(srcHash, m_destination, dstCell, overflowIdx);
                // During migration, a hash can only exist in one place among all the source tables,
                // and it is only migrated by one thread. Therefore, the hash will never already exist
                // in the destination table:
#ifdef SANITY_CHECK
                KIS_ASSERT_RECOVER_NOOP(result != InsertResult_AlreadyFound);
#endif // SANITY_CHECK
                if (result == InsertResult_Overflow) {
                    // Destination overflow.
                    // This can happen for several reasons. For example, the source table could have
                    // existed of all deleted cells when it overflowed, resulting in a small destination
                    // table size, but then another thread could re-insert all the same hashes
                    // before the migration completed.
                    // Caller will cancel the current migration and begin a new one.
                    return false;
                }
                // Migrate the old value to the new cell.
                for (;;) {
                    // Copy srcValue to the destination.
                    dstCell->value.store(srcValue, Relaxed);
                    // Try to place a Redirect marker in srcValue.
                    Value doubleCheckedSrcValue = srcCell->value.compareExchange(srcValue, Value(ValueTraits::Redirect), Relaxed);
#ifdef SANITY_CHECK
                    KIS_ASSERT_RECOVER_NOOP(doubleCheckedSrcValue != Value(ValueTraits::Redirect)); // Only one thread can redirect a cell at a time.
#endif // SANITY_CHECK
                    if (doubleCheckedSrcValue == srcValue) {
                        // No racing writes to the src. We've successfully placed the Redirect marker.
                        // srcValue was non-NULL when we decided to migrate it, but it may have changed to NULL
                        // by a late-arriving erase.
                        if (srcValue == Value(ValueTraits::NullValue)) {
                            // racing update was erase", uptr(srcTable), srcIdx)
                        }

                        break;
                    }
                    // There was a late-arriving write (or erase) to the src. Migrate the new value and try again.
                    srcValue = doubleCheckedSrcValue;
                }
                // Cell successfully migrated. Proceed to next source cell.
                break;
            }
        }
    }
    // Range has been migrated successfully.
    return true;
}

template <class Map>
void Leapfrog<Map>::TableMigration::run()
{
    // Conditionally increment the shared # of workers.
    quint64 probeStatus = m_workerStatus.load(Relaxed);
    do {
        if (probeStatus & 1) {
            // End flag is already set, so do nothing.
            return;
        }
    } while (!m_workerStatus.compareExchangeWeak(probeStatus, probeStatus + 2, Relaxed, Relaxed));
    // # of workers has been incremented, and the end flag is clear.
#ifdef SANITY_CHECK
    KIS_ASSERT_RECOVER_NOOP((probeStatus & 1) == 0);
#endif // SANITY_CHECK

    // Iterate over all source tables.
    for (quint64 s = 0; s < m_numSources; s++) {
        Source& source = getSources()[s];
        // Loop over all migration units in this source table.
        for (;;) {
            if (m_workerStatus.load(Relaxed) & 1) {
                goto endMigration;
            }
            quint64 startIdx = source.sourceIndex.fetchAdd(TableMigrationUnitSize, Relaxed);
            if (startIdx >= source.table->sizeMask + 1)
                break; // No more migration units in this table. Try next source table.
            bool overflowed = !migrateRange(source.table, startIdx);
            if (overflowed) {
                // *** FAILED MIGRATION ***
                // TableMigration failed due to destination table overflow.
                // No other thread can declare the migration successful at this point, because *this* unit will never complete,
                // hence m_unitsRemaining won't reach zero.
                // However, multiple threads can independently detect a failed migration at the same time.
                // The reason we store overflowed in a shared variable is because we can must flush all the worker threads before
                // we can safely deal with the overflow. Therefore, the thread that detects the failure is often different from
                // the thread
                // that deals with it.
                bool oldOverflowed = m_overflowed.exchange(overflowed, Relaxed);
                if (oldOverflowed) {
                    // race to set m_overflowed
                }

                m_workerStatus.fetchOr(1, Relaxed);
                goto endMigration;
            }

            qint64 prevRemaining = m_unitsRemaining.fetchSub(1, Relaxed);
#ifdef SANITY_CHECK
            KIS_ASSERT_RECOVER_NOOP(prevRemaining > 0);
#endif // SANITY_CHECK
            if (prevRemaining == 1) {
                // *** SUCCESSFUL MIGRATION ***
                // That was the last chunk to migrate.
                m_workerStatus.fetchOr(1, Relaxed);
                goto endMigration;
            }
        }
    }

endMigration:
    // Decrement the shared # of workers.
    probeStatus = m_workerStatus.fetchSub(2, AcquireRelease); // AcquireRelease makes all previous writes visible to the last worker thread.
    if (probeStatus >= 4) {
        // There are other workers remaining. Return here so that only the very last worker will proceed.
        return;
    }

    // We're the very last worker thread.
    // Perform the appropriate post-migration step depending on whether the migration succeeded or failed.
#ifdef SANITY_CHECK
    KIS_ASSERT_RECOVER_NOOP(probeStatus == 3);
#endif // SANITY_CHECK
    bool overflowed = m_overflowed.loadNonatomic(); // No racing writes at this point
    if (!overflowed) {
        // The migration succeeded. This is the most likely outcome. Publish the new subtree.
        m_map.publishTableMigration(this);
        // End the jobCoodinator.
        getSources()[0].table->jobCoordinator.end();
    } else {
        // The migration failed due to the overflow of the destination table.
        Table* origTable = getSources()[0].table;
        QMutexLocker guard(&origTable->mutex);
        SimpleJobCoordinator::Job* checkedJob = origTable->jobCoordinator.loadConsume();

        if (checkedJob != this) {
            // a new TableMigration was already started
        } else {
            TableMigration* migration = TableMigration::create(m_map, m_numSources + 1);
            // Double the destination table size.
            migration->m_destination = Table::create((m_destination->sizeMask + 1) * 2);
            // Transfer source tables to the new migration.
            for (quint64 i = 0; i < m_numSources; i++) {
                migration->getSources()[i].table = getSources()[i].table;
                getSources()[i].table = NULL;
                migration->getSources()[i].sourceIndex.storeNonatomic(0);
            }

            migration->getSources()[m_numSources].table = m_destination;
            migration->getSources()[m_numSources].sourceIndex.storeNonatomic(0);
            // Calculate total number of migration units to move.
            quint64 unitsRemaining = 0;
            for (quint64 s = 0; s < migration->m_numSources; s++) {
                unitsRemaining += migration->getSources()[s].table->getNumMigrationUnits();
            }

            migration->m_unitsRemaining.storeNonatomic(unitsRemaining);
            // Publish the new migration.
            origTable->jobCoordinator.storeRelease(migration, &m_map.getGC());
        }
    }

    // We're done with this TableMigration. Queue it for GC.
    m_map.getGC().enqueue(&TableMigration::destroy, this, true);
    m_map.getGC().update(m_map.migrationInProcess());
}

#endif // LEAPFROG_H
