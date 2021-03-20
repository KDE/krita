/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSafeNodeProjectionStore.h"

#include <QCoreApplication>
#include <QVector>
#include <KoColorSpace.h>

#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_selection.h"
#include "KisRecycleProjectionsJob.h"

/**********************************************************************/
/*     StoreImplementaionInterface                                    */
/**********************************************************************/

struct StoreImplementaionInterface
{
    virtual ~StoreImplementaionInterface() {}
    virtual StoreImplementaionInterface* clone() const = 0;
    virtual bool releaseDevice() = 0;
    virtual void discardCaches() = 0;
    virtual void recycleProjectionsInSafety() = 0;
};


/**********************************************************************/
/*     StoreImplementaion                                             */
/**********************************************************************/

template <typename DeviceSP>
struct StoreImplementation : public StoreImplementaionInterface
{
    bool releaseDevice() override {
        bool hasDeletedProjection = false;

        if (m_projection) {
            m_dirtyProjections.append(m_projection);
            m_projection = 0;
            hasDeletedProjection = true;

//            qDebug() << "release a device";
        }
        return hasDeletedProjection;
    }

    virtual void discardCaches() override {
//        qDebug() << "discard caches";
        m_dirtyProjections.clear();
    }

    virtual void recycleProjectionsInSafety() override {
//        qDebug() << "recycle caches";
        Q_FOREACH (DeviceSP projection, m_dirtyProjections) {
            projection->clear();
            m_cleanProjections.append(projection);
        }
        m_dirtyProjections.clear();
    }

protected:
    DeviceSP m_projection;
    QVector<DeviceSP> m_dirtyProjections;
    QVector<DeviceSP> m_cleanProjections;
};


/**********************************************************************/
/*     StoreImplementaionForDevice                                    */
/**********************************************************************/

struct StoreImplementationForDevice : StoreImplementation<KisPaintDeviceSP>
{
    StoreImplementationForDevice() {}
    StoreImplementationForDevice(const KisPaintDevice &prototype) {
        m_projection = new KisPaintDevice(prototype);
    }

    StoreImplementaionInterface* clone() const override {
        return m_projection ?
            new StoreImplementationForDevice(*m_projection) :
            new StoreImplementationForDevice();
    }

    KisPaintDeviceSP getDeviceLazy(KisPaintDeviceSP prototype) {
        if(!m_projection ||
           *m_projection->colorSpace() != *prototype->colorSpace()) {

            if (!m_cleanProjections.isEmpty()) {
                m_projection = m_cleanProjections.takeLast();
                m_projection->makeCloneFromRough(prototype, prototype->extent());
            } else {
                m_projection = new KisPaintDevice(*prototype);
            }

            m_projection->setProjectionDevice(true);
        }
        return m_projection;
    }
};


/**********************************************************************/
/*     StoreImplementaionForSelection                                 */
/**********************************************************************/

struct StoreImplementationForSelection : StoreImplementation<KisSelectionSP>
{
    StoreImplementationForSelection() {}
    StoreImplementationForSelection(const KisSelection &prototype) {
        m_projection = new KisSelection(prototype);
    }

    StoreImplementaionInterface* clone() const override {
        return m_projection ?
            new StoreImplementationForSelection(*m_projection) :
            new StoreImplementationForSelection();
    }

    KisSelectionSP getDeviceLazy(KisSelectionSP prototype) {
        if(!m_projection) {
            if (!m_cleanProjections.isEmpty()) {
                m_projection = m_cleanProjections.takeLast();
                m_projection->pixelSelection()->makeCloneFromRough(prototype->pixelSelection(), prototype->selectedRect());
            } else {
                m_projection = new KisSelection(*prototype);
            }

            m_projection->pixelSelection()->setProjectionDevice(true);
        }
        return m_projection;
    }
};


/**********************************************************************/
/*     KisSafeNodeProjectionStoreBase                                 */
/**********************************************************************/

struct KisSafeNodeProjectionStoreBase::Private
{
    mutable QMutex lock;
    KisImageWSP image;
    QScopedPointer<StoreImplementaionInterface> store;
};

