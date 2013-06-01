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

#include "animator_dock.h"

#include <kis_view2.h>

#include <QLabel>

#include <klocale.h>
#include <kis_animation.h>
#include <kis_canvas2.h>
#include <kis_doc2.h>
#include <kis_part2.h>
#include <QListView>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QWidget>
#include "kis_timeline_view.h"
#include <QAction>
#include <KoIcon.h>
#include <QActionGroup>
#include <QPushButton>

AnimatorDock::AnimatorDock() : QDockWidget(i18n("Animator")), m_canvas(0), m_animation(0)
{
    this->setMinimumHeight(120);
    QWidget *mainWidget = new QWidget(this);
    QWidget* timelineWidget = new QWidget(mainWidget);
    QSize size(700, 220);

    size = size.expandedTo(minimumSizeHint());
    resize(size);

    m_fpsInput = new QSpinBox(mainWidget);
    m_fpsInput->setGeometry(850, 0, 50, 20);
    m_fpsInput->setRange(0,99);
    m_timeInput = new QSpinBox(mainWidget);
    m_timeInput->setGeometry(850, 30, 50, 20);
    m_timeInput->setRange(0,999999);
    QLabel* lblFpsInput = new QLabel(mainWidget);
    lblFpsInput->setGeometry(800, 0, 50, 20);
    lblFpsInput->setText("FPS:");
    QLabel* lblTimeInput = new QLabel(mainWidget);
    lblTimeInput->setText("Duration:");
    lblTimeInput->setGeometry(800, 30, 50, 20);

    QPushButton* addKeyFrameButton = new QPushButton(koIcon("list-add"), "", mainWidget);
    addKeyFrameButton->setGeometry(10,100, 30,30);
    addKeyFrameButton->setShortcut(tr("F5"));

    QPushButton* addBlankFrameButton = new QPushButton(koIcon("list-add"),"",mainWidget);
    addBlankFrameButton->setGeometry(40,100,30,30);
    addBlankFrameButton->setShortcut(tr("F6"));

    QPushButton* removeFrameButton = new QPushButton(koIcon("list-remove"),"", mainWidget);
    removeFrameButton->setGeometry(70, 100, 30, 30);
    removeFrameButton->setShortcut(tr("F7"));

    m_timelineView = new KisTimelineView(timelineWidget);
    m_timelineView->setGeometry(QRect(0, 0, 800, 100));

    connect(m_fpsInput, SIGNAL(valueChanged(int)), this, SLOT(updateNumberOfFrames()));
    connect(m_timeInput, SIGNAL(valueChanged(int)), this, SLOT(updateNumberOfFrames()));
    connect(addKeyFrameButton, SIGNAL(clicked()), m_timelineView, SLOT(addKeyFrame()));
    connect(addBlankFrameButton, SIGNAL(clicked()), m_timelineView, SLOT(addBlankFrame()));

    this->setWidget(mainWidget);
}

void AnimatorDock::setCanvas(KoCanvasBase *canvas){
    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    if(m_canvas && m_canvas->view() && m_canvas->view()->document() && m_canvas->view()->document()->documentPart()){
        m_animation = dynamic_cast<KisPart2*>(m_canvas->view()->document()->documentPart())->animation();
        if(m_animation){
            m_fpsInput->setValue(m_animation->fps());
            m_timeInput->setValue(m_animation->time());
            m_timelineView->setNumberOfFrames(m_animation->fps() * m_animation->time());
            m_timelineView->init();
        }
    }
}

void AnimatorDock::updateNumberOfFrames(){
    m_timelineView->setNumberOfFrames(m_fpsInput->value()*m_timeInput->value());
}
#include "animator_dock.moc"
