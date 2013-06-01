/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
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

#ifndef KIS_TIMELINE_VIEW_H
#define KIS_TIMELINE_VIEW_H

#include <QGraphicsView>
#include <QList>
#include <QString>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QDebug>
#include <math.h>
#include <QKeyEvent>
#include "kis_timeline.h"
#include "kis_animation_frame.h"

class KisTimelineView : public QGraphicsView
{
    Q_OBJECT
public:
    KisTimelineView(QWidget* parent);
    ~KisTimelineView();
    QGraphicsScene *m_timelineScene;
    KisTimeline* m_tl;
    QSize sizeHint() const;
    void init();

private:
    void removePreviousSelection();
    KisAnimationFrame* m_selectedFrame;

public:
    int m_numberOfFrames;

public slots:
    void setNumberOfFrames(int val);
    void addKeyFrame();
    void addBlankFrame();

protected:
    void mousePressEvent(QMouseEvent *event);

signals:

};

#endif // KIS_TIMELINE_VIEW_H
