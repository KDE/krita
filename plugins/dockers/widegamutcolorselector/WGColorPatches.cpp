/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGColorPatches.h"
#include "WGCommonColorSet.h"
#include "WGConfig.h"

#include <kis_display_color_converter.h>
#include <kis_icon_utils.h>
#include <KisUniqueColorSet.h>

#include <QMouseEvent>
#include <QPainter>
#include <QScroller>
#include <QScrollEvent>
#include <QToolButton>

namespace {
    inline QPoint transposed(QPoint point) {
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
        return point.transposed();
#else
        return QPoint(point.y(), point.x());
#endif
    }
}
WGColorPatches::WGColorPatches(WGSelectorDisplayConfigSP displayConfig, KisUniqueColorSet *history, QWidget *parent)
    : WGSelectorWidgetBase(displayConfig, parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_viewport = new QWidget(this);
    m_viewport->installEventFilter(this);
    m_viewport->setFocusProxy(this);
    m_contentWidget = new QWidget(m_viewport);
    m_contentWidget->installEventFilter(this);
    m_contentWidget->setAttribute(Qt::WA_StaticContents);
    // this prevents repainting the entire content widget when scrolling:
    m_contentWidget->setAutoFillBackground(true);
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
    m_patchCount = cfg.get(m_configSource->maxCount);

    WGConfig::Scrolling scrolling = cfg.get(m_configSource->scrolling);
    m_allowScrolling = scrolling != WGConfig::ScrollNone;
    m_scrollInline = scrolling == WGConfig::ScrollLongitudinal;

    if (m_orientation == Qt::Vertical) {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    } else {
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }

    // redo buttons
    QList<QToolButton*> buttons;
    if (m_preset == History) {
        bool showClearButton = cfg.get(WGConfig::colorHistoryShowClearButton);
        if (showClearButton) {
            buttons.append(fetchButton(m_buttonList));
        }
    }
    else if (m_preset == CommonColors) {
        if (uiMode() == PopupMode) {
            // Small workaround: override patch limit because it's basically a
            // property of WGCommonColorSet.
            m_patchCount = cfg.get(WGConfig::commonColors.maxCount);
        }
        buttons.append(fetchButton(m_buttonList));
    }
    // clear leftover buttons (if any) and set new list
    while (m_buttonList.size() > 0) {
        delete m_buttonList.takeLast();
    }
    m_buttonList = buttons;
    reconnectButtons(m_colors, m_colors);
    updateIcons();

    // recalc metrics and resize content widget
    m_patchesPerLine = -1; // ensure resizeEvent() sets new content dimensions
    QResizeEvent dummyEvent(size(), size());
    resizeEvent(&dummyEvent);

    if (QScroller::hasScroller(m_viewport)) {
        QScroller *scroller = QScroller::scroller(m_viewport);
        if (m_orientation == Qt::Horizontal) {
            scroller->setSnapPositionsX(0.0, m_patchWidth);
            scroller->setSnapPositionsY(0.0, m_patchHeight);
        } else {
            scroller->setSnapPositionsX(0.0, m_patchHeight);
            scroller->setSnapPositionsY(0.0, m_patchWidth);
        }
    }

    m_contentWidget->update();
}

void WGColorPatches::setPreset(WGColorPatches::Preset preset)
{
    if (preset == m_preset) {
        return;
    }

    m_preset = preset;

    if (uiMode() == PopupMode) {
        m_configSource = &WGConfig::popupPatches;
    }
    else {
        switch (preset) {
        case History:
            m_configSource = &WGConfig::colorHistory;
            break;
        case CommonColors:
            m_configSource = &WGConfig::commonColors;
            break;
        case None:
        default:
            m_configSource = nullptr;
        }
    }

    updateSettings();
}

QPoint WGColorPatches::popupOffset() const
{
    return patchRect(m_buttonList.size()).center();
}

void WGColorPatches::setAdditionalButtons(QList<QToolButton *> buttonList)
{
    for (int i = 0; i < buttonList.size(); i++) {
        buttonList[i]->setParent(this);
        //buttonList[i]->setAutoFillBackground(true);
        buttonList[i]->raise();
    }
    m_buttonList = buttonList;
    // recalc metrics and resize content widget
    m_patchesPerLine = -1; // ensure resizeEvent() sets new content dimensions
    QResizeEvent dummyEvent(size(), size());
    resizeEvent(&dummyEvent);
}

