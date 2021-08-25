/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGColorPatches.h"

#include <kis_display_color_converter.h>
#include <KisUniqueColorSet.h>

#include <QMouseEvent>
#include <QPainter>

namespace {
    inline QPoint transposed(QPoint point) {
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
        return point.transposed();
#else
        return QPoint(point.y(), point.x());
#endif
    }
}
WGColorPatches::WGColorPatches(KisUniqueColorSet *history, QWidget *parent)
    : WGSelectorWidgetBase(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setColorHistory(history);
}

KisUniqueColorSet *WGColorPatches::colorHistory() const
{
    return m_colors;
}

QPoint WGColorPatches::popupOffset() const
{
    return patchRect(m_buttonList.size()).center();
}

void WGColorPatches::setAdditionalButtons(QList<QWidget *> buttonList)
{
    for (int i = 0; i < buttonList.size(); i++) {
        buttonList[i]->setParent(this);
        buttonList[i]->setGeometry(patchRect(i));
    }
    m_buttonList = buttonList;
}

void WGColorPatches::setColorHistory(KisUniqueColorSet *history)
{
    if (m_colors) {
        m_colors->disconnect(this);
    }
    if (history) {
        connect(history, SIGNAL(sigColorAdded(int)), SLOT(update()));
        connect(history, SIGNAL(sigColorMoved(int,int)), SLOT(update()));
        connect(history, SIGNAL(sigColorRemoved(int)), SLOT(update()));
        connect(history, SIGNAL(sigReset()), SLOT(update()));
        m_scrollValue = 0;
    }
    m_colors = history;
}

void WGColorPatches::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        int index = indexAt(event->pos());
        if (index >= 0 && index != m_mouseIndex) {
            emit sigColorChanged(m_colors->color(index));
            m_mouseIndex = index;
        }
    }
}

void WGColorPatches::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit sigColorInteraction(true);
        m_mouseIndex = indexAt(event->pos());
        if (m_mouseIndex >= 0) {
            emit sigColorChanged(m_colors->color(m_mouseIndex));
        }
    }
}

void WGColorPatches::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit sigColorInteraction(false);
    }
}

void WGColorPatches::wheelEvent(QWheelEvent *event)
{
    if (m_allowScrolling) {
        // scroll two patches per "tick"
        int scrollAmount = 2 * m_patchWidth;
        scrollAmount = (event->angleDelta().y() * scrollAmount) / QWheelEvent::DefaultDeltasPerStep;
        m_scrollValue = qBound(0, m_scrollValue - scrollAmount, maxScroll());
        update();
    }
}

void WGColorPatches::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    int numColors = m_colors ? m_colors->size() : 0;
    if (numColors <= 0) {
        return;
    }

    QPainter painter(this);
    int scrollCount = m_scrollValue / m_patchWidth;
    if (m_allowScrolling) {
        if (m_orientation == Qt::Vertical) {
            painter.translate(0, -m_scrollValue);
        } else {
            painter.translate(-m_scrollValue, 0);
        }
    }

    const KisDisplayColorConverter *converter = displayConverter();
    for (int i = scrollCount * m_numLines; i < qMin(m_patchCount, m_colors->size()); i++) {
        QColor qcolor = converter->toQColor(m_colors->color(i));

        painter.fillRect(patchRect(i + m_buttonList.size()), qcolor);
    }
}

void WGColorPatches::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    m_scrollValue = qBound(0, m_scrollValue, maxScroll());
}

QSize WGColorPatches::sizeHint() const
{
    int lineLen = (m_patchCount + m_buttonList.size() + m_numLines - 1) / m_numLines;
    if (m_orientation == Qt::Vertical) {
        return QSize(m_numLines * m_patchHeight, lineLen * m_patchWidth);
    } else {
        return QSize(lineLen * m_patchWidth, m_numLines * m_patchHeight);
    }
}

bool WGColorPatches::colorAt(const QPoint &pos, KoColor &result) const
{
    int patchNr = indexAt(pos);

    if (patchNr >= 0) {
        result = m_colors->color(patchNr);
        return true;
    }
    return false;
}

int WGColorPatches::indexAt(const QPoint &widgetPos) const
{
    if(!m_colors || !rect().contains(widgetPos))
        return -1;

    QPoint pos = (m_orientation == Qt::Horizontal) ? widgetPos : transposed(widgetPos);

    int column = (pos.x() + m_scrollValue) / m_patchWidth;
    int row = pos.y() / m_patchHeight;

    int patchNr = row * m_numLines + column;

    patchNr -= m_buttonList.size();

    if (patchNr >= 0 && patchNr < qMin(m_patchCount, m_colors->size())) {
        return patchNr;
    }
    return -1;
}

int WGColorPatches::maxScroll() const
{
    QSize fullSize = sizeHint();
    if (m_orientation == Qt::Vertical) {
        if (height() >= fullSize.height()) {
            return 0;
        }
        return fullSize.height() - height();
    }
    else {
        if (width() >= fullSize.width()) {
            return 0;
        }
        return fullSize.width() - width();
    }
}

QRect WGColorPatches::patchRect(int gridIndex) const
{
    int row = gridIndex % m_numLines;
    int col = gridIndex / m_numLines;

    QSize patchSize(m_patchWidth, m_patchHeight);
    QPoint pos(col * m_patchWidth, row * m_patchHeight);

    return (m_orientation == Qt::Horizontal) ? QRect(pos, patchSize)
                                             : QRect(transposed(pos), patchSize.transposed());
}
