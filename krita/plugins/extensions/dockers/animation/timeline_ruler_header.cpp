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

#include "timeline_ruler_header.h"

#include <limits>
#include <klocale.h>

#include <QMenu>
#include <QAction>
#include <QPainter>
#include <QPaintEvent>

#include "timeline_frames_model.h"

#include "kis_debug.h"

struct TimelineRulerHeader::Private
{
    Private() : fps(12), lastPressSectionIndex(-1) {}

    int fps;

    QMenu *columnEditingMenu;
    QAction *insertLeftAction;
    QAction *insertRightAction;
    QAction *removeAction;
    QAction *clearAction;

    TimelineFramesModel *model;
    int lastPressSectionIndex;

    int calcSpanWidth(const int sectionWidth);
    QVector<QPoint> prepareFramesSlab(int startCol, int endCol);
};

TimelineRulerHeader::TimelineRulerHeader(QWidget *parent)
    : QHeaderView(Qt::Horizontal, parent),
      m_d(new Private)
{
    m_d->columnEditingMenu = new QMenu(this);
    m_d->insertLeftAction = m_d->columnEditingMenu->addAction("Insert 1 Left", this, SLOT(slotInsertColumnLeft()));
    m_d->insertRightAction = m_d->columnEditingMenu->addAction("Insert 1 Right", this, SLOT(slotInsertColumnRight()));
    m_d->clearAction = m_d->columnEditingMenu->addAction("Clear Columns", this, SLOT(slotClearColumns()));
    m_d->removeAction = m_d->columnEditingMenu->addAction("Remove Columns", this, SLOT(slotRemoveColumns()));

}

TimelineRulerHeader::~TimelineRulerHeader()
{
}

void TimelineRulerHeader::paintEvent(QPaintEvent *e)
{
    QHeaderView::paintEvent(e);

    // Copied from Qt 4.8...

    if (count() == 0)
        return;

    QPainter painter(viewport());
    const QPoint offset = dirtyRegionOffset();
    QRect translatedEventRect = e->rect();
    translatedEventRect.translate(offset);

    int start = -1;
    int end = -1;
    if (orientation() == Qt::Horizontal) {
        start = visualIndexAt(translatedEventRect.left());
        end = visualIndexAt(translatedEventRect.right());
    } else {
        start = visualIndexAt(translatedEventRect.top());
        end = visualIndexAt(translatedEventRect.bottom());
    }

    const bool reverseImpl = orientation() == Qt::Horizontal && isRightToLeft();

    if (reverseImpl) {
        start = (start == -1 ? count() - 1 : start);
        end = (end == -1 ? 0 : end);
    } else {
        start = (start == -1 ? 0 : start);
        end = (end == -1 ? count() - 1 : end);
    }

    int tmp = start;
    start = qMin(start, end);
    end = qMax(tmp, end);

    ///////////////////////////////////////////////////
    /// Krita specific code. We should update in spans!

    const int spanStart = start - start % m_d->fps;
    const int spanEnd = end - end % m_d->fps + m_d->fps - 1;

    start = spanStart;
    end = qMin(count() - 1, spanEnd);

    /// End of Krita specific code
    ///////////////////////////////////////////////////

    QRect currentSectionRect;
    int logical;
    const int width = viewport()->width();
    const int height = viewport()->height();
    for (int i = start; i <= end; ++i) {
        // DK: cannot copy-paste easily...
        // if (d->isVisualIndexHidden(i))
        //     continue;
        painter.save();
        logical = logicalIndex(i);
        if (orientation() == Qt::Horizontal) {
            currentSectionRect.setRect(sectionViewportPosition(logical), 0, sectionSize(logical), height);
        } else {
            currentSectionRect.setRect(0, sectionViewportPosition(logical), width, sectionSize(logical));
        }
        currentSectionRect.translate(offset);

        QVariant variant = model()->headerData(logical, orientation(),
                                                Qt::FontRole);
        if (variant.isValid() && variant.canConvert<QFont>()) {
            QFont sectionFont = qvariant_cast<QFont>(variant);
            painter.setFont(sectionFont);
        }
        paintSection1(&painter, currentSectionRect, logical);
        painter.restore();
    }
}

