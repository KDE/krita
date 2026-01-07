/*
  SPDX-FileCopyrightText: 2006 Gábor Lehel <illissius@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIS_DOCUMENT_SECTION_VIEW_H
#define KIS_DOCUMENT_SECTION_VIEW_H

#include <QTreeView>
#include <QScroller>

#include "kritalayerdocker_export.h"

class QStyleOptionViewItem;
class KisNodeModel;

/**
 * A widget displaying the Krita nodes (layers, masks, local selections, etc.)
 * 
 * This class is designed as a Qt model-view widget.
 * 
 * The Qt documentation explains the design and terminology for these classes:
 * https://doc.qt.io/qt-5/model-view-programming.html
 *
 * This widget should work correctly in your Qt designer .ui file.
 */
class KRITALAYERDOCKER_EXPORT NodeView : public QTreeView
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

    enum ColumnIndex {
        DEFAULT_COL = 0,
        VISIBILITY_COL = 1,
        SELECTED_COL = 2,
    };

    /**
     * Create a new NodeView.
     */
    explicit NodeView(QWidget *parent = 0);
    ~NodeView() override;

    void setModel(QAbstractItemModel *model) override;
    void resizeEvent(QResizeEvent * event) override;
    void paintEvent (QPaintEvent *event) override;
    void drawBranches(QPainter *painter, const QRect &rect,
                              const QModelIndex &index) const override;

    void dropEvent(QDropEvent *ev) override;

    void dragEnterEvent(QDragEnterEvent *e) override;

    void dragMoveEvent(QDragMoveEvent *ev) override;

    void dragLeaveEvent(QDragLeaveEvent *e) override;

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
     * the toggle actions
     * @param index The model index associated with the document
     * section that may or may not provide a number of toggle actions.
     */
    void addPropertyActions(QMenu *menu, const QModelIndex &index);

    void updateNode(const QModelIndex &index);

    void toggleSolo(const QModelIndex &index);

protected:
    QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex &index, const QEvent *event) const override;
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
    void slotConfigurationChanged();

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

    void updateSelectedCheckboxColumn();

    bool m_draggingFlag;

    QStyleOptionViewItem optionForIndex(const QModelIndex &index) const;
    typedef KisNodeModel Model;
    class PropertyAction;
    class Private;
    Private* const d;
};

#endif
