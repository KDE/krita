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

#include <QMenu>
#include <QAction>
#include <QPainter>
#include <QPaintEvent>

#include <klocalizedstring.h>

#include "kis_time_based_item_model.h"
#include "timeline_color_scheme.h"
#include "kis_action.h"

#include "kis_debug.h"

struct TimelineRulerHeader::Private
{
    Private() : fps(12), lastPressSectionIndex(-1) {}

    int fps;

    KisTimeBasedItemModel *model;
    int lastPressSectionIndex;

    int calcSpanWidth(const int sectionWidth);
    QModelIndexList prepareFramesSlab(int startCol, int endCol);

    KisActionManager* actionMan = 0;

};

TimelineRulerHeader::TimelineRulerHeader(QWidget *parent)
    : QHeaderView(Qt::Horizontal, parent),
      m_d(new Private)
{
    setSectionResizeMode(QHeaderView::Fixed);
    setDefaultSectionSize(18);
    setMinimumSectionSize(8);
}

TimelineRulerHeader::~TimelineRulerHeader()
{
}


void TimelineRulerHeader::setActionManager(KisActionManager *actionManager)
{
    m_d->actionMan = actionManager;

    if (actionManager) {
        KisAction *action;

        action = actionManager->createAction("insert_column_left");
        connect(action, SIGNAL(triggered()), SIGNAL(sigInsertColumnLeft()));

        action = actionManager->createAction("insert_column_right");
        connect(action, SIGNAL(triggered()), SIGNAL(sigInsertColumnRight()));

        action = actionManager->createAction("insert_multiple_columns");
        connect(action, SIGNAL(triggered()), SIGNAL(sigInsertMultipleColumns()));

        action = actionManager->createAction("remove_columns_and_pull");
        connect(action, SIGNAL(triggered()), SIGNAL(sigRemoveColumnsAndShift()));

        action = actionManager->createAction("remove_columns");
        connect(action, SIGNAL(triggered()), SIGNAL(sigRemoveColumns()));

        action = actionManager->createAction("insert_hold_column");
        connect(action, SIGNAL(triggered()), SIGNAL(sigInsertHoldColumns()));

        action = actionManager->createAction("insert_multiple_hold_columns");
        connect(action, SIGNAL(triggered()), SIGNAL(sigInsertHoldColumnsCustom()));

        action = actionManager->createAction("remove_hold_column");
        connect(action, SIGNAL(triggered()), SIGNAL(sigRemoveHoldColumns()));

        action = actionManager->createAction("remove_multiple_hold_columns");
        connect(action, SIGNAL(triggered()), SIGNAL(sigRemoveHoldColumnsCustom()));

        action = actionManager->createAction("mirror_columns");
        connect(action, SIGNAL(triggered()), SIGNAL(sigMirrorColumns()));

        action = actionManager->createAction("copy_columns_to_clipboard");
        connect(action, SIGNAL(triggered()), SIGNAL(sigCopyColumns()));

        action = actionManager->createAction("cut_columns_to_clipboard");
        connect(action, SIGNAL(triggered()), SIGNAL(sigCutColumns()));

        action = actionManager->createAction("paste_columns_from_clipboard");
        connect(action, SIGNAL(triggered()), SIGNAL(sigPasteColumns()));
    }
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

    QStyleOptionViewItem option = viewOptions();
    const int gridHint = style()->styleHint(QStyle::SH_Table_GridLineColor, &option, this);
    const QColor gridColor = static_cast<QRgb>(gridHint);
    const QPen gridPen = QPen(gridColor);

    if (!internalIndex) {
        bool isIntegralLine = (logicalIndex + spanWidth) % m_d->fps == 0;
        bool isPrevIntegralLine = logicalIndex % m_d->fps == 0;
        paintSpan(painter, userFrameId, spanRect, isIntegralLine, isPrevIntegralLine, style(), palette(), gridPen);
    }

    {
        QBrush fillColor = TimelineColorScheme::instance()->headerEmpty();

        QVariant activeValue = model()->headerData(logicalIndex, orientation(),
                                                   KisTimeBasedItemModel::ActiveFrameRole);

        QVariant cachedValue = model()->headerData(logicalIndex, orientation(),
                                                   KisTimeBasedItemModel::FrameCachedRole);

        if (activeValue.isValid() && activeValue.toBool()) {
            fillColor = TimelineColorScheme::instance()->headerActive();
        } else if (cachedValue.isValid() && cachedValue.toBool()) {
            fillColor = TimelineColorScheme::instance()->headerCachedFrame();
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

bool TimelineRulerHeader::setZoom(qreal zoom)
{
    const int minSectionSize = 4;
    const int unitSectionSize = 18;

    int newSectionSize = zoom * unitSectionSize;

    if (newSectionSize < minSectionSize) {
        newSectionSize = minSectionSize;
        zoom = qreal(newSectionSize) / unitSectionSize;
    }

    if (newSectionSize != defaultSectionSize()) {
        setDefaultSectionSize(newSectionSize);
        return true;
    }

    return false;
}

void TimelineRulerHeader::updateMinimumSize()
{
    QFontMetrics metrics(this->font());
    const int textHeight = metrics.height();

    setMinimumSize(0, 1.5 * textHeight);
}

void TimelineRulerHeader::setModel(QAbstractItemModel *model)
{
    KisTimeBasedItemModel *framesModel = qobject_cast<KisTimeBasedItemModel*>(model);
    m_d->model = framesModel;

    QHeaderView::setModel(model);
}

int getColumnCount(const QModelIndexList &indexes, int *leftmostCol, int *rightmostCol)
{
    QVector<int> columns;
    int leftmost = std::numeric_limits<int>::max();
    int rightmost = std::numeric_limits<int>::min();

    Q_FOREACH (const QModelIndex &index, indexes) {
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
                model()->setHeaderData(logical, orientation(), true, KisTimeBasedItemModel::ActiveFrameRole);
            }

            /* Fix for safe-assert involving kis_animation_curve_docker.
             * There should probably be a more elagant way for dealing
             * with reused timeline_ruler_header instances in other
             * timeline views instead of simply animation_frame_view.
             *
             * This works for now though... */
            if(!m_d->actionMan){
                return;
            }

            QMenu menu;
            KisActionManager::safePopulateMenu(&menu, "cut_columns_to_clipboard", m_d->actionMan);
            KisActionManager::safePopulateMenu(&menu, "copy_columns_to_clipboard", m_d->actionMan);
            KisActionManager::safePopulateMenu(&menu, "paste_columns_from_clipboard", m_d->actionMan);

            menu.addSeparator();

            {   //Frame Columns Submenu
                QMenu *frames = menu.addMenu(i18nc("@item:inmenu", "Keyframe Columns"));
                KisActionManager::safePopulateMenu(frames, "insert_column_left", m_d->actionMan);
                KisActionManager::safePopulateMenu(frames, "insert_column_right", m_d->actionMan);
                frames->addSeparator();
                KisActionManager::safePopulateMenu(frames, "insert_multiple_columns", m_d->actionMan);
            }

            {   //Hold Columns Submenu
                QMenu *hold = menu.addMenu(i18nc("@item:inmenu", "Hold Frame Columns"));
                KisActionManager::safePopulateMenu(hold, "insert_hold_column", m_d->actionMan);
                KisActionManager::safePopulateMenu(hold, "remove_hold_column", m_d->actionMan);
                hold->addSeparator();
                KisActionManager::safePopulateMenu(hold, "insert_multiple_hold_columns", m_d->actionMan);
                KisActionManager::safePopulateMenu(hold, "remove_multiple_hold_columns", m_d->actionMan);
            }

            menu.addSeparator();

            KisActionManager::safePopulateMenu(&menu, "remove_columns", m_d->actionMan);
            KisActionManager::safePopulateMenu(&menu, "remove_columns_and_pull", m_d->actionMan);

            if (numSelectedColumns > 1) {
                menu.addSeparator();
                KisActionManager::safePopulateMenu(&menu, "mirror_columns", m_d->actionMan);
            }

            menu.exec(e->globalPos());

            return;

        } else if (e->button() == Qt::LeftButton) {
            m_d->lastPressSectionIndex = logical;
            model()->setHeaderData(logical, orientation(), true, KisTimeBasedItemModel::ActiveFrameRole);
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
            model()->setHeaderData(logical, orientation(), true, KisTimeBasedItemModel::ActiveFrameRole);

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

QModelIndexList TimelineRulerHeader::Private::prepareFramesSlab(int startCol, int endCol)
{
    QModelIndexList frames;

    const int numRows = model->rowCount();

    for (int i = 0; i < numRows; i++) {
        for (int j = startCol; j <= endCol; j++) {
            QModelIndex index = model->index(i, j);
            const bool exists = model->data(index, KisTimeBasedItemModel::FrameExistsRole).toBool();
            if (exists) {
                frames << index;
            }
        }
    }

    return frames;
}
