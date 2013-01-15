/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Marc Pegon <pe.marc@free.fr>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_PERSPECTIVETRANSFORM_WORKER_H
#define KIS_PERSPECTIVETRANSFORM_WORKER_H

#include "kis_types.h"
#include "krita_export.h"

#include <QRect>
#include <QRegion>
#include <QTransform>


#include <KoUpdater.h>
typedef QPointer<KoUpdater> KoUpdaterPtr;

class KRITAIMAGE_EXPORT KisPerspectiveTransformWorker : public QObject
{

    Q_OBJECT

public:
    //KisPerspectiveTransformWorker(KisPaintDeviceSP dev, KisSelectionSP selection, const QPointF& topLeft, const QPointF& topRight, const QPointF& bottomLeft, const QPointF& bottomRight, KoUpdaterPtr progress);
    KisPerspectiveTransformWorker(KisPaintDeviceSP dev, QPointF center, double aX, double aY, double distance, KoUpdaterPtr progress);

    ~KisPerspectiveTransformWorker();

    void run();
private:
    KisPaintDeviceSP m_dev;
    KoUpdaterPtr m_progressUpdater;
    QRegion m_dstRegion;
    QRectF m_srcRect;
    QTransform m_newTransform;
    bool m_isIdentity;
};

#endif
