/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TIMELINE_RULER_HEADER_H
#define TIMELINE_RULER_HEADER_H

#include <QHeaderView>
#include <QScopedPointer>
#include "kis_action_manager.h"

class QPaintEvent;

class KisAnimTimelineTimeHeader : public QHeaderView
{
    Q_OBJECT
public:
    KisAnimTimelineTimeHeader(QWidget *parent = 0);
    ~KisAnimTimelineTimeHeader() override;

    void setPixelOffset(qreal offset);

    void setFramePerSecond(int fps);
    bool setZoom(qreal zoomLevel);
    qreal zoom();

    void setModel(QAbstractItemModel *model) override;

    void setActionManager(KisActionManager *actionManager);

    void mouseMoveEvent(QMouseEvent *e) override;

    int estimateFirstVisibleColumn();
    int estimateLastVisibleColumn();

protected:
    void mousePressEvent(QMouseEvent *e) override;

    void mouseReleaseEvent(QMouseEvent *e) override;

    void paintEvent(QPaintEvent *e) override;
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
    void paintSection1(QPainter *painter, const QRect &rect, int logicalIndex) const;
    void changeEvent(QEvent *event) override;

private:
    void updateMinimumSize();

    void paintSpan(QPainter *painter, int userFrameId,
                   const QRect &spanRect,
                   bool isIntegralLine,
                   bool isPrevIntegralLine,
                   QStyle *style,
                   const QPalette &palette,
                   const QPen &gridPen) const;

Q_SIGNALS:
    void sigInsertColumnLeft();
    void sigInsertColumnRight();
    void sigInsertMultipleColumns();

    void sigRemoveColumns();
    void sigRemoveColumnsAndShift();

    void sigInsertHoldColumns();
    void sigRemoveHoldColumns();
    void sigInsertHoldColumnsCustom();
    void sigRemoveHoldColumnsCustom();

    void sigMirrorColumns();

    void sigCutColumns();
    void sigCopyColumns();
    void sigPasteColumns();

    void sigZoomChanged(qreal zoom);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // TIMELINE_RULER_HEADER_H
