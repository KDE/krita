/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org
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
#ifndef KIS_PROJECTION
#define KIS_PROJECTION

#include <QObject>
#include <QThread>

#include "kis_shared.h"
#include "kis_types.h"
#include "krita_export.h"

class QRegion;
class QRect;


/**
 * A thread that owns an updater object; the object is
 * connected to a KisImage instance. Every time we need
 * an update, a signal is thrown. The event loop of this
 * thread delivers the signal to the updater, and when
 * the updater is done, it delivers another signal to KisImage.
 */
class KRITAIMAGE_EXPORT KisProjection : public QThread
{

    Q_OBJECT

public:

    KisProjection(KisImageWSP image);
    virtual ~KisProjection();

    virtual void run();

    void lock();
    void unlock();

    /**
     * called from the main thread, this divides rc in chunks and emits a signal
     * for KisImageUpdater to catch. KisImageUpdater belongs to this thread.
     */
    void updateProjection(KisNodeSP node, const QRect& rc);
    void setRegionOfInterest(const QRect & roi);
    void updateSettings();
    void stop();

signals:

    void sigUpdateProjection(KisNodeSP node, const QRect& rc);

private:

    KisProjection(const KisProjection &);
    KisProjection & operator=(const KisProjection&);

    class Private;
    Private * const m_d;

};

/**
 * while travelling down to the rootlayer, we update the projection
 * of every node.
 */
class KisImageUpdater : public QObject
{
    Q_OBJECT

public slots:

    void startUpdate(KisNodeSP node, const QRect& rc);

signals:

    void updateDone(const QRect& rc);

private:

    void update(KisNodeSP node, KisNodeSP child, const QRect& rc);

};


#endif
