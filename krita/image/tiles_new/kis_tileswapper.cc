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

#include "kis_tileswapper.h"

#include <QCoreApplication>
#include <QMutexLocker>
#include <QEvent>

#include <k3staticdeleter.h>
#include <kglobal.h>
#include <ksharedconfig.h>

#include <kis_debug.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <cassert>


#include "kis_tileddatamanager.h"
#include "kis_tile.h"
#include "kis_tilestorememory.h"


KisTileSwapper* KisTileSwapper::m_singleton = 0;
static K3StaticDeleter<KisTileSwapper> staticDeleter;

// If not defined, it does not sleep, and keeps swapping or so (### Only when swap pressure would be too high) ### Implement this code path
#define SWAPMECHANICS_CHECK_AGE

/*
   Currently, we mutex the creation of new files.
*/

static QMutex tempMutex;

/*
### SPECIAL LOCKING POLICY For isInSwappableList
*/

/*
    If a tile is in the swaplist, also add one to the 'locked in memory', so that we don't delete it (this hopefully
    avoids a deadlock)
*/

KisTileSwapper::KisTileSwapper()
    : m_swapQueueLock(QMutex::Recursive)
{
    kDebug()  << QThread::currentThread();
    Q_ASSERT(KisTileSwapper::m_singleton == 0);
    //KisTileSwapper::m_singleton = this;
    m_swapForbidden = false;
    m_stopThread = false;
}

KisTileSwapper::~KisTileSwapper() {
    kDebug() << "";
    // Wait on ourself to stop...(this is called from the MAIN THREAD)
    m_stopThread = true;
    m_waitLock.lock();
    m_waitCondition.wakeAll();
    m_waitLock.unlock();
    wait(); // ### Right?

     // ### DELETE SWAP FILES AND CLEANUP IN GENERAL!
}

KisTileSwapper* KisTileSwapper::instance() {
    tempMutex.lock(); // ### Look at the C++ book from Alexandrescu(?) for the 3-locking thing for singletons
    if(KisTileSwapper::m_singleton == 0) {
        staticDeleter.setObject(KisTileSwapper::m_singleton, new KisTileSwapper()); // threadsafe?
        Q_CHECK_PTR(KisTileSwapper::m_singleton);
        m_singleton->start();
    }
    tempMutex.unlock();

    return KisTileSwapper::m_singleton;
}

void KisTileSwapper::run() {
    forever {
        if (m_stopThread) // No locking, since it only changes from false to true
            return;
        // AGRESSIVE SWAPPER! Loops until everything is swapped, and wakes as soon as something is swappable! (Hopefully, there's the chance of a signal happening
        // between queue unlock and waitlock ###)
        // ### Less agressive swapper would be problematic atm: If we signal during the swapping, it is lost!

        // There are new items in the workqueue, hopefully
        m_swapQueueLock.lock();
        KisTile::SharedTileData* tileData = 0;
        if (!m_swapList.empty()) {
            tileData = m_swapList.front();
            m_swapQueueLock.unlock(); // First unlock list, then lock tiledata: no deadlock (is legal, since we are locked in mem)

            tileData->lock.lock(); // No mutex locker here: we'd otherwise SLEEP in it!

            KisTileStoreMemory::SharedDataMemoryInfo* memInfo
                    = dynamic_cast<KisTileStoreMemory::SharedDataMemoryInfo*>(tileData->storeData);

            if ( (tileData->tiles.size() == 0) && (tileData->timesLockedInMemory == 0) ) {
                // The swap list was the last reference to this tile info, delete it
                // TODO: When we add the m_store to the tileData, call deregister here!
                tileData->lock.unlock();
                fromSwappableList(tileData);
                delete tileData;
                continue;
            }

            /*
              The front element is the oldest element. If the oldest element is too young, we have to go to sleep at least as long as its age, and then check again
              to see if, in the case it's still there, to swap it
            */
            
            unsigned long toSleepMSec = shouldSleepAmountmsecs(tileData);

            if (toSleepMSec > 0) { // ### FUZZIFY?
                tileData->lock.unlock();
                QThread::msleep(toSleepMSec); // LOCKING? (### WHAT IF WE'RE BEING DESTROYED? -> TOO SLOW)
                // tileData is still (possibly) in the swap queue at this point, since we never removed it from it
            } else {
                swapTileData(tileData); // ### This takes too long inside the queuelock! But it'd be badly locked otherwise? (STILL BADLY LOCKED) (removes the tileData from swappableList)
                tileData->lock.unlock();
            }
        } else {
            m_swapQueueLock.unlock();

            m_waitLock.lock();
            m_waitCondition.wait(&m_waitLock);
            m_waitLock.unlock();
        }
    }
}

KisTile::TimeDiffType KisTileSwapper::idleThreshold() {
    return 500; // ###
}

void KisTileSwapper::enqueueForSwapping(KisTile::SharedTileData* tileData) {
    QMutexLocker dataLock(&(tileData->lock));

    KisTileStoreMemory::SharedDataMemoryInfo* memInfo = dynamic_cast<KisTileStoreMemory::SharedDataMemoryInfo*>(tileData->storeData);

    Q_ASSERT(memInfo->isSwappable);

    if (memInfo->isInSwappableList)
        return; // Be friendly

    Q_ASSERT(tileData->timesLockedInMemory == 0);
    tileData->timesLockedInMemory = 1;

    m_swapList.push_back(tileData);

    // This has to be done after the adding to the list, of course!
    memInfo->node = --(m_swapList.end());
    memInfo->isInSwappableList = true;

    // ### HELP! If we wake here, we still have the tileData lock (recursively, even), does this then deadlock?
    m_waitLock.lock();
    m_waitCondition.wakeAll();
    m_waitLock.unlock();
}


// ### ACTUALLY! Can we multithread file reads (whilst ftruncating, in particular)???
void KisTileSwapper::addTileDataToSwapFile(KisTile::SharedTileData* tileData) { // LOCKED
    KisTileStoreMemory::SharedDataMemoryInfo* memInfo = dynamic_cast<KisTileStoreMemory::SharedDataMemoryInfo*>(tileData->storeData);

    // This tile is not yet in the file. Save it there
    bool foundFree = false;
    FreeInfo freeInfo;

    // tileData should be locked already

    m_freeListsMutex.lock();
    if (m_freeLists.contains(tileData->tileSize)) {
        FreeList& freeList = m_freeLists[tileData->tileSize];
        if (!freeList.empty()) {
            freeInfo = freeList.back();
            freeList.pop_back();
            foundFree = true;
        }
    }
    m_freeListsMutex.unlock();

    if (foundFree) {
        // Found a position
        memInfo->file = freeInfo.tempFile->tempFile;
        memInfo->filePos = freeInfo.filePos;
    } else {
        // No position found or free, create a new
        long pageSize;
#ifdef Q_WS_WIN
        SYSTEM_INFO systemInfo;
        GetSystemInfo(&systemInfo);
        pageSize = systemInfo.dwPageSize;
#else
        pageSize = sysconf(_SC_PAGESIZE);
#endif

        // For each tilesize, there's a vector of files. Either this has a file at it's back with room for one more tile, or it has not.
        TempFile* tempFile = 0;

        TempFileVector* tempFiles = 0;
        m_filesMutex.lock();
        if (m_files.contains(tileData->tileSize)) {
            tempFiles = m_files[tileData->tileSize];
        } else {
            tempFiles = new TempFileVector();
            m_files[tileData->tileSize] = tempFiles;
        }
        m_filesMutex.unlock();

        Q_ASSERT(tempFiles);

        tempFiles->mutex.lock();
        if (tempFiles->vector.empty() || tempFiles->vector.back()->fileSize + tileData->tileSize >= MaxSwapFileSize) {
            // There's no file in this vector, or it would be too big. In either case, we have to add a new tempfile for this size. Keep it locked.
            tempFile = new TempFile();

            tempFile->tempFile = new KTemporaryFile();
            tempFile->tempFile->setAutoRemove(false);
            tempFile->tempFile->open();
            tempFile->fileSize = 0;

            kDebug() << "Added KTemporaryFile at " << tempFile->tempFile->fileName();

            tempFiles->vector.push_back(tempFile);
        } else {
            // The back vector here has a file that is small enough, use it!
            tempFile = tempFiles->vector.back();
        }

        tempFile->mutex.lock(); // ### Deadlock with tempFiles lock? Check!

        // We don't unlock yet! We will unlock after the fileSize has been updated!

        off_t oldSize = tempFile->fileSize;
        off_t newSize = oldSize + tileData->tileSize;
        if (newSize % pageSize > 0) {
            newSize += pageSize - (newSize % pageSize);
            Q_ASSERT(newSize % pageSize == 0);
        }

        tempFile->fileSize = newSize;

        tempFiles->mutex.unlock();

        // Now resize the file
        if (ftruncate(tempFile->tempFile->handle(), newSize)) {
            ftruncateError(errno, oldSize, newSize, tileData->tileSize, tempFile);
            tempFile->mutex.unlock(); // ### Deadlock with tempFiles lock? Check!
            return;
        }

        tempFile->mutex.unlock(); // ### Deadlock with tempFiles lock? Check!

        memInfo->file = tempFile->tempFile;
        memInfo->filePos = oldSize;
    }

    // Don't yet set the onFile or mmap &co properties.
}


