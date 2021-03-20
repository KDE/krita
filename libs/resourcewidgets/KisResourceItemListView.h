/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2019 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISRESOURCEITEMLISTVIEW_H
#define KISRESOURCEITEMLISTVIEW_H

#include <QWidget>
#include <QListView>

#include <KisKineticScroller.h>

#include "KisIconToolTip.h"

#include "kritaresourcewidgets_export.h"

class KRITARESOURCEWIDGETS_EXPORT KisResourceItemListView : public QListView
{
    Q_OBJECT

public:
    KisResourceItemListView(QWidget *parent = nullptr);

    /**
     * @brief setItemSize
     * convenience function which sets both the icon and the grid size
     * to the same value.
     * @param size - the size you wish either to be.
     */
    void setItemSize(QSize size);

    void setFixedToolTipThumbnailSize(const QSize &size);
    void setToolTipShouldRenderCheckers(bool value);

public Q_SLOTS:
    void slotScrollerStateChange(QScroller::State state){ KisKineticScroller::updateCursor(this, state); }

Q_SIGNALS:

    void sigSizeChanged();

    void currentResourceChanged(const QModelIndex &);
    void currentResourceClicked(const QModelIndex &);

    void contextMenuRequested(const QPoint &);

protected:
    friend class KisResourceItemChooser;

    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

    bool viewportEvent(QEvent *event) override;

private:
    KisIconToolTip m_tip;
};

#endif // KISRESOURCEITEMLISTVIEW_H
