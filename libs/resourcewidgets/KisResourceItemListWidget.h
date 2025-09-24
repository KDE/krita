/*
 * SPDX-FileCopyrightText: 2023 Srirupa Datta <srirupa.sps@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KISRESOURCEITEMLISTWIDGET_H
#define KISRESOURCEITEMLISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QScopedPointer>

#include <KisKineticScroller.h>
#include "kritaresourcewidgets_export.h"
#include "ResourceListViewModes.h"


class KRITARESOURCEWIDGETS_EXPORT KisResourceItemListWidget : public QListWidget
{
    Q_OBJECT

public:
    KisResourceItemListWidget(QWidget *parent = nullptr);
    ~KisResourceItemListWidget() override;

    void setListViewMode(ListViewMode layout);

    /**
     * @brief setItemSize
     * convenience function which sets both the icon and the grid size
     * to the same value.
     * @param size - the size you wish either to be.
     */
    void setItemSize(QSize size);

    /**
     * @brief setStrictSelectionMode sets additional restrictions on the selection.
     *
     * When in QAbstractItemView::SingleSelection mode, this ensures that the
     * selection never gets transferred to another item. Instead, the selection
     * is cleared if the current item gets removed (filtered) from the model.
     * Furthermore, it prevents users from deselecting the current item with Ctrl+click.
     * This behavior is important for resource selectors.
     * @param enable Determines if strict mode is enabled.
     */
    void setStrictSelectionMode(bool enable);

    void setFixedToolTipThumbnailSize(const QSize &size);
    void setToolTipShouldRenderCheckers(bool value);

public Q_SLOTS:
    void slotScrollerStateChange(QScroller::State state){ KisKineticScroller::updateCursor(this, state); }

Q_SIGNALS:

    void sigSizeChanged();

    void currentResourceChanged(const QModelIndex &);
    void currentResourceClicked(const QModelIndex &);

    void contextMenuRequested(const QPoint &);

protected Q_SLOTS:
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

protected:
    QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex &index, const QEvent *event = nullptr) const override;
    void contextMenuEvent(QContextMenuEvent *event) override;

    bool viewportEvent(QEvent *event) override;

private:
    void resizeEvent(QResizeEvent *event) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISRESOURCEITEMLISTWIDGET_H

