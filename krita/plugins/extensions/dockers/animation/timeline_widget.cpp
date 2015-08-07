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

#include "timeline_widget.h"

#include <QPainter>
#include <QHeaderView>
#include <QScrollBar>
#include <QSplitter>

#include "kis_image.h"
#include "kis_canvas2.h"
#include "kis_image_animation_interface.h"
#include "kis_animation_frame_cache.h"
#include "kis_animation_player.h"

TimelineWidget::TimelineWidget(QWidget *parent)
    : QWidget(parent)
    , m_layerTree(new QTreeView(this))
    , m_timelineView(new TimelineView(this, this))
    , m_canvas(0)
    , m_image(0)
{
    m_layout = new QVBoxLayout(this);
    QSplitter *splitter = new QSplitter(this);

    // Leave space at top for the ruler
    m_layout->insertSpacing(0, 16);

    m_layout->addWidget(splitter);
    splitter->addWidget(m_layerTree);
    splitter->addWidget(m_timelineView);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 6);

    m_layerTree->setFrameShape(QFrame::NoFrame);
    m_layerTree->viewport()->setAutoFillBackground(false);
    m_timelineView->setFrameShape(QFrame::NoFrame);
    m_timelineView->viewport()->setAutoFillBackground(false);

    m_layerTree->header()->hide();
    m_layerTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_layerTree->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    connect(m_timelineView->verticalScrollBar(), SIGNAL(valueChanged(int)),
            m_layerTree->verticalScrollBar(), SLOT(setValue(int)));
    connect(m_layerTree->verticalScrollBar(), SIGNAL(valueChanged(int)),
            m_timelineView->verticalScrollBar(), SLOT(setValue(int)));

    connect(m_layerTree, SIGNAL(collapsed(QModelIndex)), m_timelineView, SLOT(collapse(QModelIndex)));
    connect(m_layerTree, SIGNAL(expanded(QModelIndex)), m_timelineView, SLOT(expand(QModelIndex)));

    connect(splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(update()));
}

void TimelineWidget::paintEvent(QPaintEvent *e)
{
    if (!m_image) return;

    QPainter painter(this);

    QColor background = palette().background().color();
    bool dark = background.value() > 100;

    drawRuler(painter, e, dark);
    drawPlayhead(painter, e, dark);
}

void TimelineWidget::drawRuler(QPainter &painter, QPaintEvent *e, bool dark)
{
    Q_UNUSED(dark);

    int t0 = positionToTime(e->rect().left());
    int t1 = positionToTime(e->rect().right());

    painter.setPen(QPen(QColor(128, 128, 128, 128)));
    for (int t=t0; t<=t1; t++) {
        int x = timeToPosition(t);
        int xNext = timeToPosition(t + 1);
        if (!m_timelineView->isWithingView(t)) continue;

        if (t % 10 == 0) {
            painter.drawLine(x, e->rect().top(), x, e->rect().bottom());
        } else {
            painter.drawLine(x, 4, x, 12);
        }

        if (m_cache->frameStatus(t) == KisAnimationFrameCache::Cached) {
            painter.fillRect(QRect(x, 0, xNext - x, 4), QColor(128, 128, 128, 128));
        }
    }
}

void TimelineWidget::drawPlayhead(QPainter &painter, QPaintEvent *e, bool dark)
{
    int time = m_canvas->animationPlayer()->isPlaying() ?
                m_canvas->animationPlayer()->currentTime() :
                m_image->animationInterface()->currentUITime();

    int x = timeToPosition(time) + 4;
    if (m_timelineView->isWithingView(time)) {
        painter.setPen(QPen(
            dark ? QColor(0, 0, 0, 128) : QColor(255, 255, 255, 128)
        ));
        painter.drawLine(x, e->rect().top(), x, e->rect().bottom());
    }
}

void TimelineWidget::imageTimeChanged()
{
    update();
}

void TimelineWidget::cacheChanged()
{
    update();
}

void TimelineWidget::setCanvas(KisCanvas2 *canvas)
{
    if (m_image) {
        disconnect(m_image, 0, this, 0);
        disconnect(m_cache, 0, this, 0);
        disconnect(m_canvas->animationPlayer(), 0, this, 0);

        m_timelineView->reset();
        m_image = 0;
    }

    m_canvas = canvas;

    if (m_canvas) {
        m_image = m_canvas->image();
        m_cache = m_canvas->frameCache();

        if (m_image && m_cache) {
            connect(m_image->animationInterface(), SIGNAL(sigTimeChanged(int)), this, SLOT(imageTimeChanged()));
            connect(m_cache.data(), SIGNAL(changed()), this, SLOT(cacheChanged()));
            connect(m_canvas->animationPlayer(), SIGNAL(sigFrameChanged()), this, SLOT(imageTimeChanged()));
            connect(m_canvas->animationPlayer(), SIGNAL(sigPlaybackStopped()), this, SLOT(imageTimeChanged()));
        }
    }
}

void TimelineWidget::setModel(QAbstractItemModel *model)
{
    m_layerTree->setModel(model);
    m_timelineView->setModel(model);

    m_layerTree->hideColumn(1);
}

void TimelineWidget::mousePressEvent(QMouseEvent *e)
{
    scrub(e, true);
}

void TimelineWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (e->buttons() == Qt::LeftButton) {
        scrub(e, true);
    }
}

void TimelineWidget::mouseReleaseEvent(QMouseEvent *e)
{
    scrub(e, false);
}

void TimelineWidget::scrub(QMouseEvent *e, bool preview)
{
    if (!m_image) return;

    int time = positionToTime(e->pos().x());
    if (time >= 0) {
        scrubTo(time, preview);
    }

    e->accept();
}

void TimelineWidget::scrubTo(int time, bool preview)
{
    if (!m_image) return;

    if (preview) {
        m_canvas->animationPlayer()->displayFrame(time);
        update();
    } else {
        m_image->animationInterface()->requestTimeSwitchWithUndo(time);
        m_canvas->animationPlayer()->stop();
    }
}

int TimelineWidget::timeToPosition(int time) const
{
    return m_timelineView->timeToPosition(time) + m_timelineView->pos().x();
}

int TimelineWidget::positionToTime(int x) const
{
    return m_timelineView->positionToTime(x - m_timelineView->pos().x());
}

#include "timeline_widget.moc"
