/*
  Copyright (c) 2006 Gábor Lehel <illissius@gmail.com>

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
  Boston, MA 02110-1301, USA.
*/

#ifndef KIS_DOCUMENT_SECTION_VIEW_H
#define KIS_DOCUMENT_SECTION_VIEW_H

#include <QTreeView>
#include <QScroller>

class QStyleOptionViewItem;
class KisNodeModel;

/**
 * A widget displaying the Krita nodes (layers, masks, local selections, etc.)
 * 
 * The widget can show the document sections as big thumbnails, 
 * in a listview with two rows of informative text and icons,
 * or as single rows of text and property icons.
 *
 * This class is designed as a Qt model-view widget.
 * 
 * The Qt documentation explains the design and terminology for these classes:
 * http://doc.qt.io/qt-5/model-view-programming.html
 *
 * This widget should work correctly in your Qt designer .ui file.
 */
class NodeView: public QTreeView
{
    Q_OBJECT
Q_SIGNALS:
    /**
     * Emitted whenever the user clicks with the secondary mouse
     * button on an item. It is up to the application to design the
     * contents of the context menu and show it.
     */
    void contextMenuRequested(const QPoint &globalPos, const QModelIndex &index);
    void selectionChanged(const QModelIndexList &);
public:

    /**
     * Create a new NodeView.
     */
    explicit NodeView(QWidget *parent = 0);
    ~NodeView() override;

    /// how items should be displayed
    enum DisplayMode {
        /// large fit-to-width thumbnails, with only titles or page numbers
        ThumbnailMode,

        /// smaller thumbnails, with titles and property icons in two rows
        DetailedMode,

        /// no thumbnails, with titles and property icons in a single row
        MinimalMode
    };

    void resizeEvent(QResizeEvent * event) override;
    void paintEvent (QPaintEvent *event) override;
    void drawBranches(QPainter *painter, const QRect &rect,
                              const QModelIndex &index) const override;

    void dropEvent(QDropEvent *ev) override;

    void dragEnterEvent(QDragEnterEvent *e) override;

    void dragMoveEvent(QDragMoveEvent *ev) override;

    void dragLeaveEvent(QDragLeaveEvent *e) override;

    /**
     * Set the display mode of the view to one of the options.
     *
     * @param mode The NodeView::DisplayMode mode
     */
    void setDisplayMode(DisplayMode mode);

    /**
     * @return the currently active display mode
     */
    DisplayMode displayMode() const;

    /**
     * Add toggle actions for all the properties associated with the
     * current document section associated with the model index to the
     * specified menu.
     *
     * For instance, if a document section can be locked and visible,
     * the menu will be expanded with locked and visible toggle
     * actions.
     *
     * For instance
     @code
     NodeView * nodeView;
     QModelIndex index = getCurrentNode();
     QMenu menu;
     if (index.isValid()) {
         sectionView->addPropertyActions(&menu, index);
     } else {
         menu.addAction(...); // Something to create a new document section, for example.
     }

     @endcode
     *
     * @param menu A pointer to the menu that will be expanded with
     * the toglge actions
     * @param index The model index associated with the document
     * section that may or may not provide a number of toggle actions.
     */
    void addPropertyActions(QMenu *menu, const QModelIndex &index);

    void updateNode(const QModelIndex &index);

    QRect originalVisualRect(const QModelIndex &index) const;

protected:
    QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex &index,
                                                         const QEvent *event) const override;

    QRect visualRect(const QModelIndex &index) const override;
    QModelIndex indexAt(const QPoint &point) const override;
    bool viewportEvent(QEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    virtual void showContextMenu(const QPoint &globalPos, const QModelIndex &index);
    void startDrag (Qt::DropActions supportedActions) override;
    QPixmap createDragPixmap() const;

    /**
     * Calculates the index of the nearest item to the cursor position
     */
    int cursorPageIndex() const;

public Q_SLOTS:
    /// called with a theme change to refresh icon colors
    void slotUpdateIcons();
    void slotScrollerStateChanged(QScroller::State state);

protected Q_SLOTS:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>()) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

private Q_SLOTS:
    void slotActionToggled(bool on, const QPersistentModelIndex &index, int property);

private:

    /**
     * Permit to know if a slide is dragging
     *
     * @return boolean
     */
    bool isDragging() const;

    /**
     * Setter for the dragging flag
     *
     * @param flag boolean
     */
    void setDraggingFlag(bool flag = true);

    bool m_draggingFlag;

    QStyleOptionViewItem optionForIndex(const QModelIndex &index) const;
    typedef KisNodeModel Model;
    class PropertyAction;
    class Private;
    Private* const d;
};

#endif
