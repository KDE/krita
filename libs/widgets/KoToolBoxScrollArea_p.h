/*
 * Copyright (c) 2018 Alvin Wong <alvinhochun@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KO_TOOLBOX_SCROLL_AREA_H
#define KO_TOOLBOX_SCROLL_AREA_H

#include "KoToolBox_p.h"
#include "KoToolBoxLayout_p.h"

#include <QScrollArea>
#include <QScrollBar>
#include <QScroller>
#include <QStyleOption>
#include <QToolButton>

class KoToolBoxScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    KoToolBoxScrollArea(KoToolBox *toolBox, QWidget *parent)
        : QScrollArea(parent)
        , m_toolBox(toolBox)
        , m_orientation(Qt::Vertical)
        , m_scrollPrev(new QToolButton(this))
        , m_scrollNext(new QToolButton(this))
    {
        setFrameShape(QFrame::NoFrame);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        m_toolBox->setOrientation(m_orientation);
        setWidget(m_toolBox);

        m_scrollPrev->setAutoRepeat(true);
        m_scrollPrev->setAutoFillBackground(true);
        m_scrollPrev->setFocusPolicy(Qt::NoFocus);
        connect(m_scrollPrev, &QToolButton::clicked, this, &KoToolBoxScrollArea::doScrollPrev);
        m_scrollNext->setAutoRepeat(true);
        m_scrollNext->setAutoFillBackground(true);
        m_scrollNext->setFocusPolicy(Qt::NoFocus);
        connect(m_scrollNext, &QToolButton::clicked, this, &KoToolBoxScrollArea::doScrollNext);

        QScroller::grabGesture(viewport(), QScroller::LeftMouseButtonGesture);
        QScroller *scroller = QScroller::scroller(viewport());
        QScrollerProperties sp = scroller->scrollerProperties();

        sp.setScrollMetric(QScrollerProperties::MaximumVelocity, 0.0);
        sp.setScrollMetric(QScrollerProperties::OvershootDragResistanceFactor, 0.1);
        sp.setScrollMetric(QScrollerProperties::OvershootDragDistanceFactor, 0.1);
        sp.setScrollMetric(QScrollerProperties::OvershootScrollDistanceFactor, 0.0);
        sp.setScrollMetric(QScrollerProperties::OvershootScrollTime, 0.4);

        scroller->setScrollerProperties(sp);
    }

    void setOrientation(Qt::Orientation orientation)
    {
        if (orientation == m_orientation) {
            return;
        }
        m_orientation = orientation;
        m_toolBox->setOrientation(orientation);
        layoutItems();
    }

    Qt::Orientation orientation() const
    {
        return m_orientation;
    }

    QSize minimumSizeHint() const override
    {
        return m_toolBox->minimumSizeHint();
    }

    QSize sizeHint() const override
    {
        return m_toolBox->sizeHint();
    }

protected:
    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::LayoutRequest) {
            // LayoutRequest can be triggered by icon changes, so resize the toolbox
            layoutItems();
            // The toolbox might have changed the sizeHint and minimumSizeHint
            updateGeometry();
        }
        return QScrollArea::event(event);
    }

    void resizeEvent(QResizeEvent *event) override
    {
        layoutItems();
        QScrollArea::resizeEvent(event);
        updateScrollButtons();
    }

    void wheelEvent(QWheelEvent *event) override
    {
        if (m_orientation == Qt::Vertical) {
            QApplication::sendEvent(verticalScrollBar(), event);
        } else {
            QApplication::sendEvent(horizontalScrollBar(), event);
        }
    }

    void scrollContentsBy(int dx, int dy) override
    {
        QScrollArea::scrollContentsBy(dx, dy);
        updateScrollButtons();
    }

private Q_SLOTS:
    void doScrollPrev()
    {
        if (m_orientation == Qt::Vertical) {
            verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepSub);
        } else {
            horizontalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepSub);
        }
    }

    void doScrollNext()
    {
        if (m_orientation == Qt::Vertical) {
            verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepAdd);
        } else {
            horizontalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepAdd);
        }
    }

private:
    int scrollButtonWidth() const
    {
        QStyleOption opt;
        opt.init(this);
        return style()->pixelMetric(QStyle::PM_TabBarScrollButtonWidth, &opt, this);
    }

    void layoutItems()
    {
        const KoToolBoxLayout *l = m_toolBox->toolBoxLayout();
        QSize newSize = viewport()->size();
        if (m_orientation == Qt::Vertical) {
            newSize.setHeight(l->heightForWidth(newSize.width()));
        } else {
            newSize.setWidth(l->widthForHeight(newSize.height()));
        }
        m_toolBox->resize(newSize);

        updateScrollButtons();
    }

    void updateScrollButtons()
    {
        // We move the scroll buttons outside the widget rect instead of setting
        // the visibility, because setting the visibility triggers a relayout
        // of QAbstractScrollArea, which occasionally causes an offset bug when
        // QScroller performs the overshoot animation. (Overshoot is done by
        // moving the viewport widget, but the viewport position is reset during
        // a relayout.)
        const int scrollButtonWidth = this->scrollButtonWidth();
        if (m_orientation == Qt::Vertical) {
            m_scrollPrev->setArrowType(Qt::UpArrow);
            m_scrollPrev->setEnabled(verticalScrollBar()->value() != verticalScrollBar()->minimum());
            if (m_scrollPrev->isEnabled()) {
                m_scrollPrev->setGeometry(0, 0, width(), scrollButtonWidth);
            } else {
                m_scrollPrev->setGeometry(-width(), 0, width(), scrollButtonWidth);
            }
            m_scrollNext->setArrowType(Qt::DownArrow);
            m_scrollNext->setEnabled(verticalScrollBar()->value() != verticalScrollBar()->maximum());
            if (m_scrollNext->isEnabled()) {
                m_scrollNext->setGeometry(0, height() - scrollButtonWidth, width(), scrollButtonWidth);
            } else {
                m_scrollNext->setGeometry(-width(), height() - scrollButtonWidth, width(), scrollButtonWidth);
            }
        } else {
            m_scrollPrev->setArrowType(Qt::LeftArrow);
            m_scrollPrev->setEnabled(horizontalScrollBar()->value() != horizontalScrollBar()->minimum());
            if (m_scrollPrev->isEnabled()) {
                m_scrollPrev->setGeometry(0, 0, scrollButtonWidth, height());
            } else {
                m_scrollPrev->setGeometry(0, -height(), scrollButtonWidth, height());
            }
            m_scrollNext->setArrowType(Qt::RightArrow);
            m_scrollNext->setEnabled(horizontalScrollBar()->value() != horizontalScrollBar()->maximum());
            if (m_scrollNext->isEnabled()) {
                m_scrollNext->setGeometry(width() - scrollButtonWidth, 0, scrollButtonWidth, height());
            } else {
                m_scrollNext->setGeometry(width() - scrollButtonWidth, -height(), scrollButtonWidth, height());
            }
        }
    }

    KoToolBox *m_toolBox;
    Qt::Orientation m_orientation;

    QToolButton *m_scrollPrev;
    QToolButton *m_scrollNext;
};

#endif // KO_TOOLBOX_SCROLL_AREA_H
