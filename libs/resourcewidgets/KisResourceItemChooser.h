/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
   SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
   SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
   SPDX-FileCopyrightText: 2010 Boudewijn Rempt <boud@valdyas.org>
   SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
   SPDX-FileCopyrightText: 2011 José Luis Vergara <pentalis@gmail.com>
   SPDX-FileCopyrightText: 2013 Sascha Suelzer <s.suelzer@gmail.com>
   SPDX-FileCopyrightText: 2019 Boudewijn Rempt <boud@valdyas.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIS_RESOURCE_ITEM_CHOOSER
#define KIS_RESOURCE_ITEM_CHOOSER

#include <QWidget>

#include <QModelIndex>
#include <QListView>

#include <KoResource.h>
#include <KisKineticScroller.h>
#include "KisPopupButton.h"
#include "ResourceListViewModes.h"

class QAbstractProxyModel;
class QAbstractItemDelegate;
class QAbstractButton;
class QToolButton;
class QSortFilterProxyModel;
class KisResourceItemListView;
class KisTagFilterResourceProxyModel;

#include "kritaresourcewidgets_export.h"


/**
 * A widget that contains a KisResourceItemListView with filters for resource and tags.
 */
class KRITARESOURCEWIDGETS_EXPORT KisResourceItemChooser : public QWidget
{
    Q_OBJECT
public:
    enum Buttons { Button_Import, Button_Remove };
    enum class Layout {
        NotSet,
        Vertical,
        Horizontal2Rows,
        Horizontal1Row
    };

    /**
     * @param resourceType Type of resource to choose from.
     * @param usePreview Displays the selected resource icon to the right side of the resources view.
     *        It looks bad, should be deleted.
     */
    explicit KisResourceItemChooser(const QString &resourceType, bool usePreview = false, QWidget *parent = 0);
    ~KisResourceItemChooser() override;

    /// Enable or disable changing the layout based on size.
    /// Default is false no responsiveness, layout is vertical.
    void setResponsiveness(bool isResponsive);

    /// Set's the desired view mode for the resource list.
    /// Caller should use this instead of directly tampering with the KisResourceItemListView.
    void setListViewMode(ListViewMode viewMode);

    /// Sets the visibility of tagging KlineEdits.
    /// Default is false.
    void showTaggingBar(bool show);

    KisTagFilterResourceProxyModel *tagFilterModel() const;

    /// Show the button for changing the view mode.
    /// Default is false.
    void showViewModeBtn(bool visible);

    KisPopupButton *viewModeButton() const;

    /// Shows or hides the storage button.
    /// Default is true.
    void showStorageBtn(bool visible);

    /// Sets the height of the view rows
    void setRowHeight(int rowHeight);

    /// Sets the width of the view columns
    void setColumnWidth(int columnWidth);

    /// Sets a custom delegate for the view
    void setItemDelegate(QAbstractItemDelegate *delegate);

    /**
     * @brief Gets the currently selected resource
     * @param includeHidden If true, return the remembered resource even if
     *        it is currently not visible in the item view
     * @return The selected resource, 0 is no resource is selected
     */
    KoResourceSP currentResource(bool includeHidden = false) const;

    /// Sets the item representing the resource as selected
    void setCurrentResource(KoResourceSP resource);
    void setCurrentResource(QString resourceName);

    /**
     * Sets the selected resource, does nothing if there is no valid item
     * @param row row of the item
     * @param column column of the item
     */
    void setCurrentItem(int row);

    /// Shows the import and export buttons for the resource.
    /// Default is true.
    void showImportExportBtns(bool show);

    /// determines whether the preview right or below the splitter
    void setPreviewOrientation(Qt::Orientation orientation);

    /// determines whether the preview should tile the resource's image or not
    void setPreviewTiled(bool tiled);

    /// shows the preview converted to grayscale
    void setGrayscalePreview(bool grayscale);

    /// View size for the resources view.
    QSize viewSize() const;

    /// Do not use this to change the view mode and flow directly.
    /// Use the requestViewMode() and requestFlow() methods so as to not
    /// intervene in the responsive design layout.
    KisResourceItemListView *itemView() const;

    void setSynced(bool sync);

    /// Allows zooming with Ctrl + Mouse Wheel
    bool eventFilter(QObject *object, QEvent *event) override;

Q_SIGNALS:
    /// Emitted when the view mode for the internal KisResourceItemListView changes
    void listViewModeChanged(ListViewMode newViewMode);

    /// Emitted when a resource was selected
    void resourceSelected(KoResourceSP resource);

    /// Emitted when an *already selected* resource is clicked
    /// again
    void resourceClicked(KoResourceSP resource);

public Q_SLOTS:
    void slotButtonClicked(int button);
    void slotScrollerStateChanged(QScroller::State state){ KisKineticScroller::updateCursor(this, state); }
    void updateView();

private Q_SLOTS:
    void scrollBackwards();
    void scrollForwards();
    void activate(const QModelIndex &index);
    void clicked(const QModelIndex &index);
    void contextMenuRequested(const QPoint &pos);
    void baseLengthChanged(int length);
    void afterFilterChanged();
    void slotSaveSplitterState();

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateButtonState();
    void updatePreview(const QModelIndex &idx);

    void hideEverything();
    void applyVerticalLayout();
    void changeLayoutBasedOnSize();

    /// Resource for a given model index
    /// @returns the resource pointer, 0 is index not valid
    KoResourceSP resourceFromModelIndex(const QModelIndex &index) const;

    class Private;
    Private *const d;
};

#endif // KO_RESOURCE_ITEM_CHOOSER
