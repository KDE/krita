/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISSAFENODEPROJECTIONSTORE_H
#define KISSAFENODEPROJECTIONSTORE_H

#include <QObject>

#include <QMutex>
#include <QScopedPointer>

#include "kritaimage_export.h"
#include "kis_shared.h"
#include "kis_types.h"
#include "kis_image_interfaces.h"

struct StoreImplementaionInterface;

class KRITAIMAGE_EXPORT KisSafeNodeProjectionStoreBase : public QObject, public KisShared
{
    Q_OBJECT

public:
    KisSafeNodeProjectionStoreBase(const KisSafeNodeProjectionStoreBase &rhs);
    ~KisSafeNodeProjectionStoreBase();

    /**
     * Notify the the store that current projection device can be
     * safely dropped. The store will try to recycle the device
     * (taking ABA problem into account).
     */
    void releaseDevice();

    void setImage(KisImageWSP image);

Q_SIGNALS:
    void internalInitiateProjectionsCleanup();

private Q_SLOTS:
    void slotInitiateProjectionsCleanup();

protected:
    KisSafeNodeProjectionStoreBase(StoreImplementaionInterface *storeImpl);

private:
    void discardCaches();

    friend class KisRecycleProjectionsJob;
    void recycleProjectionsInSafety();

protected:
    struct Private;
    QScopedPointer<Private> m_d;
};

class KRITAIMAGE_EXPORT KisSafeNodeProjectionStore : public KisSafeNodeProjectionStoreBase
{
public:
    KisSafeNodeProjectionStore();
    KisSafeNodeProjectionStore(const KisSafeNodeProjectionStore &rhs);

    /**
     * Safely fetch the current node projection device. If projection
     * already exists, the existing device is returned. Otherwise a new
     * device is created as a full copy of \p prototype.
     */
    KisPaintDeviceSP getDeviceLazy(KisPaintDeviceSP prototype);
};

class KRITAIMAGE_EXPORT KisSafeSelectionNodeProjectionStore : public KisSafeNodeProjectionStoreBase
{
public:
    KisSafeSelectionNodeProjectionStore();
    KisSafeSelectionNodeProjectionStore(const KisSafeSelectionNodeProjectionStore &rhs);

    /**
     * Safely fetch the current node projection device. If projection
     * already exists, the existing device is returned. Otherwise a new
     * device is created as a full copy of \p prototype.
     */
    KisSelectionSP getDeviceLazy(KisSelectionSP prototype);
};


typedef KisSharedPtr<KisSafeNodeProjectionStoreBase> KisSafeNodeProjectionStoreBaseSP;
typedef KisWeakSharedPtr<KisSafeNodeProjectionStoreBase> KisSafeNodeProjectionStoreBaseWSP;

typedef KisSharedPtr<KisSafeNodeProjectionStore> KisSafeNodeProjectionStoreSP;
typedef KisWeakSharedPtr<KisSafeNodeProjectionStore> KisSafeNodeProjectionStoreWSP;

typedef KisSharedPtr<KisSafeSelectionNodeProjectionStore> KisSafeSelectionNodeProjectionStoreSP;
typedef KisWeakSharedPtr<KisSafeSelectionNodeProjectionStore> KisSafeSelectionNodeProjectionStoreWSP;

#endif // KISSAFENODEPROJECTIONSTORE_H
