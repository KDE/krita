/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGColorPatches.h"
#include "WGConfig.h"

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

void WGColorPatches::updateSettings()
{
    if (!m_configSource) {
        return;
    }

    WGConfig::Accessor cfg;

    QSize patchSize = cfg.get(m_configSource->patchSize);
    m_patchWidth = patchSize.width();
    m_patchHeight = patchSize.height();
    m_orientation = cfg.get(m_configSource->orientation);
    m_numLines = cfg.get(m_configSource->rows);

    WGConfig::Scrolling scrolling = cfg.get(m_configSource->scrolling);
    m_allowScrolling = scrolling != WGConfig::ScrollNone;
    m_scrollInline = scrolling == WGConfig::ScrollLongitudinal;

    updateMetrics();

    if (m_orientation == Qt::Vertical) {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    } else {
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }

    updateGeometry();
    update();
}

void WGColorPatches::setConfigSource(const WGConfig::ColorPatches *source)
{
    m_configSource = source;
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
    if (!m_allowScrolling) {
        return;
    }

    int oldScroll = m_scrollValue;

    if (m_scrollInline) {
        // scroll two patches per "tick"
        int scrollAmount = 2 * m_patchWidth;
        scrollAmount = (event->angleDelta().y() * scrollAmount) / QWheelEvent::DefaultDeltasPerStep;
        m_scrollValue = qBound(0, m_scrollValue - scrollAmount, m_maxScroll);
    }
    else {
        // scroll one row per "tick"
        int scrollAmount = (event->angleDelta().y() * m_patchHeight)  / QWheelEvent::DefaultDeltasPerStep;
        m_scrollValue = qBound(0, m_scrollValue - scrollAmount, m_maxScroll);
    }

    if (oldScroll != m_scrollValue) {
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
    painter.translate(-scrollOffset());

    int scrollCount = 0; // m_scrollValue / m_patchWidth; TODO: determine proper ranges

    const KisDisplayColorConverter *converter = displayConverter();
    for (int i = scrollCount * m_numLines; i < qMin(m_patchCount, m_colors->size()); i++) {
        QColor qcolor = converter->toQColor(m_colors->color(i));

        painter.fillRect(patchRect(i + m_buttonList.size()), qcolor);
    }
}

void WGColorPatches::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    int oldLineCount = m_numLines;
    updateMetrics();
    m_scrollValue = qBound(0, m_scrollValue, m_maxScroll);
    if (oldLineCount != m_numLines) {
        // notify the layout system that sizeHint() in the fixed policy dimension changed
        updateGeometry();
    }
}

QSize WGColorPatches::sizeHint() const
{
    if (m_orientation == Qt::Vertical) {
        return QSize(m_numLines * m_patchHeight, m_patchesPerLine * m_patchWidth);
    } else {
        return QSize(m_patchesPerLine * m_patchWidth, m_numLines * m_patchHeight);
    }
}

int WGColorPatches::indexAt(const QPoint &widgetPos) const
{
    if(!m_colors || !rect().contains(widgetPos))
        return -1;

    QPoint pos = (m_orientation == Qt::Horizontal) ? widgetPos : transposed(widgetPos);
    int row, col, patchNr;
    if (m_scrollInline) {
        col = (pos.x() + m_scrollValue) / m_patchWidth;
        row = pos.y() / m_patchHeight;
        patchNr = row * m_numLines + col;
    }
    else {
        col = pos.x() / m_patchWidth;
        row = (pos.y() + m_scrollValue) / m_patchHeight;
        patchNr = row * m_patchesPerLine + col;
    }

    if (col > m_patchesPerLine || row > m_totalLines) {
        return -1;
    }

    patchNr -= m_buttonList.size();

    if (patchNr >= 0 && patchNr < qMin(m_patchCount, m_colors->size())) {
        return patchNr;
    }
    return -1;
}

QRect WGColorPatches::patchRect(int gridIndex) const
{
    int row, col;
    if (m_scrollInline) {
        row = gridIndex % m_numLines;
        col = gridIndex / m_numLines;
    }
    else {
        row = gridIndex / m_patchesPerLine;
        col = gridIndex % m_patchesPerLine;
    }

    QSize patchSize(m_patchWidth, m_patchHeight);
    QPoint pos(col * m_patchWidth, row * m_patchHeight);

    return (m_orientation == Qt::Horizontal) ? QRect(pos, patchSize)
                                             : QRect(transposed(pos), patchSize.transposed());
}

QPoint WGColorPatches::scrollOffset() const
{
    if (!m_allowScrolling) {
        return QPoint(0, 0);
    }
    QPoint offset(0, 0);
    if (m_orientation == Qt::Horizontal) {
        if (m_scrollInline) {
            offset.rx() += m_scrollValue;
        } else {
            offset.ry() += m_scrollValue;
        }
    } else {
        if (m_scrollInline) {
            offset.ry() += m_scrollValue;
        } else {
            offset.rx() += m_scrollValue;
        }
    }
    return offset;
}

void WGColorPatches::updateMetrics()
{
    if (m_scrollInline) {
        m_patchesPerLine = (m_patchCount + m_buttonList.size() + m_numLines - 1) / m_numLines;
        m_totalLines = m_numLines;
    }
    else {
        // in this mode, the line length and count depends on widget size
        int availableLength = (m_orientation == Qt::Horizontal) ? width() : height();
        m_patchesPerLine = qMax(1, availableLength / m_patchWidth);
        m_totalLines = (m_patchCount + m_buttonList.size() + m_patchesPerLine - 1) / m_patchesPerLine;
        if (!m_allowScrolling) {
            m_numLines = m_totalLines;
            m_maxScroll = 0;
        }
        qDebug() << "patchesPerLine/numLines:" << m_patchesPerLine << m_numLines;
    }
    // scroll limit
    if (m_allowScrolling) {
        if (m_scrollInline) {
            int available = (m_orientation == Qt::Horizontal) ? width() : height();
            int required = m_patchesPerLine * m_patchWidth;
            m_maxScroll = qMax(0, required - available);
        }
        else {
            int available = (m_orientation == Qt::Horizontal) ? height() : width();
            int required = m_totalLines * m_patchHeight;
            m_maxScroll = qMax(0, required - available);
        }
    }
}
