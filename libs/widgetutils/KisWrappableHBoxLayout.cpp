/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2021 Agata Cacko <cacko.azh@gmail.com>
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <QtWidgets>
#include <KisWrappableHBoxLayout.h>


KisWrappableHBoxLayout::KisWrappableHBoxLayout(QWidget *parent)
    : QLayout(parent)
{
    // Remove extra floating space.
    setContentsMargins(0, 0, 0, 0);
}

KisWrappableHBoxLayout::~KisWrappableHBoxLayout()
{
    QLayoutItem *item;
    while ((item = takeAt(0))) {
        delete item;
    }
}

void KisWrappableHBoxLayout::addItem(QLayoutItem *item)
{
    m_items.append(item);
}

QSize KisWrappableHBoxLayout::sizeHint() const
{
    const QMargins margins = contentsMargins();
    if (!geometry().isEmpty()) {
        return QSize(geometry().width(), heightForWidth(geometry().width()))
                + QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
    }
    return minimumSize();
}

QSize KisWrappableHBoxLayout::minimumSize() const
{
    const QMargins margins = contentsMargins();
    const QSize marginsSize = QSize(margins.left() + margins.right(), margins.top() + margins.bottom());

    QSize size;
    for (const QLayoutItem *item : qAsConst(m_items))
        size = size.expandedTo(item->minimumSize());

    if (!geometry().isEmpty()) {
        QSize optimal = QSize(geometry().width(), heightForWidth(geometry().width()));
        int minimumWidth = size.width(); // cannot be smaller than the biggest item
        int minimumHeight = qMax(size.height(), optimal.height());
        QSize minimum = QSize(minimumWidth, minimumHeight);
        return minimum;
    }

    size += marginsSize;
    //size = QSize(size.width(), size.height()*5);
    return size;
}

int KisWrappableHBoxLayout::count() const
{
    return m_items.size();
}



QLayoutItem *KisWrappableHBoxLayout::itemAt(int idx) const
{
    return m_items.value(idx);
}

QLayoutItem *KisWrappableHBoxLayout::takeAt(int idx)
{
    return idx >= 0 && idx < m_items.size() ? m_items.takeAt(idx) : 0;
}

void KisWrappableHBoxLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

bool KisWrappableHBoxLayout::hasHeightForWidth() const
{
    return true;
}

int KisWrappableHBoxLayout::heightForWidth(int width) const
{
    int height = doLayout(QRect(0, 0, width, 0), true);
    return height;
}

int KisWrappableHBoxLayout::doLayout(const QRect &rect, bool testOnly) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    for (QLayoutItem *item : qAsConst(m_items)) {
        int nextX = x + item->sizeHint().width() + spacing();
        if (nextX - spacing() > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x();
            y = y + lineHeight + spacing();
            nextX = x + item->sizeHint().width() + spacing();
            lineHeight = 0;
        }

        if (!testOnly) {
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));
        }

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    return y + lineHeight - rect.y() + bottom;
}

