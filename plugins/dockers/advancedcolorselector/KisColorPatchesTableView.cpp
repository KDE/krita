/*
 * SPDX-FileCopyrightText: 2023 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisColorPatchesTableView.h"

#include <QCoreApplication>
#include <QHeaderView>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

#include <utility>

#include "kis_color_patches.h"

class KisColorPatchesTableDelegate : public QStyledItemDelegate
{
public:
    KisColorPatchesTableDelegate(QObject *parent)
        : QStyledItemDelegate(parent)
    {
    }
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

void KisColorPatchesTableDelegate::paint(QPainter *painter,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    const QColor color = index.data(Qt::UserRole).value<QColor>();

    if (color.isValid()) {
        // painter->fillRect(QRect(option.rect.topLeft(), size), color);
        painter->fillRect(option.rect, color);
    }
}

struct KisColorPatchesTableView::Private {
    QScopedPointer<QStandardItemModel> model;
    QList<KoColor> colorPatches;
    const QString configPrefix;
    KisColorPatches::Direction direction{KisColorPatches::Horizontal};
    int numRows {0};
    int numCols {0};
    int patchWidth {1};
    int patchHeight {1};
    int patchCount {1};
    bool wasScrollingDisabled {false};

    Private(QString configPrefix)
        : configPrefix(std::move(configPrefix))
    {
    }
};

KisColorPatchesTableView::KisColorPatchesTableView(const QString &configPrefix, QWidget *parent)
    : QTableView(parent)
    , m_d(new KisColorPatchesTableView::Private(configPrefix))
{
    setShowGrid(false);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::NoSelection);
    // makes sure that the patches don't go out of bounds, one can notice if kinetic scrolling enabled.
    setStyleSheet("QTableView{ border: 0px}");
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    reloadWidgetConfig();

    QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(this);
    if (scroller) {
        QScrollerProperties props;
        props.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy,
                              QScrollerProperties::OvershootAlwaysOff);
        props.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy,
                              QScrollerProperties::OvershootAlwaysOff);
        scroller->setScrollerProperties(props);
        connect(scroller,
                SIGNAL(stateChanged(QScroller::State)),
                this,
                SLOT(slotScrollerStateChanged(QScroller::State)));
    }
    reloadWidgetConfig();
}

KisColorPatchesTableView::~KisColorPatchesTableView()
{
}

void KisColorPatchesTableView::setColors(const QList<KoColor> &colors)
{
    m_d->colorPatches.clear();
    m_d->colorPatches = colors;
    redraw();
}

QList<KoColor> KisColorPatchesTableView::colors() const
{
    return m_d->colorPatches;
}

void KisColorPatchesTableView::reloadWidgetConfig()
{
    const KConfigGroup cfg = KSharedConfig::openConfig()->group("advancedColorSelector");

    m_d->numCols = cfg.readEntry(m_d->configPrefix + "NumCols", 1);
    m_d->numRows = cfg.readEntry(m_d->configPrefix + "NumRows", 1);

    m_d->patchWidth = cfg.readEntry(m_d->configPrefix + "Width", 20);
    m_d->patchHeight = cfg.readEntry(m_d->configPrefix + "Height", 20);
    if (cfg.readEntry(m_d->configPrefix + "Alignment", false)) {
        m_d->direction = KisColorPatches::Vertical;
    } else {
        m_d->direction = KisColorPatches::Horizontal;
    }
    m_d->patchCount = cfg.readEntry(m_d->configPrefix + "Count", 15);
    const bool allowScrolling = cfg.readEntry(m_d->configPrefix + "Scrolling", true);
    if (!allowScrolling) {
        QScroller::scroller(this)->ungrabGesture(this);
        horizontalScrollBar()->setEnabled(false);
        verticalScrollBar()->setEnabled(false);
        m_d->wasScrollingDisabled = true;
    } else if (m_d->wasScrollingDisabled) {
        QScroller *scroller = QScroller::scroller(this);
        scroller->grabGesture(this, KisKineticScroller::getConfiguredGestureType());
        horizontalScrollBar()->setEnabled(true);
        verticalScrollBar()->setEnabled(true);
        m_d->wasScrollingDisabled = false;
    }

    m_d->model.reset(new QStandardItemModel(m_d->numRows, m_d->numCols, this));

    if (m_d->direction == KisColorPatches::Vertical) {
        setMinimumWidth((m_d->patchWidth + 0) * m_d->numCols);
        setMaximumWidth((m_d->patchWidth + 0) * m_d->numCols);

        // reset if this was previously Horizontal
        setMinimumHeight(m_d->patchHeight);
        setMaximumHeight(QWIDGETSIZE_MAX);

        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
    } else {
        setMinimumHeight((m_d->patchHeight + 0) * m_d->numRows);
        setMaximumHeight((m_d->patchHeight + 0) * m_d->numRows);

        // reset if this was previously Vertical
        setMinimumWidth(m_d->patchWidth);
        setMaximumWidth(QWIDGETSIZE_MAX);

        setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    }

    // minimum should always be set before default.
    verticalHeader()->setMinimumSectionSize(m_d->patchHeight);
    verticalHeader()->setMaximumSectionSize(m_d->patchHeight);
    verticalHeader()->setDefaultSectionSize(m_d->patchHeight);
    horizontalHeader()->setMinimumSectionSize(m_d->patchWidth);
    horizontalHeader()->setMaximumSectionSize(m_d->patchWidth);
    horizontalHeader()->setDefaultSectionSize(m_d->patchWidth);

    // don't let the user resize us.
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    verticalHeader()->hide();
    horizontalHeader()->hide();

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setModel(m_d->model.data());
    setItemDelegate(new KisColorPatchesTableDelegate(this));

    redraw();
}

QSize KisColorPatchesTableView::cellSize() const
{
    return QSize(m_d->patchWidth, m_d->patchHeight);
}

int KisColorPatchesTableView::patchCount() const
{
    return m_d->patchCount;
}

void KisColorPatchesTableView::addColorPatch(const KoColor &color)
{
    m_d->colorPatches.removeAll(color);
    m_d->colorPatches.prepend(color);

    if (m_d->colorPatches.size() > 200) {
        m_d->colorPatches.pop_back();
    }

    redraw();
}

void KisColorPatchesTableView::wheelEvent(QWheelEvent *event)
{
    if (m_d->direction == KisColorPatches::Horizontal) {
        bool horizontal = qAbs(event->angleDelta().x()) > qAbs(event->angleDelta().y());
        if (!horizontal) {
            // HACK: here we make our normal vertical scroller, also do a horizontal scroll
            QPoint modifiedAngleDelta = {event->angleDelta().y(), event->angleDelta().x()};
            QWheelEvent modifiedEvent(event->pos(),
                                      event->globalPosF(),
                                      event->pixelDelta(),
                                      modifiedAngleDelta,
                                      event->buttons(),
                                      event->modifiers(),
                                      event->phase(),
                                      event->inverted());
            horizontalScrollBar()->event(&modifiedEvent);
            event->accept();
            return;
        }
    }
    QTableView::wheelEvent(event);
}

void KisColorPatchesTableView::mousePressEvent(QMouseEvent *event)
{
    event->ignore();
}

void KisColorPatchesTableView::mouseReleaseEvent(QMouseEvent *event)
{
    event->ignore();
}

void KisColorPatchesTableView::redraw()
{
    m_d->model->clear();

    if (m_d->colorPatches.isEmpty()) {
        return;
    }

    // a simple index, keeping count of how many patches have been put in the model.
    int linearIndex = 0;
    // sorry for the lack of term, think of pointer as pointing to an array and index iterating inside that
    // array.
    const int limit = ((m_d->direction == KisColorPatches::Vertical) ? m_d->numCols : m_d->numRows);
    int pointer = 0;
    // index should start from 1, because the first item is the "extra" button we have to take care of.
    int index = 1 % limit;

    Q_FOREACH (const KoColor &color, m_d->colorPatches) {
        if (linearIndex > m_d->patchCount) {
            break;
        }
        // We handle two cases:
        // (1) say if numColumn = 1 and we are on row = 0, then we should add a row than trying to draw in
        // i + 1 column (since column never changes in this case).
        // (2) we have reached the end of the "array". Create a new sequence.
        if (index == 0) {
            pointer++;
            if (m_d->direction == KisColorPatches::Vertical) {
                m_d->model->insertRow(pointer);
            } else {
                m_d->model->insertColumn(pointer);
            }
        }
        QStandardItem *item = new QStandardItem;
        item->setData(QVariant(), Qt::DisplayRole);
        item->setData(color.toQColor(), Qt::UserRole);
        if (m_d->direction == KisColorPatches::Vertical) {
            m_d->model->setItem(pointer, index, item);
        } else {
            m_d->model->setItem(index, pointer, item);
        }
        index = (index + 1) % limit;
        linearIndex++;
    }
}

boost::optional<KoColor> KisColorPatchesTableView::colorPatchAt(const QPoint &pos) const
{
    // TODO(sh_zam): Is there a better way that doesn't have to involve mapFromGlobal?
    const QModelIndex index = indexAt(mapFromGlobal(pos));
    if (!index.isValid()) {
        return boost::none;
    }
    int linearIndex = 0;
    if (m_d->direction == KisColorPatches::Vertical) {
        linearIndex = index.row() * m_d->model->columnCount() + index.column();
    } else {
        linearIndex = index.column() * m_d->model->rowCount() + index.row();
    }
    // since (0, 0) index has the additional button.
    linearIndex -= 1;

    if (linearIndex < 0 || linearIndex >= m_d->colorPatches.size()) {
        return boost::none;
    }
    return m_d->colorPatches[linearIndex];
}
