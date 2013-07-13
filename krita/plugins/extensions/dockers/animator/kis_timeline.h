/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version..
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

#ifndef KIS_TIMELINE_H
#define KIS_TIMELINE_H

#include <QWidget>
#include <QToolButton>
#include <QScrollArea>
#include <QAction>

class KisFrameBox;
class KisAnimationLayerBox;
class KisCanvas2;
class KisAnimation;

class KisTimeline : public QWidget
{
    Q_OBJECT
public:
    KisTimeline(QWidget* parent = 0);
    void setCanvas(KisCanvas2* canvas);
    KisCanvas2* getCanvas();
    KisAnimationLayerBox* getLayerBox();
    KisFrameBox* getFrameBox();
    void setModel(KisAnimation* animation);
    bool scrubbing;

protected:
    void resizeEvent(QResizeEvent *event);

public:
    //QToolButton* m_addLayerButton;
    QAction* m_addPaintLayerAction;
    QAction* m_addVectorLayerAction;

public slots:
    void documentModified();

private:
    KisFrameBox* m_cells;
    int m_numberOfLayers;
    KisCanvas2* m_canvas;
    KisAnimationLayerBox *m_list;
    KisAnimation* m_animation;

private slots:
    void updateHeight();
    void blankFramePressed();
    void keyFramePressed();
    void addframePressed();
    void changeCanvas();

signals:
    void canvasModified();
};

#endif // KIS_TIMELINE_H
