/*
 *  Copyright (c) 2007 Bart Coppens <kde@bartcoppens.be>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_TILESWAPPER_H_
#define KIS_TILESWAPPER_H_

#include <KTemporaryFile>
#include <QWaitCondition>
#include <QLinkedList>
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QVector>
#include <QTimer>
#include <QMap>

#include "kis_tile.h"
#include "kis_tilestorememory.h"
#include "kis_sharedtiledata.h"

/**
 * An idle swapper. This class is responsible for the management of the files that the tiles of individual KisTileStoreMemories are swapped to.
 *
 * How the tiledata is mapped to the files is an implementational detail of this class, with some part of these details being shared with the TileStoreMemory.
 * In the current case, we map each Tile of a certain tileSize to a file. When such a file is full, we add a new one for the tileSize.
 * This is a break with Krita 1.5+ which shared swapfiles for all tilesizes -> there was the possibility of swapfile fragmentation.
 *
 * The 'one swapfile per paintdev' sounds a bit risky, since having a swapfile per paintdev -> lots of swapfiles -> problems with Qt possibly (Qt uses
 * select(2) for all kinds of stuff, which by default will break if >= 1024 file descriptors are in use.
 *
 * TODO: add something to a generic paintdevice that it should be considered 'volatile', i.e. will be used so shortly that swapping or compressing it is useless
 * (think paintbrush dabs).
 */
class KRITAIMAGE_EXPORT KisTileSwapper : public QThread {
    Q_OBJECT
public:
    ~KisTileSwapper();
    static KisTileSwapper* instance();

    void enqueueForSwapping(KisSharedTileData* tileData);
    void fromSwap(KisSharedTileData* tileData); // Locked tile with a locked memInfo!

    void fromSwappableList(KisSharedTileData* tileData); // A locked memInfo!
private:
    QMutex m_mutex;

private:
    static KisSharedTileData::TimeDiffType idleThreshold();
    void addTileDataToSwapFile(KisSharedTileData* tileData); // Locked tileData!
    unsigned long shouldSleepAmountmsecs(KisSharedTileData* tileData);
    void swapTileData(KisSharedTileData* tileData);

protected:
    void run();

private: // File handling
    QMutex m_swapQueueLock;
    QLinkedList<KisSharedTileData*> m_swapList;

    // This keeps track of open swap files, and their associated filesizes
    struct TempFile {
        QMutex mutex;
        KTemporaryFile* tempFile;
        off_t fileSize;
    };

    struct TempFileVector {
        QMutex mutex;
        QVector<TempFile*> vector;
    };

    typedef QMap<int, TempFileVector*> TempFileVectorMap; // maps tileSize to a swapfile

    //typedef QVector<TempFile> FileList;
    //FileList m_files;

    QMutex m_filesMutex;
    TempFileVectorMap m_files;

    // For use when any swap-allocating function failed; the risk of swap allocating failing
    // again is too big, and we'd clutter the logs with kWarnings otherwise
    bool m_swapForbidden;

    bool m_stopThread;

    // Information about a freelist entry in a file (we 'could' store this in the file, given proper locking there, esp. if 1 file/tilesize)
    typedef struct {
        TempFile* tempFile; // We need to lock it
        off_t filePos;
    } FreeInfo;

    typedef QVector<FreeInfo> FreeList; // No list: with 1 file per tileSize we can just push & pop ### weird naming!
    typedef QMap<int, FreeList> FreeListMap; // maps tileSize to a freelist

    QMutex m_freeListsMutex;
    FreeListMap m_freeLists;

    // This is the constant that we will use to see if we want to add a new tempfile
    // We use 1<<30 (one gigabyte) because apparently 32bit systems don't really like very
    // large files.
    static const long MaxSwapFileSize = 1<<30; // For debugging purposes: 1<<20 is a megabyte

    void ftruncateError(int errorNumber, off_t oldSize, off_t newSize, int tileSize, TempFile* tempFile);

    // Mmap wrapper that prints warnings on error. The result is stored in the *& result
    // the return value is true on succes, false on failure. Other args as in man mmap
    bool kritaMmap(quint8*& result, void *start, size_t length,
                   int prot, int flags, int fd, off_t offset);

private: // Actual Operation
    QMutex m_waitLock;
    QWaitCondition m_waitCondition;

private:
    KisTileSwapper();
    KisTileSwapper(KisTileSwapper&); // {}
    KisTileSwapper operator=(const KisTileSwapper&);

    static KisTileSwapper *m_singleton;
};

#endif // KIS_TILESWAPPER_H_
