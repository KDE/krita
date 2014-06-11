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

#include "animator_settings_dialog.h"

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

protected:
    void resizeEvent(QResizeEvent *event);

private:
    void addLayerUiUpdate();
    void init();

public slots:
    void documentModified();

private:
    KisFrameBox* m_cells;
    int m_numberOfLayers;
    KisCanvas2* m_canvas;
    KisAnimationLayerBox *m_list;
    KisAnimation* m_animation;
    QRect m_lastBrokenFrame;
    bool m_frameBreakState;
    AnimatorSettingsDialog* m_settingsDialog;
    bool m_initialized;
    QWidget* m_parent;

private slots:
    void blankFramePressed();
    void keyFramePressed();
    void addframePressed();
    void frameSelectionChanged(QRect frame);
    void playAnimation();
    void pauseAnimation();
    void stopAnimation();
    void breakFrame(QRect position);
    void frameBreakStateChanged(bool state);
    void nextFramePressed();
    void prevFramePressed();
    void nextKeyFramePressed();
    void prevKeyFramePressed();
    void settingsButtonPressed();
    void timelineWidthChanged(int width);
    void paintLayerPressed();
    void vectorLayerPressed();
    void importUI(QList<QRect> timelineMap);

signals:
    void canvasModified();
};

#endif // KIS_TIMELINE_H