KisSafeNodeProjectionStoreBase::KisSafeNodeProjectionStoreBase(StoreImplementaionInterface *storeImpl)
    : m_d(new Private)
{
    m_d->store.reset(storeImpl);
    moveToThread(qApp->thread());
    connect(this, SIGNAL(internalInitiateProjectionsCleanup()), this, SLOT(slotInitiateProjectionsCleanup()));
}

KisSafeNodeProjectionStoreBase::KisSafeNodeProjectionStoreBase(const KisSafeNodeProjectionStoreBase &rhs)
    : QObject(),
      KisShared(),
      m_d(new Private)
{
    {
        QMutexLocker rhsLocker(&rhs.m_d->lock);

        m_d->image = rhs.m_d->image;
        m_d->store.reset(rhs.m_d->store->clone());
    }

    moveToThread(qApp->thread());
    connect(this, SIGNAL(internalInitiateProjectionsCleanup()), this, SLOT(slotInitiateProjectionsCleanup()));
}

KisSafeNodeProjectionStoreBase::~KisSafeNodeProjectionStoreBase()
{
}

void KisSafeNodeProjectionStoreBase::releaseDevice()
{
    QMutexLocker locker(&m_d->lock);
    if (m_d->store->releaseDevice()) {
        locker.unlock();
        emit internalInitiateProjectionsCleanup();
    }
}

void KisSafeNodeProjectionStoreBase::setImage(KisImageWSP image)
{
    m_d->image = image;
}

void KisSafeNodeProjectionStoreBase::slotInitiateProjectionsCleanup()
{
    /**
     * After the projection has been used, we should clean it. But we cannot
     * clean it until all the workers accessing it have completed their job.
     *
     * Therefore we just schedule an exclusive job that will execute the
     * recycling action in an exclusive context, when no jobs are running.
     */

    KisImageSP image = m_d->image;

    if (image) {
        image->addSpontaneousJob(new KisRecycleProjectionsJob(this));
    } else {
        discardCaches();
    }
}

void KisSafeNodeProjectionStoreBase::discardCaches()
{
    QMutexLocker locker(&m_d->lock);
    m_d->store->discardCaches();
}

void KisSafeNodeProjectionStoreBase::recycleProjectionsInSafety()
{
    QMutexLocker locker(&m_d->lock);
    m_d->store->recycleProjectionsInSafety();
}


/**********************************************************************/
/*     KisSafeNodeProjectionStore                                     */
/**********************************************************************/

KisSafeNodeProjectionStore::KisSafeNodeProjectionStore()
    : KisSafeNodeProjectionStoreBase(new StoreImplementationForDevice)
{
}

KisSafeNodeProjectionStore::KisSafeNodeProjectionStore(const KisSafeNodeProjectionStore &rhs)
    : KisSafeNodeProjectionStoreBase(rhs)
{
}

KisPaintDeviceSP KisSafeNodeProjectionStore::getDeviceLazy(KisPaintDeviceSP prototype)
{
    QMutexLocker locker(&m_d->lock);
    StoreImplementationForDevice *store = dynamic_cast<StoreImplementationForDevice*>(m_d->store.data());
    KIS_ASSERT(store);

    return store->getDeviceLazy(prototype);
}


/**********************************************************************/
/*     KisSafeSelectionNodeProjectionStore                            */
/**********************************************************************/

KisSafeSelectionNodeProjectionStore::KisSafeSelectionNodeProjectionStore()
    : KisSafeNodeProjectionStoreBase(new StoreImplementationForSelection)
{
}

KisSafeSelectionNodeProjectionStore::KisSafeSelectionNodeProjectionStore(const KisSafeSelectionNodeProjectionStore &rhs)
    : KisSafeNodeProjectionStoreBase(rhs)
{
}

KisSelectionSP KisSafeSelectionNodeProjectionStore::getDeviceLazy(KisSelectionSP prototype)
{
    QMutexLocker locker(&m_d->lock);
    StoreImplementationForSelection *store = dynamic_cast<StoreImplementationForSelection*>(m_d->store.data());
    KIS_ASSERT(store);

    return store->getDeviceLazy(prototype);
}

