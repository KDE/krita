/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#ifndef _TIMELINE_WIDGET_H_
#define _TIMELINE_WIDGET_H_

#include <QWidget>
#include <QAbstractItemModel>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QVBoxLayout>
#include "kis_types.h"
#include "timeline_view.h"

class KisCanvas2;

class TimelineWidget : public QWidget
{
    Q_OBJECT

public:
    TimelineWidget(QWidget *parent);

    void setModel(QAbstractItemModel *model);
    void setCanvas(KisCanvas2 *canvas);

    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

    void paintEvent(QPaintEvent *e);

private slots:
    void imageTimeChanged();
    void cacheChanged();

private:
    void scrub(QMouseEvent *e, bool preview);

    QVBoxLayout *m_layout;
    TimelineView *m_timelineView;

    KisCanvas2 *m_canvas;
    KisImageWSP m_image;
    KisAnimationFrameCacheSP m_cache;

    void drawRuler(QPainter &painter, QPaintEvent *e, bool dark);
    void drawPlayhead(QPainter &painter, QPaintEvent *e, bool dark);
};

#endif