void TimelineRulerHeader::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    // Base paint event should paint nothing in the sections area

    Q_UNUSED(painter);
    Q_UNUSED(rect);
    Q_UNUSED(logicalIndex);
}

void TimelineRulerHeader::paintSpan(QPainter *painter, int userFrameId,
                                    const QRect &spanRect,
                                    bool isIntegralLine,
                                    bool isPrevIntegralLine,
                                    QStyle *style,
                                    const QPalette &palette,
                                    const QPen &gridPen) const
{
    painter->fillRect(spanRect, palette.brush(QPalette::Button));

    int safeRight = spanRect.right();

    QPen oldPen = painter->pen();
    painter->setPen(gridPen);

    int adjustedTop = spanRect.top() + (!isIntegralLine ? spanRect.height() / 2 : 0);
    painter->drawLine(safeRight, adjustedTop, safeRight, spanRect.bottom());

    if (isPrevIntegralLine) {
        painter->drawLine(spanRect.left() + 1, spanRect.top(), spanRect.left() + 1, spanRect.bottom());
    }

    painter->setPen(oldPen);

    QString frameIdText = QString::number(userFrameId);
    QRect textRect(spanRect.topLeft() + QPoint(2, 0), QSize(spanRect.width() - 2, spanRect.height()));

    QStyleOptionHeader opt;
    initStyleOption(&opt);

    QStyle::State state = QStyle::State_None;
    if (isEnabled())
        state |= QStyle::State_Enabled;
    if (window()->isActiveWindow())
        state |= QStyle::State_Active;
    opt.state |= state;
    opt.selectedPosition = QStyleOptionHeader::NotAdjacent;

    opt.textAlignment = Qt::AlignLeft | Qt::AlignTop;
    opt.rect = textRect;
    opt.text = frameIdText;
    style->drawControl(QStyle::CE_HeaderLabel, &opt, painter, this);
}

int TimelineRulerHeader::Private::calcSpanWidth(const int sectionWidth) {
    const int minWidth = 36;

    int spanWidth = this->fps;

    while (spanWidth * sectionWidth < minWidth) {
        spanWidth *= 2;
    }

    bool splitHappened = false;

    do {
        splitHappened = false;

        if (!(spanWidth & 0x1) &&
            spanWidth * sectionWidth / 2 > minWidth) {

            spanWidth /= 2;
            splitHappened = true;

        } else if (!(spanWidth % 3) &&
                   spanWidth * sectionWidth / 3 > minWidth) {

            spanWidth /= 3;
            splitHappened = true;

        } else if (!(spanWidth % 5) &&
                   spanWidth * sectionWidth / 5 > minWidth) {

            spanWidth /= 5;
            splitHappened = true;
        }

    } while (splitHappened);


    if (sectionWidth > minWidth) {
        spanWidth = 1;
    }

    return spanWidth;
}

