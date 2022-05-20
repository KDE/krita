/*
 * SPDX-FileCopyrightText: 2018 Alvin Wong <alvinhochun@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
#include <KisKineticScroller.h>

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
        // These are for filtering the mouse wheel events:
        m_scrollPrev->installEventFilter(this);
        m_scrollNext->installEventFilter(this);

        QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(this);
        if (!scroller) {
            QScroller::grabGesture(viewport(), QScroller::MiddleMouseButtonGesture);
            scroller = QScroller::scroller(viewport());
            QScrollerProperties sp = scroller->scrollerProperties();

            sp.setScrollMetric(QScrollerProperties::MaximumVelocity, 0.0);
            sp.setScrollMetric(QScrollerProperties::OvershootDragResistanceFactor, 0.1);
            sp.setScrollMetric(QScrollerProperties::OvershootDragDistanceFactor, 0.1);
            sp.setScrollMetric(QScrollerProperties::OvershootScrollDistanceFactor, 0.0);
            sp.setScrollMetric(QScrollerProperties::OvershootScrollTime, 0.4);

            scroller->setScrollerProperties(sp);
        }
        connect(scroller, SIGNAL(stateChanged(QScroller::State)), this, SLOT(slotScrollerStateChange(QScroller::State)));
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

public Q_SLOTS:
    void slotScrollerStateChange(QScroller::State state){ KisKineticScroller::updateCursor(this, state); }

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

    bool eventFilter(QObject *watched, QEvent *event) override
    {
        // The toolbuttons eat the wheel events, so we filter them for our use.
        if ((watched == m_scrollPrev || watched == m_scrollNext) && event->type() == QEvent::Wheel) {
            wheelEvent(static_cast<QWheelEvent *>(event));
            return true;
        }
        return QScrollArea::eventFilter(watched, event);
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
        const QScrollBar *scrollbar = m_orientation == Qt::Vertical ? verticalScrollBar() : horizontalScrollBar();
        const bool canPrev = scrollbar->value() != scrollbar->minimum();
        const bool canNext = scrollbar->value() != scrollbar->maximum();
        m_scrollPrev->setEnabled(canPrev);
        m_scrollNext->setEnabled(canNext);
        if (m_orientation == Qt::Vertical) {
            m_scrollPrev->setArrowType(Qt::UpArrow);
            m_scrollPrev->setGeometry(canPrev ? 0 : -width(), 0, width(), scrollButtonWidth);
            m_scrollNext->setArrowType(Qt::DownArrow);
            m_scrollNext->setGeometry(canNext? 0 : -width(), height() - scrollButtonWidth, width(), scrollButtonWidth);
        } else if (isLeftToRight()) {
            m_scrollPrev->setArrowType(Qt::LeftArrow);
            m_scrollPrev->setGeometry(0, canPrev ? 0 : -height(), scrollButtonWidth, height());
            m_scrollNext->setArrowType(Qt::RightArrow);
            m_scrollNext->setGeometry(width() - scrollButtonWidth, canNext ? 0 : -height(), scrollButtonWidth, height());
        } else {
            // Right-to-left is mirrored.
            m_scrollPrev->setArrowType(Qt::RightArrow);
            m_scrollPrev->setGeometry(width() - scrollButtonWidth, canPrev ? 0 : -height(), scrollButtonWidth, height());
            m_scrollNext->setArrowType(Qt::LeftArrow);
            m_scrollNext->setGeometry(0, canNext ? 0 : -height(), scrollButtonWidth, height());
        }
    }

    KoToolBox *m_toolBox;
    Qt::Orientation m_orientation;

    QToolButton *m_scrollPrev;
    QToolButton *m_scrollNext;
};

#endif // KO_TOOLBOX_SCROLL_AREA_H
