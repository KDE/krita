/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_color_label_selector_widget.h"

#include "kis_debug.h"
#include "kis_global.h"

#include <QAction>
#include <QIcon>
#include <QMenu>

#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QResizeEvent>

#include <QStyleOptionToolButton>
#include <QStylePainter>

#include "kis_node_view_color_scheme.h"

struct KisColorLabelSelectorWidget::Private
{
    Private(KisColorLabelSelectorWidget *_q)
        : q(_q),
          xMenuOffset(0),
          yCenteringOffset(0),
          realItemSize(0),
          realItemSpacing(0),
          hoveringItem(-1),
          selectedItem(0)
    {
    }

    KisColorLabelSelectorWidget *q;
    QVector<QColor> colors;


    const int minHeight = 12 + 4;
    const int minSpacing = 1;
    const int maxSpacing = 3;
    const int border = 2;

    int xMenuOffset;
    int yCenteringOffset;
    int realItemSize;
    int realItemSpacing;

    int hoveringItem;
    int selectedItem;

    QRect itemRect(int index) const;
    int indexFromPos(const QPoint &pos);
    void updateItem(int index);

    int widthForHeight(int height, int spacing) const;
    int heightForWidth(int width, int spacing) const;
    void updateItemSizes(const QSize &widgetSize);
};

KisColorLabelSelectorWidget::KisColorLabelSelectorWidget(QWidget *parent)
    : QWidget(parent),
      m_d(new Private(this))
{
    KisNodeViewColorScheme scm;
    m_d->colors = scm.allColorLabels();
    setMouseTracking(true);
}

KisColorLabelSelectorWidget::~KisColorLabelSelectorWidget()
{
}

int KisColorLabelSelectorWidget::currentIndex() const
{
    return m_d->selectedItem;
}

void KisColorLabelSelectorWidget::setCurrentIndex(int index)
{
    if (index == m_d->selectedItem) return;

    const int oldItem = m_d->selectedItem;
    m_d->selectedItem = index;
    m_d->updateItem(oldItem);
    m_d->updateItem(m_d->selectedItem);
    m_d->hoveringItem = index;

    emit currentIndexChanged(m_d->selectedItem);
}

QSize KisColorLabelSelectorWidget::minimumSizeHint() const
{
    return QSize(m_d->widthForHeight(m_d->minHeight, m_d->minSpacing), m_d->minHeight);
}

QSize KisColorLabelSelectorWidget::sizeHint() const
{
    const int preferredHeight = 22 + 2 * m_d->border;
    return QSize(m_d->widthForHeight(preferredHeight, m_d->maxSpacing), preferredHeight);
}

void KisColorLabelSelectorWidget::resizeEvent(QResizeEvent *e)
{
    m_d->xMenuOffset = 0;

    bool hasWideItems = false;
    QMenu *menu = qobject_cast<QMenu*>(parent());
    if (menu) {
        Q_FOREACH(QAction *action, menu->actions()) {
            if (action->isCheckable() ||
                !action->icon().isNull()) {

                hasWideItems = true;
                break;
            }
        }
    }

    if (hasWideItems) {
        QStyleOption opt;
        opt.init(this);
        // some copy-pasted code from QFusionStyle style
        const int hmargin = style()->pixelMetric(QStyle::PM_MenuHMargin, &opt, this);
        const int icone = style()->pixelMetric(QStyle::PM_SmallIconSize, &opt, this);
        m_d->xMenuOffset = hmargin + icone + 6;
    }

    m_d->updateItemSizes(e->size());
    QWidget::resizeEvent(e);
}

int KisColorLabelSelectorWidget::Private::widthForHeight(int height, int spacing) const
{
    return height * colors.size() + spacing * (colors.size() - 1) + 2 * border + xMenuOffset;
}

int KisColorLabelSelectorWidget::Private::heightForWidth(int width, int spacing) const
{
    const int numItems = colors.size();
    return qRound(qreal(width - spacing * (numItems - 1) - 2 * border - xMenuOffset) / numItems);
}

void KisColorLabelSelectorWidget::Private::updateItemSizes(const QSize &widgetSize)
{
    const int height = qBound(minHeight,
                              heightForWidth(widgetSize.width(), minSpacing),
                              widgetSize.height());

    const int size = height - 2 * border;
    const int numItems = colors.size();

    const int rest = widgetSize.width() - size * numItems - 2 * border - xMenuOffset;
    const int spacing = qBound(minSpacing,
                               rest / (numItems - 1),
                               maxSpacing);

    realItemSize = size;
    realItemSpacing = spacing;
    yCenteringOffset = qMax(0, (widgetSize.height() - height) / 2);
}

QRect KisColorLabelSelectorWidget::Private::itemRect(int index) const
{
    const int x = xMenuOffset + border + index * realItemSize + index * realItemSpacing;
    const int y = border + yCenteringOffset;

    return QRect(x, y, realItemSize, realItemSize);
}

