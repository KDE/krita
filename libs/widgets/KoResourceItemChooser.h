/* This file is part of the KDE project
   Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
   Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
   Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
   Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
   Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>

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

#ifndef KO_RESOURCE_ITEM_CHOOSER
#define KO_RESOURCE_ITEM_CHOOSER

#include <QWidget>
#include <QScroller>

#include "kritawidgets_export.h"
#include <QPushButton>

class QModelIndex;
class QAbstractProxyModel;
class QAbstractItemDelegate;
class QAbstractButton;
class QToolButton;
class KoAbstractResourceServerAdapter;
class KoResourceItemView;
class KoResource;

/**
 * A widget that contains a KoResourceChooser as well
 * as an import/export button
 */
class KRITAWIDGETS_EXPORT KoResourceItemChooser : public QWidget
{
    Q_OBJECT
public:
    enum Buttons { Button_Import, Button_Remove };

    /// \p usePreview shows the aside preview with the resource's image
    explicit KoResourceItemChooser(QSharedPointer<KoAbstractResourceServerAdapter> resourceAdapter, QWidget *parent = 0, bool usePreview = false);
    ~KoResourceItemChooser() override;

    /// Sets number of columns in the view and causes the number of rows to be calculated accordingly
    void setColumnCount(int columnCount);

    /// Sets number of rows in the view and causes the number of columns to be calculated accordingly
    void setRowCount(int rowCount);

    /// Sets the height of the view rows
    void setRowHeight(int rowHeight);

    /// Sets the width of the view columns
    void setColumnWidth(int columnWidth);

    /// Sets a custom delegate for the view
    void setItemDelegate(QAbstractItemDelegate *delegate);

    /// Gets the currently selected resource
    /// @returns the selected resource, 0 is no resource is selected
    KoResource *currentResource() const;

    /// Sets the item representing the resource as selected
    void setCurrentResource(KoResource *resource);

    /**
     * Sets the selected resource, does nothing if there is no valid item
     * @param row row of the item
     * @param column column of the item
     */
    void setCurrentItem(int row, int column);

    void showButtons(bool show);

    void addCustomButton(QAbstractButton *button, int cell);

    /// determines whether the preview right or below the splitter
    void setPreviewOrientation(Qt::Orientation orientation);
    /// determines whether the preview should tile the resource's image or not
    void setPreviewTiled(bool tiled);
    /// shows the preview converted to grayscale
    void setGrayscalePreview(bool grayscale);

    /// sets the visibilty of tagging KlineEdits.
    void showTaggingBar(bool show);

    ///Set a proxy model with will be used to filter the resources
    void setProxyModel(QAbstractProxyModel *proxyModel);

    QSize viewSize() const;

    KoResourceItemView *itemView() const;

    void setViewModeButtonVisible(bool visible);
    QToolButton *viewModeButton() const;

    void setSynced(bool sync);

    bool eventFilter(QObject *object, QEvent *event) override;

    /// sets up this chooser for kinetic (drag triggered) scrolling
    void configureKineticScrolling(int gesture, int sensitivity, bool scrollbar);

Q_SIGNALS:
    /// Emitted when a resource was selected
    void resourceSelected(KoResource *resource);
    /// Emitted when an *already selected* resource is clicked
    /// again
    void resourceClicked(KoResource *resource);
    void splitterMoved();
public Q_SLOTS:
    void slotButtonClicked(int button);

private Q_SLOTS:
    void activated(const QModelIndex &index);
    void clicked(const QModelIndex &index);
    void contextMenuRequested(const QPoint &pos);
    void baseLengthChanged(int length);
    void updateView();
    void slotBeforeResourcesLayoutReset(KoResource *activateAfterReset);
    void slotAfterResourcesLayoutReset();

protected:
    void showEvent(QShowEvent *event) override;

private:
    void updateButtonState();
    void updatePreview(KoResource *resource);



    void resizeEvent(QResizeEvent *event) override;

    /// Resource for a given model index
    /// @returns the resource pointer, 0 is index not valid
    KoResource *resourceFromModelIndex(const QModelIndex &index) const;

    class Private;
    Private *const d;

   QPushButton *importButton;
   QPushButton *deleteButton;

};

#endif // KO_RESOURCE_ITEM_CHOOSER
