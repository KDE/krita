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

#ifndef TIMELINE_RULER_HEADER_H
#define TIMELINE_RULER_HEADER_H

#include <QHeaderView>
#include <QScopedPointer>
#include "kis_action_manager.h"

class QPaintEvent;

class TimelineRulerHeader : public QHeaderView
{
    Q_OBJECT
public:
    TimelineRulerHeader(QWidget *parent = 0);
    ~TimelineRulerHeader() override;

    void setFramePerSecond(int fps);
    bool setZoom(qreal zoomLevel);

    void setModel(QAbstractItemModel *model) override;

    void setActionManager(KisActionManager *actionManager);

     void mouseMoveEvent(QMouseEvent *e) override;

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



private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // TIMELINE_RULER_HEADER_H
