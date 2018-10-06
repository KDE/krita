/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_LOD_AVAILABILITY_WIDGET_H
#define __KIS_LOD_AVAILABILITY_WIDGET_H

#include <QWidget>
#include <QScopedPointer>

#include <brushengine/kis_paintop_lod_limitations.h>

class KoCanvasResourceProvider;


class KisLodAvailabilityWidget : public QWidget
{
    Q_OBJECT

public:
    KisLodAvailabilityWidget(QWidget *parent);
    ~KisLodAvailabilityWidget() override;

    void setCanvasResourceManager(KoCanvasResourceProvider *resourceManager);

    void setLimitations(const KisPaintopLodLimitations &l);

public Q_SLOTS:
    void slotUserChangedLodAvailability(bool value);
    void slotUserChangedLodThreshold(qreal value);
    void slotUserChangedSize(qreal value);

Q_SIGNALS:
    void sigUserChangedLodAvailability(bool value);
    void sigUserChangedLodThreshold(qreal value);

private Q_SLOTS:
    void showLodToolTip();
    void showLodThresholdWidget(const QPoint &pos);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_LOD_AVAILABILITY_WIDGET_H */
