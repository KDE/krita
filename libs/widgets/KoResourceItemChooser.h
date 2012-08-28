/* This file is part of the KDE project
   Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
   Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
   Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
   Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>

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
#include <QModelIndex>
#include <QCompleter>

#include "kowidgets_export.h"
#include <KoConfig.h>

class QAbstractProxyModel;
class QButtonGroup;
class QAbstractItemDelegate;
class KoAbstractResourceServerAdapter;
class KoResourceItemView;
class KoResource;

/**
 * A widget that contains a KoResourceChooser as well
 * as an import/export button
 */
class KOWIDGETS_EXPORT KoResourceItemChooser : public QWidget
{
  Q_OBJECT
public:
    enum Buttons { Button_Import, Button_Remove, Button_GhnsDownload, Button_GhnsUpload };
    
    explicit KoResourceItemChooser( KoAbstractResourceServerAdapter * resourceAdapter, QWidget *parent = 0 );
    ~KoResourceItemChooser();

    /// Sets number of columns in the view and causes the number of rows to be calculated accordingly
    void setColumnCount( int columnCount );

    /// Sets number of rows in the view and causes the number of columns to be calculated accordingly
    void setRowCount( int rowCount );
    
    /// Sets the height of the view rows
    void setRowHeight( int rowHeight );

    /// Sets the width of the view columns
    void setColumnWidth( int columnWidth );
    
    /// Sets a custom delegate for the view
    void setItemDelegate( QAbstractItemDelegate * delegate );

    /// Gets the currently selected resource
    /// @returns the selected resource, 0 is no resource is selected
    KoResource *currentResource();

    /// Sets the item representing the resource as selected
    void setCurrentResource(KoResource* resource);

    /**
     * Sets the sected resource, does nothing if there is no valid item
     * @param row row of the item
     * @param column column of the item
     */
    void setCurrentItem(int row, int column);

    void showButtons( bool show );

    /// shows the aside preview with the resource's image
    void showPreview(bool show);
    /// determines whether the preview right or below the splitter
    void setPreviewOrientation(Qt::Orientation orientation);
    /// determines whether the preview should tile the resource's image or not
    void setPreviewTiled(bool tiled);
    /// shows the preview converted to grayscale
    void setGrayscalePreview(bool grayscale);


    void showGetHotNewStuff( bool showDownload, bool showUpload);
    /// sets the visibilty of tagging KlineEdits.
    void showTaggingBar( bool showSearchBar, bool showOpBar );

    ///Set a proxy model with will be used to filter the resources
    void setProxyModel( QAbstractProxyModel* proxyModel );

    void setKnsrcFile(const QString& knsrcFileArg);
    QSize viewSize();
    /// Gets the tagged resource names from tagObject in resource Server
    QStringList getTaggedResourceFileNames(QString lineEditText);
    /// Gets the tag Names from tag Object for setting the Completer Object
    QStringList getTagNamesList(QString lineEditText);

signals:
    /// Emitted when a resource was selected
    void resourceSelected( KoResource * resource );
    void splitterMoved();
public slots:
    void slotButtonClicked( int button );
    
private slots:
    void activated ( const QModelIndex & index );

    void setTagOpLineEdit(QStringList assignedTagsList);

    void tagOpLineEditActivated(QString lineEditText);
    void tagOpLineEditTextChanged(QString lineEditText);

    void tagSearchLineEditActivated(QString lineEditText);
    void tagSearchLineEditTextChanged(QString lineEditText);

private:
    void updateButtonState();
    void updatePreview(KoResource *resource);

    /// Resource for a given model index
    /// @returns the resource pointer, 0 is index not valid
    KoResource* resourceFromModelIndex(const QModelIndex & index );

    class Private;
    Private * const d;
};

#endif // KO_RESOURCE_ITEM_CHOOSER
