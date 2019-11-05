/* This file is part of the KDE project
   Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
   Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
   Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
   Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
   Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>
   Copyright (c) 2019 Boudewijn Rempt <boud@valdyas.org>   

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KIS_RESOURCE_ITEM_CHOOSER
#define KIS_RESOURCE_ITEM_CHOOSER

#include <QWidget>

#include <QPushButton>
#include <QModelIndex>

#include <KoResource.h>
#include <KisKineticScroller.h>

class QAbstractProxyModel;
class QAbstractItemDelegate;
class QAbstractButton;
class QToolButton;
class QSortFilterProxyModel;
class KisResourceItemListView;

#include "kritaresourcewidgets_export.h"


/**
 * A widget that contains a KoResourceChooser as well
 * as an import/export button
 */
class KRITARESOURCEWIDGETS_EXPORT KisResourceItemChooser : public QWidget
{
    Q_OBJECT
public:
    enum Buttons { Button_Import, Button_Remove };

    /// \p usePreview shows the aside preview with the resource's image
    /// \p extraFilterProxy is an extra filter proxy model for additional filtering. KisResourceItemChooser will take over ownership
    explicit KisResourceItemChooser(const QString &resourceType, bool usePreview = false, QWidget *parent = 0, QSortFilterProxyModel *extraFilterProxy = 0);
    ~KisResourceItemChooser() override;

    /// return the number of rows in the view
    int rowCount() const;

    /// Sets the height of the view rows
    void setRowHeight(int rowHeight);

    /// Sets the width of the view columns
    void setColumnWidth(int columnWidth);

    /// Sets a custom delegate for the view
    void setItemDelegate(QAbstractItemDelegate *delegate);

    /// Gets the currently selected resource
    /// @returns the selected resource, 0 is no resource is selected
    KoResourceSP currentResource() const;

    /// Sets the item representing the resource as selected
    void setCurrentResource(KoResourceSP resource);

    /**
     * Sets the selected resource, does nothing if there is no valid item
     * @param row row of the item
     * @param column column of the item
     */
    void setCurrentItem(int row);

    void showButtons(bool show);

    void addCustomButton(QAbstractButton *button, int cell);

    /// determines whether the preview right or below the splitter
    void setPreviewOrientation(Qt::Orientation orientation);
    /// determines whether the preview should tile the resource's image or not
    void setPreviewTiled(bool tiled);
    /// shows the preview converted to grayscale
    void setGrayscalePreview(bool grayscale);

    /// sets the visibility of tagging KlineEdits.
    void showTaggingBar(bool show);

    QSize viewSize() const;

    KisResourceItemListView *itemView() const;

    void setViewModeButtonVisible(bool visible);
    QToolButton *viewModeButton() const;

    void setSynced(bool sync);

    bool eventFilter(QObject *object, QEvent *event) override;

Q_SIGNALS:
    /// Emitted when a resource was selected
    void resourceSelected(KoResourceSP resource);
    /// Emitted when an *already selected* resource is clicked
    /// again
    void resourceClicked(KoResourceSP resource);
    void splitterMoved();

public Q_SLOTS:
    void slotButtonClicked(int button);
    void slotScrollerStateChanged(QScroller::State state){ KisKineticScroller::updateCursor(this, state); }

private Q_SLOTS:
    void activated(const QModelIndex &index);
    void clicked(const QModelIndex &index);
    void contextMenuRequested(const QPoint &pos);
    void baseLengthChanged(int length);
    void updateView();
    void slotBeforeResourcesLayoutReset(QModelIndex activateAfterReset);
    void slotAfterResourcesLayoutReset();

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateButtonState();
    void updatePreview(const QModelIndex &idx);

    /// Resource for a given model index
    /// @returns the resource pointer, 0 is index not valid
    KoResourceSP resourceFromModelIndex(const QModelIndex &index) const;

    class Private;
    Private *const d;


};

#endif // KO_RESOURCE_ITEM_CHOOSER