void TimelineRulerHeader::paintSection1(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    if (!rect.isValid())
        return;

    QFontMetrics metrics(this->font());
    const int textHeight = metrics.height();

    QPoint p1 = rect.topLeft() + QPoint(0, textHeight);
    QPoint p2 = rect.topRight() + QPoint(0, textHeight);

    QRect frameRect = QRect(p1, QSize(rect.width(), rect.height() - textHeight));

    const int width = rect.width();

    int spanWidth = m_d->calcSpanWidth(width);

    const int internalIndex = logicalIndex % spanWidth;
    const int userFrameId = logicalIndex;

    const int spanEnd = qMin(count(), logicalIndex + spanWidth);
    QRect spanRect(rect.topLeft(), QSize(width * (spanEnd - logicalIndex), textHeight));

    QStyleOptionViewItemV4 option = viewOptions();
    const int gridHint = style()->styleHint(QStyle::SH_Table_GridLineColor, &option, this);
    const QColor gridColor = static_cast<QRgb>(gridHint);
    const QPen gridPen = QPen(gridColor);

    if (!internalIndex) {
        bool isIntegralLine = (logicalIndex + spanWidth) % m_d->fps == 0;
        bool isPrevIntegralLine = logicalIndex % m_d->fps == 0;
        paintSpan(painter, userFrameId, spanRect, isIntegralLine, isPrevIntegralLine, style(), palette(), gridPen);
    }

    {
        QBrush fillColor = palette().brush(QPalette::Button);

        QVariant activeValue = model()->headerData(logicalIndex, orientation(),
                                                   TimelineFramesModel::ActiveFrameRole);

        QVariant cachedValue = model()->headerData(logicalIndex, orientation(),
                                                   TimelineFramesModel::FrameCachedRole);

        if (activeValue.isValid() && activeValue.toBool()) {
            QColor baseColor = QColor(200, 220, 150);
            fillColor = baseColor.darker(130);
        } else if (cachedValue.isValid() && cachedValue.toBool()) {
            fillColor = fillColor.color().darker(115);
        }

        painter->fillRect(frameRect, fillColor);

        QVector<QLine> lines;
        lines << QLine(p1, p2);
        lines << QLine(frameRect.topRight(), frameRect.bottomRight());
        lines << QLine(frameRect.bottomLeft(), frameRect.bottomRight());

        QPen oldPen = painter->pen();
        painter->setPen(gridPen);
        painter->drawLines(lines);
        painter->setPen(oldPen);
    }
}

void TimelineRulerHeader::changeEvent(QEvent *event)
{
    Q_UNUSED(event);

    updateMinimumSize();
}

void TimelineRulerHeader::setFramePerSecond(int fps)
{
    m_d->fps = fps;
    update();
}

void TimelineRulerHeader::updateMinimumSize()
{
    QFontMetrics metrics(this->font());
    const int textHeight = metrics.height();

    setMinimumSize(0, 1.5 * textHeight);
}

void TimelineRulerHeader::setModel(QAbstractItemModel *model)
{
    TimelineFramesModel *framesModel = qobject_cast<TimelineFramesModel*>(model);
    m_d->model = framesModel;

    QHeaderView::setModel(model);
}

int getColumnCount(const QModelIndexList &indexes, int *leftmostCol, int *rightmostCol)
{
    QVector<int> columns;
    int leftmost = std::numeric_limits<int>::max();
    int rightmost = std::numeric_limits<int>::min();

    foreach (const QModelIndex &index, indexes) {
        leftmost = qMin(leftmost, index.column());
        rightmost = qMax(rightmost, index.column());
        if (!columns.contains(index.column())) {
            columns.append(index.column());
        }
    }

    if (leftmostCol) *leftmostCol = leftmost;
    if (rightmostCol) *rightmostCol = rightmost;

    return columns.size();
}

void TimelineRulerHeader::mousePressEvent(QMouseEvent *e)
{
    int logical = logicalIndexAt(e->pos());
    if (logical != -1) {
        QModelIndexList selectedIndexes = selectionModel()->selectedIndexes();
        int numSelectedColumns = getColumnCount(selectedIndexes, 0, 0);

        if (e->button() == Qt::RightButton) {
            if (numSelectedColumns <= 1) {
                model()->setHeaderData(logical, orientation(), true, TimelineFramesModel::ActiveFrameRole);
            }

            m_d->insertLeftAction->setText(i18n("Insert %1 left", numSelectedColumns));
            m_d->insertRightAction->setText(i18n("Insert %1 right", numSelectedColumns));
            m_d->clearAction->setText(i18n("Clear %1 columns", numSelectedColumns));
            m_d->removeAction->setText(i18n("Remove %1 columns", numSelectedColumns));

            m_d->columnEditingMenu->exec(e->globalPos());
            return;

        } else if (e->button() == Qt::LeftButton) {
            m_d->lastPressSectionIndex = logical;
            model()->setHeaderData(logical, orientation(), true, TimelineFramesModel::ActiveFrameRole);
        }
    }

    QHeaderView::mousePressEvent(e);
}