void WGColorPatches::setColorHistory(KisUniqueColorSet *history)
{
    if (m_colors) {
        m_colors->disconnect(m_contentWidget);
    }
    if (history) {
        connect(history, SIGNAL(sigColorAdded(int)), m_contentWidget, SLOT(update()));
        connect(history, SIGNAL(sigColorMoved(int,int)), m_contentWidget, SLOT(update()));
        connect(history, SIGNAL(sigColorRemoved(int)), m_contentWidget, SLOT(update()));
        connect(history, SIGNAL(sigReset()), m_contentWidget, SLOT(update()));
        m_scrollValue = 0;
    }
    reconnectButtons(m_colors, history);
    m_colors = history;
}

void WGColorPatches::updateIcons()
{
    if (m_buttonList.isEmpty() || m_preset == None) {
        return;
    }
    if (m_preset == History) {
        m_buttonList.first()->setIcon(KisIconUtils::loadIcon("edit-clear-16"));
    }
    else if (m_preset == CommonColors) {
        m_buttonList.first()->setIcon(KisIconUtils::loadIcon("reload-preset-16"));
    }
}

bool WGColorPatches::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Paint:
        // TODO: remove?
        // this widget doesn't paint anything on itself, instead the viewport's paint event
        // is redirected to this paintEvent() handler
        return true;
    case QEvent::ScrollPrepare:
    {
        QScrollPrepareEvent *se = static_cast<QScrollPrepareEvent *>(event);
        if (m_allowScrolling && m_maxScroll > 0) {

            se->setViewportSize(size());
            if ((m_orientation == Qt::Horizontal && m_scrollInline) ||
                (m_orientation == Qt::Vertical && !m_scrollInline)) {
                se->setContentPosRange(QRectF(0, 0, m_maxScroll, 0));
                se->setContentPos(QPointF(m_scrollValue, 0));
            }
            else {
                se->setContentPosRange(QRectF(0, 0, 0, m_maxScroll));
                se->setContentPos(QPointF(0, m_scrollValue));
            }
            se->accept();
            return true;
        }
        return false;
    }
    case QEvent::Scroll:
    {
        QScrollEvent *se = static_cast<QScrollEvent *>(event);

        if ((m_orientation == Qt::Horizontal && m_scrollInline) ||
            (m_orientation == Qt::Vertical && !m_scrollInline)) {
            m_scrollValue = qRound(se->contentPos().x() + se->overshootDistance().x());
        }
        else {
            m_scrollValue = qRound(se->contentPos().y() + se->overshootDistance().y());
        }

        // TODO: keep overshoot seperately

        m_contentWidget->move(-scrollOffset());
        return true;
    }
    default:
        return WGSelectorWidgetBase::event(event);
    }
}

bool WGColorPatches::eventFilter(QObject *watched, QEvent *e)
{
    if (watched == m_viewport) {
        // this is basically a stripped down version of QAbstractScrollArea::viewportEvent()
        switch (e->type()) {
        // redirect to base class, as this event() implementation does not care
        case QEvent::ContextMenu:
        case QEvent::Wheel:
        case QEvent::Drop:
        case QEvent::DragEnter:
        case QEvent::DragMove:
        case QEvent::DragLeave:
            return WGSelectorWidgetBase::event(e);

        // these are handled in WGColorPatches::event()
        case QEvent::ScrollPrepare:
        case QEvent::Scroll:
            return event(e);

        default: break;
        }
    }
    else if (watched == m_contentWidget) {
        switch (e->type()) {
        // redirect to base class and handle them in the specialized handlers
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::TouchBegin:
        case QEvent::TouchUpdate:
        case QEvent::TouchEnd:
        case QEvent::MouseMove:
            return WGSelectorWidgetBase::event(e);

        case QEvent::Paint:
        {
            QPaintEvent *pe = static_cast<QPaintEvent*>(e);
            this->contentPaintEvent(pe);
            return true;
        }
        default:
            break;
        }
    }
    return false;
}

void WGColorPatches::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        int index = indexAt(event->pos());
        if (index >= 0 && index != m_mouseIndex) {
            Q_EMIT sigColorChanged(m_colors->color(index));
            m_mouseIndex = index;
        }
    }
}

void WGColorPatches::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        Q_EMIT sigColorInteraction(true);
        m_mouseIndex = indexAt(event->pos());
        if (m_mouseIndex >= 0) {
            Q_EMIT sigColorChanged(m_colors->color(m_mouseIndex));
        }
    }
}

