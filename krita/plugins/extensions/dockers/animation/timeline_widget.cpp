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

#include "kis_image.h"
#include "kis_image_animation_interface.h"

TimelineWidget::TimelineWidget(QWidget *parent)
    : QWidget(parent)
    , m_timelineView(new TimelineView(this))
    , m_image(0)
{
    m_layout = new QVBoxLayout(this);

    m_layout->insertSpacing(0, 16);
    m_layout->addWidget(m_timelineView);

    m_timelineView->setFrameShape(QFrame::NoFrame);
    m_timelineView->viewport()->setAutoFillBackground(false);
}

void TimelineWidget::paintEvent(QPaintEvent *e)
{
    if (!m_image) return;

    QColor background = palette().background().color();
    bool dark = background.value() > 100;

    QPainter painter(this);

    int t0 = m_timelineView->positionToTime(e->rect().left());
    int t1 = m_timelineView->positionToTime(e->rect().right());

    painter.setPen(QPen(QColor(128, 128, 128, 128)));
    for (int t=t0; t<=t1; t++) {
        int x = m_timelineView->timeToPosition(t);
        if (!m_timelineView->isWithingView(x)) continue;

        if (t % 10 == 0) {
            painter.drawLine(x, e->rect().top(), x, e->rect().bottom());
        } else {
            painter.drawLine(x, 4, x, 12);
        }
    }

    int x = m_timelineView->timeToPosition(m_image->animationInterface()->currentTime()) + 4;
    if (m_timelineView->isWithingView(x)) {
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

void TimelineWidget::setImage(KisImageWSP image)
{
    if (m_image) {
        disconnect(m_image, 0, this, 0);
        m_timelineView->reset();
    }

    m_image = image;

    if (m_image) {
        connect(m_image->animationInterface(), SIGNAL(sigTimeChanged(int)), this, SLOT(imageTimeChanged()));
    }
}

void TimelineWidget::setModel(QAbstractItemModel *model)
{
    m_timelineView->setModel(model);
}

void TimelineWidget::mousePressEvent(QMouseEvent *e)
{
    scrub(e);
}

void TimelineWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (e->buttons() == Qt::LeftButton) {
        scrub(e);
    }
}

void TimelineWidget::scrub(QMouseEvent *e)
{
    if (!m_image) return;

    int time = m_timelineView->positionToTime(e->pos().x());
    if (time >= 0) {
        m_image->animationInterface()->switchCurrentTimeAsync(time);
    }

    e->accept();
}

#include "timeline_widget.moc"