int KisColorLabelSelectorWidget::Private::indexFromPos(const QPoint &pos)
{
    const int x = pos.x() - border - xMenuOffset;
    const int y = pos.y() - border - yCenteringOffset;
    if (y < 0 || y >= realItemSize) return -1;
    int idx = (x + realItemSpacing) / (realItemSize + realItemSpacing);

    if (idx < 0 || idx >= colors.size()) {
        idx = -1;
    }

    return idx;
}

void KisColorLabelSelectorWidget::Private::updateItem(int index)
{
    if (index >= 0 && index < colors.size()) {
        q->update(kisGrowRect(itemRect(index), border));
    }
}

enum State {
    NORMAL = 0,
    HOVER,
    CHECKED,
    DISABLED
};

void drawToolButton(QWidget *widget, const QRect &rc, State state, const QColor &color, int border)
{
    QStylePainter p(widget);
    QStyleOption opt;
    opt.initFrom(widget);
    opt.rect = kisGrowRect(rc, border);

    switch (state) {
    case DISABLED:
    case NORMAL:
        opt.state &= ~QStyle::State_Raised;
        break;
    case HOVER:
        opt.state |= QStyle::State_Raised;
        break;
    case CHECKED:
        opt.state |= QStyle::State_On;
        break;
    };


    if (opt.state & (QStyle::State_Sunken | QStyle::State_On | QStyle::State_Raised)) {

        const QRect borderRect = kisGrowRect(rc, 1);
        p.setPen(QPen(opt.palette.text().color(), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.drawRect(borderRect);
    }

    const int offset = qMax(1, rc.height() / 10);
    const QRect colorBlobRect = kisGrowRect(rc, -offset);
    if (color.alpha() > 0) {
        QColor fillColor = color;

        if (state == DISABLED) {
            fillColor.setHsl(0, 0, color.lightness());
        }

        p.fillRect(colorBlobRect, fillColor);
    } else {

        // draw an X for no color for the first item
        QRect crossRect = kisGrowRect(colorBlobRect, -offset);

        QColor shade = opt.palette.text().color();
        p.setPen(QPen(shade, 2));
        p.drawLine(crossRect.topLeft(), crossRect.bottomRight());
        p.drawLine(crossRect.bottomLeft(), crossRect.topRight());
    }
}

void KisColorLabelSelectorWidget::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
    if (isEnabled()) {
        for (int i = 0; i < m_d->colors.size(); i++) {
            if (i == m_d->selectedItem || i == m_d->hoveringItem) {
                continue;
            }
            drawToolButton(this, m_d->itemRect(i), NORMAL, m_d->colors[i], m_d->border);
        }

        if (m_d->selectedItem >= 0) {
            drawToolButton(this, m_d->itemRect(m_d->selectedItem), CHECKED, m_d->colors[m_d->selectedItem], m_d->border);
        }

        if (m_d->hoveringItem >= 0 && m_d->hoveringItem != m_d->selectedItem) {
            drawToolButton(this, m_d->itemRect(m_d->hoveringItem), HOVER, m_d->colors[m_d->hoveringItem], m_d->border);
        }
    } else {
        for (int i = 0; i < m_d->colors.size(); i++) {
            drawToolButton(this, m_d->itemRect(i), DISABLED, m_d->colors[i], m_d->border);
        }
    }
}

void KisColorLabelSelectorWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Right || e->key() == Qt::Key_Up) {
        int newItem = (m_d->selectedItem + 1) % m_d->colors.size();
        setCurrentIndex(newItem);
    } else if (e->key() == Qt::Key_Left || e->key() == Qt::Key_Down) {
        int newItem = m_d->selectedItem < 0 ? m_d->colors.size() - 1 :
            (m_d->selectedItem + m_d->colors.size() - 1) % m_d->colors.size();
        setCurrentIndex(newItem);
    }

    QWidget::keyPressEvent(e);
}

void KisColorLabelSelectorWidget::mousePressEvent(QMouseEvent *e)
{
    const int newItem = m_d->indexFromPos(e->pos());
    if (newItem >= 0 && e->buttons() & Qt::LeftButton) {
        setCurrentIndex(newItem);
    }
    QWidget::mousePressEvent(e);
}

void KisColorLabelSelectorWidget::mouseReleaseEvent(QMouseEvent *e)
{
    QWidget::mouseReleaseEvent(e);
}

void KisColorLabelSelectorWidget::mouseMoveEvent(QMouseEvent *e)
{
    const int newItem = m_d->indexFromPos(e->pos());
    if (newItem >= 0 && e->buttons() & Qt::LeftButton) {
        setCurrentIndex(newItem);
    }

    const int oldItem = m_d->hoveringItem;
    m_d->hoveringItem = m_d->indexFromPos(e->pos());
    m_d->updateItem(oldItem);
    m_d->updateItem(m_d->hoveringItem);

    update();
    QWidget::mouseMoveEvent(e);
}

void KisColorLabelSelectorWidget::leaveEvent(QEvent *e)
{
    const int oldItem = m_d->hoveringItem;
    m_d->hoveringItem = -1;
    m_d->updateItem(oldItem);
    QWidget::leaveEvent(e);
}
