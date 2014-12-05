/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version.
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

#include "kis_animation_frame_widget.h"
#include "kis_debug.h"

#include <QPainter>

KisAnimationFrameWidget::KisAnimationFrameWidget(KisLayerContentsWidget *parent, int type, int width)
{
    this->m_type = type;
    this->m_width = width;
    this->setParent(parent);
    this->m_parent = parent;
}

void KisAnimationFrameWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    if (this->m_type == KisAnimationFrameWidget::FRAME) {
        painter.setPen(Qt::black);
        painter.setBrush(Qt::white);
        painter.drawRect(0, 0, this->m_width, 20);
        painter.drawEllipse(QPointF(5, 10), 3, 3);

        if (this->m_width > 10) {
            painter.drawRect(this->m_width - 8, 3, 6, 14);
        }
    }

    if (this->m_type == KisAnimationFrameWidget::SELECTION) {
        painter.setPen(Qt::blue);
        painter.setBrush(Qt::blue);
        painter.setOpacity(0.5);
        painter.drawRect(0, 0, this->m_width - 1, 19);
    }
}

int KisAnimationFrameWidget::getWidth()
{
    return m_width;
}

void KisAnimationFrameWidget::setWidth(int width)
{
    this->m_width = width;
    this->repaint();
}

KisLayerContentsWidget* KisAnimationFrameWidget::getParent()
{
    return this->m_parent;
}

int KisAnimationFrameWidget::getType()
{
    return this->m_type;
}

void KisAnimationFrameWidget::setType(int type)
{
    this->m_type = type;
    this->repaint();
}

int KisAnimationFrameWidget::getIndex()
{
    int index = (int)this->geometry().x() / 10;
    return index;
}

QRect KisAnimationFrameWidget::convertSelectionToFrame()
{
    QRect globalGeometry;

    if (this->getType() == KisAnimationFrameWidget::SELECTION) {

        this->getParent()->mapFrame(this->geometry().x() / 10, this);

        KisAnimationFrameWidget* oldPrevFrame = this->getParent()->getPreviousFrameFrom(this);

        if (!oldPrevFrame) {
            return QRect();
        }

        oldPrevFrame->hide();

        int oldPrevFrameWidth = oldPrevFrame->getWidth();
        int previousFrameWidth = this->geometry().x() - this->getParent()->getPreviousFrameFrom(this)->geometry().x();

        KisAnimationFrameWidget* newPreviousFrame = new KisAnimationFrameWidget(this->getParent(), KisAnimationFrameWidget::FRAME, previousFrameWidth);
        newPreviousFrame->setGeometry(this->getParent()->getPreviousFrameFrom(this)->geometry().x(), 0, previousFrameWidth, 20);
        newPreviousFrame->show();

        int previousFrameIndex = this->getParent()->getPreviousFrameFrom(this)->geometry().x() / 10;

        this->getParent()->mapFrame(previousFrameIndex, newPreviousFrame);

        int newFrameWidth;

        int nextFrameIndex = this->getParent()->getNextFrameIndexFrom(this->getParent()->getIndex(this));
        newFrameWidth = nextFrameIndex * 10 - this->geometry().x();

        if (newFrameWidth == 0) {
            if (this->geometry().x() < this->getParent()->getPreviousFrameFrom(this)->geometry().x() + oldPrevFrameWidth) {
                newFrameWidth = this->getParent()->getPreviousFrameFrom(this)->geometry().x() + oldPrevFrameWidth - this->geometry().x();
            }
            else {
                newFrameWidth = 10;
            }
        }

        KisAnimationFrameWidget* newFrame = new KisAnimationFrameWidget(this->getParent(), KisAnimationFrameWidget::FRAME, newFrameWidth);
        newFrame->setGeometry(this->geometry().x(), 0, newFrameWidth, 20);
        newFrame->show();

        globalGeometry.setRect(this->geometry().x(), this->getParent()->getLayerIndex() * 20, newFrameWidth, 20);

        this->getParent()->mapFrame(this->geometry().x() / 10, newFrame);
        this->getParent()->getParent()->getSelectedFrame()->hide();
    }

    return globalGeometry;
}

void KisAnimationFrameWidget::expandWidth()
{
    this->getParent()->mapFrame(this->geometry().x() / 10, this);

    KisAnimationFrameWidget* previousFrame = this->getParent()->getPreviousFrameFrom(this);

    if (previousFrame->geometry().x() + previousFrame->getWidth() - 10 < this->geometry().x()) {
        int newWidth = this->geometry().x() - previousFrame->geometry().x() + 10;
        KisAnimationFrameWidget* newPreviousFrame = new KisAnimationFrameWidget(previousFrame->getParent(), previousFrame->getType(), newWidth);
        newPreviousFrame->setGeometry(previousFrame->x(), 0, newWidth, 20);
        newPreviousFrame->show();
        this->getParent()->mapFrame(previousFrame->x() / 10, newPreviousFrame);
    }

    this->getParent()->getParent()->getSelectedFrame()->hide();
    this->getParent()->unmapFrame(this->geometry().x() / 10);
    this->getParent()->getParent()->setSelectedFrame();
}

QRect KisAnimationFrameWidget::removeFrame()
{
    QRect deletedFrame;

    KisAnimationFrameWidget* previousFrame = this->getParent()->getFrameAt(this->geometry().x() / 10);

    if (previousFrame) {
        this->getParent()->unmapFrame(previousFrame->geometry().x() / 10);
        previousFrame->hide();

        deletedFrame.setRect(previousFrame->geometry().x(), this->getParent()->getLayerIndex() * 20, previousFrame->geometry().width(), 20);

        this->getParent()->getParent()->getSelectedFrame()->hide();
    }
    return deletedFrame;
}
