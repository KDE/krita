/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_layer_contents.h"
#include "kis_animation_frame_widget.h"
#include "kis_debug.h"

#include <QMenu>
#include <QMouseEvent>
#include <QPainter>

KisLayerContentsWidget::KisLayerContentsWidget(KisFrameBox *parent)
{
    this->setFixedHeight(20);
    this->setFixedWidth(100000);

    this->m_parent = parent;
    this->setParent(parent);

    this->initialize();
}


void KisLayerContentsWidget::initialize()
{

    int length;

    if (this->m_parent->getFirstLayer()) {
        length = this->m_parent->getFirstLayer()->getContentLength();
    }
    else {
        length = 1;
    }

    KisAnimationFrameWidget* firstFrame = new KisAnimationFrameWidget(this, KisAnimationFrameWidget::FRAME, length * 10);
    firstFrame->setGeometry(0, 0, length * 10, 20);
    this->m_frames[0] = firstFrame;
}

void KisLayerContentsWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    for (int i = 0; i < 10000; i++) {
        if (i % 10 == 0) {
            painter.setPen(Qt::red);
            painter.drawRect(QRectF(10 * i, 0, 9, height() - 1));
        }
        else {
            painter.setPen(Qt::lightGray);
            painter.drawRect(QRectF(10 * i, 0, 10, height() - 1));
        }

    }
}

void KisLayerContentsWidget::mouseReleaseEvent(QMouseEvent *event)
{
    int x = event->x();
    x = x - (x % 10);
    this->m_parent->setSelectedFrame(x, this);
}

void KisLayerContentsWidget::mapFrame(int frameNumber, KisAnimationFrameWidget *frame)
{
    if (this->m_frames.contains(frameNumber)) {
        this->m_frames.remove(frameNumber);
    }

    this->m_frames[frameNumber] = frame;
}

void KisLayerContentsWidget::unmapFrame(int frameNumber)
{
    if (this->m_frames.contains(frameNumber)) {
        this->m_frames.remove(frameNumber);
    }
}

int KisLayerContentsWidget::getLastFrameIndex()
{
    QList<int> keys = m_frames.keys();
    int maximum = keys.at(0);
    for (int i = 1; i < keys.length(); i++) {
        if (maximum < keys.at(i)) {
            maximum = keys.at(i);
        }
    }
    return maximum;
}

int KisLayerContentsWidget::getPreviousFrameIndexFrom(int index)
{
    QList<int> keys = m_frames.keys();
    int previous = 0;
    for (int i = 0; i < keys.length(); i++) {
        if (keys.at(i) < index) {
            if (keys.at(i) > previous) {
                previous = keys.at(i);
            }
        }
    }
    return previous;
}

int KisLayerContentsWidget::getNextFrameIndexFrom(int index)
{
    QList<int> keys = m_frames.keys();
    int next = this->getLastFrameIndex();
    for (int i = 0; i < keys.length(); i++) {
        if (keys.at(i) > index) {
            if (keys.at(i) < next) {
                next = keys.at(i);
            }
        }
    }
    return next;
}

int KisLayerContentsWidget::getContentLength()
{
    int lastFrameLength = this->m_frames.value(this->getLastFrameIndex())->getWidth() / 10;
    return this->getLastFrameIndex() + lastFrameLength;
}

int KisLayerContentsWidget::getIndex(KisAnimationFrameWidget *frame)
{
    return this->m_frames.key(frame);
}

KisAnimationFrameWidget* KisLayerContentsWidget::getNextFrameFrom(KisAnimationFrameWidget *frame)
{
    return this->m_frames.value(this->getNextFrameIndexFrom(this->getIndex(frame)));
}

KisAnimationFrameWidget* KisLayerContentsWidget::getNextFrameFrom(int index)
{
    return this->m_frames.value(this->getNextFrameIndexFrom(index));
}

KisAnimationFrameWidget* KisLayerContentsWidget::getPreviousFrameFrom(KisAnimationFrameWidget *frame)
{
    return this->m_frames.value(this->getPreviousFrameIndexFrom(this->getIndex(frame)));
}

KisAnimationFrameWidget* KisLayerContentsWidget::getPreviousFrameFrom(int index)
{
    return this->m_frames.value(this->getNextFrameIndexFrom(index));
}

KisAnimationFrameWidget* KisLayerContentsWidget::getFrameAt(int index)
{
    QList<int> keys = m_frames.keys();
    int previous = 0;
    for (int i = 0; i < keys.length(); i++) {
        if (keys.at(i) <= index) {
            if (keys.at(i) > previous) {
                previous = keys.at(i);
            }
        }
    }
    return m_frames.value(previous);
}

KisFrameBox* KisLayerContentsWidget::getParent()
{
    return m_parent;
}

int KisLayerContentsWidget::getLayerIndex()
{
    return this->getParent()->getLayerContents().indexOf(this);
}
