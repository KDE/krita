/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