void TimelineRulerHeader::mouseMoveEvent(QMouseEvent *e)
{
    int logical = logicalIndexAt(e->pos());
    if (logical != -1) {
        if (e->buttons() & Qt::LeftButton) {
            m_d->model->setScrubState(true);
            model()->setHeaderData(logical, orientation(), true, TimelineFramesModel::ActiveFrameRole);

            if (m_d->lastPressSectionIndex >= 0 &&
                logical != m_d->lastPressSectionIndex &&
                e->modifiers() & Qt::ShiftModifier) {

                const int minCol = qMin(m_d->lastPressSectionIndex, logical);
                const int maxCol = qMax(m_d->lastPressSectionIndex, logical);

                QItemSelection sel(m_d->model->index(0, minCol), m_d->model->index(0, maxCol));
                selectionModel()->select(sel,
                                         QItemSelectionModel::Columns |
                                         QItemSelectionModel::SelectCurrent);
            }

        }
    }

    QHeaderView::mouseMoveEvent(e);
}

void TimelineRulerHeader::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_d->model->setScrubState(false);
    }
    QHeaderView::mouseReleaseEvent(e);
}

QVector<QPoint> TimelineRulerHeader::Private::prepareFramesSlab(int startCol, int endCol)
{
    QVector<QPoint> frames;

    const int numRows = model->rowCount();

    for (int i = 0; i < numRows; i++) {
        for (int j = startCol; j <= endCol; j++) {
            const bool exists = model->data(model->index(i, j), TimelineFramesModel::FrameExistsRole).toBool();
            if (exists) {
                frames << QPoint(j, i);
            }
        }
    }

    return frames;
}

void TimelineRulerHeader::slotInsertColumnLeft()
{
    QModelIndexList selectedIndexes = selectionModel()->selectedIndexes();
    int leftmostCol = 0, rightmostCol = 0;
    int numColumns = getColumnCount(selectedIndexes, &leftmostCol, &rightmostCol);

    QVector<QPoint> movingFrames = m_d->prepareFramesSlab(leftmostCol, m_d->model->columnCount() - 1);
    m_d->model->offsetFrames(movingFrames, QPoint(numColumns, 0), false);
}

void TimelineRulerHeader::slotInsertColumnRight()
{
    QModelIndexList selectedIndexes = selectionModel()->selectedIndexes();
    int leftmostCol = 0, rightmostCol = 0;
    int numColumns = getColumnCount(selectedIndexes, &leftmostCol, &rightmostCol);

    QVector<QPoint> movingFrames = m_d->prepareFramesSlab(rightmostCol + 1, m_d->model->columnCount() - 1);
    m_d->model->offsetFrames(movingFrames, QPoint(numColumns, 0), false);
}

void TimelineRulerHeader::slotClearColumns(bool removeColumns)
{
    QModelIndexList selectedIndexes = selectionModel()->selectedIndexes();
    int leftmostCol = 0, rightmostCol = 0;
    int numColumns = getColumnCount(selectedIndexes, &leftmostCol, &rightmostCol);

    QVector<QPoint> movingFrames = m_d->prepareFramesSlab(leftmostCol, rightmostCol);

    QModelIndexList framesToRemove;
    foreach (const QPoint &pt, movingFrames) {
        QModelIndex index = m_d->model->index(pt.y(), pt.x());
        if (index.isValid()) {
            framesToRemove << index;
        }
    }

    m_d->model->removeFrames(framesToRemove);

    if (removeColumns) {
        QVector<QPoint> movingFrames = m_d->prepareFramesSlab(rightmostCol + 1, m_d->model->columnCount() - 1);
        m_d->model->offsetFrames(movingFrames, QPoint(-numColumns, 0), false);
    }
}

void TimelineRulerHeader::slotRemoveColumns()
{
    slotClearColumns(true);
}