unsigned long KisTileSwapper::shouldSleepAmountmsecs(KisTile::SharedTileData* tileData) {
    // ASSUMPTION: Queue is still locked so it can't get pulled away under our feet! ### tileData also locked
    // Not old enough?
    KisTile::TimeDiffType age = tileData->lastUse.msecsTo(QTime::currentTime());

    if (age > idleThreshold()) {
        return 0;
    } else {
        return idleThreshold() - age;
    }
}

void KisTileSwapper::swapTileData(KisTile::SharedTileData* tileData) { // LOCKED
    /* Read comments at the beginning of swapTilesInStore*/

    Q_ASSERT(tileData);

    KisTileStoreMemory::SharedDataMemoryInfo* memInfo = dynamic_cast<KisTileStoreMemory::SharedDataMemoryInfo*>(tileData->storeData);

    if (memInfo->isInSwappableList) {
        fromSwappableList(tileData);
    }

    // ### Compress first? Perhaps not?

    // Do the swap dance

    // If this tile has not yet been assigned a place in a file, do that first
    // This information can be gotten lockless (### make sure)
    if (!memInfo->onFile) {
        addTileDataToSwapFile(tileData);

        QFile* file = memInfo->file;
        if(!file) {
            kWarning() << "Opening the file as QFile failed";
            m_swapForbidden = true;
            return;
        }

        int fd = file->handle();
        quint8* data = 0;
        // ### TODO We could perhaps fwrite directly -> faster???
        if (!kritaMmap(data, 0, tileData->tileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, memInfo->filePos)) {
                 kWarning() << "Initial mmap failed";
                 memInfo->onFile = false;
                 m_swapForbidden = true;
                 return;
        }

        memcpy(data, tileData->data, tileData->tileSize);
        munmap(data, tileData->tileSize);

             // ### tile pools
        delete[] tileData->data;

        tileData->data = 0;
    } else {
        Q_ASSERT(memInfo->onFile);
        Q_ASSERT(memInfo->inMem);

        // munmap it
        munmap(tileData->data, tileData->tileSize);
        tileData->data = 0;
    }

    /* Update the information */
    memInfo->inMem = false;
    memInfo->onFile = true;
    memInfo->isSwappable = false;

/*
    m_currentInMem--;
    m_bytesInMem -= info->size;
*/
}

void KisTileSwapper::fromSwap(KisTile::SharedTileData* tileData)
{
    QMutexLocker dataLock(&(tileData->lock)); // ### Reeds locken?

    KisTileStoreMemory::SharedDataMemoryInfo* memInfo = dynamic_cast<KisTileStoreMemory::SharedDataMemoryInfo*>(tileData->storeData);

    if (memInfo->inMem) {
        return;
    }

    Q_ASSERT(memInfo->onFile);
    Q_ASSERT(memInfo->file);
    Q_ASSERT(!memInfo->inMem);

    if (!kritaMmap(tileData->data, 0, tileData->tileSize, PROT_READ | PROT_WRITE, MAP_SHARED, memInfo->file->handle(), memInfo->filePos)) {
        kWarning() << "fromSwap failed!";
        return;
    }

    memInfo->inMem = true;

    //m_currentInMem++;
    //m_bytesInMem += info->size;
}