void WGColorPatches::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        Q_EMIT sigColorInteraction(false);
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
        m_contentWidget->move(-scrollOffset());
    }
    event->accept();
}

void WGColorPatches::contentPaintEvent(QPaintEvent *event)
{
    QRect updateRect = event->rect();
    //qDebug() << "WGColorPatches::conentPaintEvent region:" << event->region();
    int numColors = m_colors ? m_colors->size() : 0;
    if (numColors <= 0) {
        return;
    }

    QPainter painter(m_contentWidget);
    const KisDisplayColorConverter *converter = displayConverter();

    // this could be optimized a bit more...
    for (int i = 0; i < qMin(m_patchCount, m_colors->size()); i++) {
        QRect patch = patchRect(i);
        if (patch.intersects(updateRect)) {
            QColor qcolor = converter->toQColor(m_colors->color(i));

            painter.fillRect(patch, qcolor);
        }
    }
}

void WGColorPatches::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    int oldLineLength = m_patchesPerLine;
    updateMetrics();
    m_viewport->resize(size());
    m_scrollValue = qBound(0, m_scrollValue, m_maxScroll);
    if (oldLineLength != m_patchesPerLine) {
        QSize conentSize(m_patchesPerLine * m_patchWidth, m_totalLines * m_patchHeight);
        m_contentWidget->resize(m_orientation == Qt::Horizontal ? conentSize : conentSize.transposed());
        // notify the layout system that sizeHint() in the fixed policy dimension changed
        updateGeometry();
    }
    for (int i = 0; i < m_buttonList.size(); i++) {
        QRect buttonRect = patchRect(i);
        // mirror the rect around the center
        buttonRect.moveBottomRight(rect().bottomRight() - buttonRect.topLeft());
        m_buttonList[i]->setGeometry(buttonRect);
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
    if(!m_colors || !m_contentWidget->rect().contains(widgetPos))
        return -1;

    QPoint pos = (m_orientation == Qt::Horizontal) ? widgetPos : transposed(widgetPos);

    int col = pos.x() / m_patchWidth;
    int row = pos.y() / m_patchHeight;

    if (col > m_patchesPerLine || row > m_totalLines) {
        return -1;
    }

    int patchNr = m_scrollInline ? col * m_numLines + row : row * m_patchesPerLine + col;

    //patchNr -= m_buttonList.size();

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

        if (m_allowScrolling) {
            // with only one line, we need to subtract the buttons because we can't scroll past them
            if (m_numLines == 1) {
                m_patchesPerLine = qMax(1, m_patchesPerLine - m_buttonList.size());
                m_totalLines = (m_patchCount + m_patchesPerLine - 1) / m_patchesPerLine;
            } else {
                m_totalLines = (m_patchCount + m_buttonList.size() + m_patchesPerLine - 1) / m_patchesPerLine;
            }
        } else {
            m_totalLines = (m_patchCount + m_buttonList.size() + m_patchesPerLine - 1) / m_patchesPerLine;
            m_numLines = m_totalLines;
            m_maxScroll = 0;
        }
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

QToolButton *WGColorPatches::fetchButton(QList<QToolButton *> &recycleList)
{
    if (recycleList.size() > 0) {
        return recycleList.takeLast();
    }
    QToolButton *button = new QToolButton(this);
    button->setAutoRaise(true);
    button->show();
    return button;
}

void WGColorPatches::reconnectButtons(KisUniqueColorSet *oldSet, KisUniqueColorSet *newSet)
{
    if (m_preset == History && !m_buttonList.isEmpty()) {
        QToolButton *clearButton = m_buttonList.first();
        if (oldSet) {
            clearButton->disconnect(oldSet);
        }
        connect(clearButton, SIGNAL(clicked(bool)), newSet, SLOT(clear()));
    }
    else if (m_preset == CommonColors && !m_buttonList.isEmpty()) {
        QToolButton *reloadButton = m_buttonList.first();
        if (oldSet) {
            reloadButton->disconnect(oldSet);
        }
        WGCommonColorSet *ccSet = qobject_cast<WGCommonColorSet *>(newSet);
        if (ccSet) {
            connect(reloadButton, SIGNAL(clicked(bool)), ccSet, SLOT(slotUpdateColors()));
        }
    }
}
