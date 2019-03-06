/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_NODE_MODEL
#define KIS_NODE_MODEL

#include "kritaui_export.h"
#include <kis_types.h>
#include <QAbstractItemModel>
#include <QIcon>
#include <QList>
#include <QString>
#include <QVariant>

#include <KisSelectionTags.h>

class KisDummiesFacadeBase;
class KisNodeDummy;
class KisShapeController;
class KisModelIndexConverterBase;
class KisNodeSelectionAdapter;
class KisNodeInsertionAdapter;
class KisSelectionActionsAdapter;
class KisNodeDisplayModeAdapter;
class KisNodeManager;

/**
 * KisNodeModel offers a Qt model-view compatible view of the node
 * hierarchy. The KisNodeView displays a thumbnail and a row of
 * icon properties for every document section.
 *
 * Note that there's a discrepancy between the krita node tree model
 * and the model Qt wants to see: we hide the root node from Qt.
 *
 * The node model also shows an inverse view of the layer tree: we want
 * the first layer to show up at the bottom.
 * 
 * See also the Qt documentation for QAbstractItemModel. 
 * This class extends that interface to provide a name and set of toggle
 * properties (like visible, locked, selected.)
 * 
 */
class KRITAUI_EXPORT KisNodeModel : public QAbstractItemModel
{

    Q_OBJECT

public:
    /// Extensions to Qt::ItemDataRole.
    enum ItemDataRole
    {
        /// Whether the section is the active one
        ActiveRole = Qt::UserRole + 1,

        /// A list of properties the part has.
        PropertiesRole,

        /// The aspect ratio of the section as a floating point value: width divided by height.
        AspectRatioRole,

        /// Use to communicate a progress report to the section delegate on an action (a value of -1 or a QVariant() disable the progress bar
        ProgressRole,

        /// Speacial activation role which is emitted when the user Atl-clicks on a section
        /// The item is first activated with ActiveRole, then a separate AlternateActiveRole comes
        AlternateActiveRole,

        // When a layer is not (recursively) visible, then it should be gayed out
        ShouldGrayOutRole,

        // An index of a color label associated with the node
        ColorLabelIndexRole,

        // Instruct this model to update all its items' Qt::ItemIsDropEnabled flags in order to
        // reflect if the item allows an "onto" drop of the given QMimeData*.
        DropEnabled,

        // Instructs the model to activate "select opaque" action,
        // the selection action (of type SelectionAction) value
        // is passed via QVariant as integer
        SelectOpaqueRole,

        /// This is to ensure that we can extend the data role in the future, since it's not possible to add a role after BeginThumbnailRole (due to "Hack")
        ReservedRole = Qt::UserRole + 99,

        /**
         * For values of BeginThumbnailRole or higher, a thumbnail of the layer of which neither dimension
         * is larger than (int) value - (int) BeginThumbnailRole.
         * This is a hack to work around the fact that Interview doesn't have a nice way to
         * request thumbnails of arbitrary size.
         */
        BeginThumbnailRole
    };

public: // from QAbstractItemModel

    KisNodeModel(QObject * parent);
    ~KisNodeModel() override;

    void setDummiesFacade(KisDummiesFacadeBase *dummiesFacade,
                          KisImageWSP image,
                          KisShapeController *shapeController,
                          KisSelectionActionsAdapter *selectionActionsAdapter,
                          KisNodeManager *nodeManager);
    KisNodeSP nodeFromIndex(const QModelIndex &index) const;
    QModelIndex indexFromNode(KisNodeSP node) const;

    bool showGlobalSelection() const;

public Q_SLOTS:
    void setShowGlobalSelection(bool value);

public:

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList & indexes) const override;
    bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;
    bool hasDummiesFacade();

    static bool belongsToIsolatedGroup(KisImageSP image, KisNodeSP node, KisDummiesFacadeBase *dummiesFacade);

Q_SIGNALS:

    void toggleIsolateActiveNode();

protected Q_SLOTS:
    void slotBeginInsertDummy(KisNodeDummy *parent, int index, const QString &metaObjectType);
    void slotEndInsertDummy(KisNodeDummy *dummy);
    void slotBeginRemoveDummy(KisNodeDummy *dummy);
    void slotEndRemoveDummy();
    void slotDummyChanged(KisNodeDummy *dummy);

    void slotIsolatedModeChanged();

    void slotNodeDisplayModeChanged(bool showRootNode, bool showGlobalSelectionMask);

    void processUpdateQueue();
    void progressPercentageChanged(int, const KisNodeSP);

protected:
    virtual KisModelIndexConverterBase *createIndexConverter();
    KisModelIndexConverterBase *indexConverter() const;
    KisDummiesFacadeBase *dummiesFacade() const;

private:
    friend class KisModelIndexConverter;
    friend class KisModelIndexConverterShowAll;

    void connectDummy(KisNodeDummy *dummy, bool needConnect);
    void connectDummies(KisNodeDummy *dummy, bool needConnect);

    void resetIndexConverter();

    void regenerateItems(KisNodeDummy *dummy);
    bool belongsToIsolatedGroup(KisNodeSP node) const;

	void setDropEnabled(const QMimeData *data);
	void updateDropEnabled(const QList<KisNodeSP> &nodes, QModelIndex parent = QModelIndex());
    
private:

    struct Private;
    Private * const m_d;
};

#endif