void KisTileSwapper::fromSwappableList(KisTile::SharedTileData* tileData) { // ### Special Locking policy
    KisTileStoreMemory::SharedDataMemoryInfo* memInfo = dynamic_cast<KisTileStoreMemory::SharedDataMemoryInfo*>(tileData->storeData);

    assert(memInfo->isInSwappableList);
    Q_ASSERT(memInfo->isInSwappableList);
    Q_ASSERT(memInfo->isSwappable);
    kDebug() << tileData->timesLockedInMemory;
    // It's per definition 1 or 2 at this point: if it is added to the swappablelist, it is forced to 1, if we no load it again ->
    // + 1, but if we now want to delete it or actually swap it out, it's still 0
    Q_ASSERT(tileData->timesLockedInMemory == 1 || tileData->timesLockedInMemory == 2);
    tileData->timesLockedInMemory--;

    // Do a tryLock here? ###
    memInfo->isInSwappableList = false;

    m_swapQueueLock.lock();
    m_swapList.remove(memInfo->node);
    m_swapQueueLock.unlock();
}

void KisTileSwapper::ftruncateError(int errorNumber, off_t oldSize, off_t newSize, int tileSize, TempFile* tempFile) {
    // XXX make these maybe i18n()able and in an error box, but then through
    // some kind of proxy such that we don't pollute this with GUI code
    kWarning(DBG_AREA_TILES) << "Resizing the temporary swapfile failed!";

    // Be somewhat polite and try to figure out why it failed
    switch (errorNumber) {
        case EIO:
            kWarning(DBG_AREA_TILES) << "Error was E IO,"
                                        " possible reason is a disk error!";
            break;
        case EINVAL: kWarning(DBG_AREA_TILES) << "Error was E INVAL,"
                                                 " possible reason is that you are using more memory than "
                                                 " the filesystem or disk can handle";
            break;
        default:
            kWarning(DBG_AREA_TILES) << "Errno was:" << errno;
    }

    kWarning(DBG_AREA_TILES) << "The swapfile is:" << tempFile->tempFile->fileName();
    kWarning(DBG_AREA_TILES) << "Will try to avoid using the swap any further";

    dbgTiles << " Failed ftruncate info:"
            "tried adding " << tileSize << " bytes "
            "(rounded to pagesize: " << newSize << ") "
            "to a" << tempFile->fileSize << "bytes file (after adding)";

#ifndef DO_NOT_PRINT_INFO
            //printInfo();
#endif

    m_swapForbidden = true;
}

bool KisTileSwapper::kritaMmap(quint8*& result, void *start, size_t length,
                               int prot, int flags, int fd, off_t offset) {
    result = (quint8*) mmap(start, length, prot, flags, fd, offset);

    // Same here for warning and GUI
    if (result == (quint8*)-1) {
        kWarning(DBG_AREA_TILES) << "mmap failed: errno is" << errno << "; we're probably going to crash very soon now...";

        // Try to ignore what happened and carry on, but unlikely that we'll get
        // much further, since the file resizing went OK and this is memory-related...
        if (errno == ENOMEM) {
            kWarning(DBG_AREA_TILES) << "mmap failed with E NOMEM! This means that"
                    << "either there are no more memory mappings available for Krita, "
                    << "or that there is no more memory available!" << endl;
        }

        kWarning(DBG_AREA_TILES) << "Trying to continue anyway (no guarantees)";
        kWarning(DBG_AREA_TILES) <<" Will try to avoid using the swap any further";
        dbgTiles <<" Failed mmap info:"
                << "tried mapping " << length << " bytes" << endl;
        /*if (!m_files.empty()) {
            dbgTiles << "Probably to a" << m_files.back().fileSize << " bytes file";
        }*/
        // printInfo(); ### DO IT!

        assert(false);

        // Be nice
        result = 0;

        return false;
    }

    return true;
}

#include "kis_tileswapper.moc"

