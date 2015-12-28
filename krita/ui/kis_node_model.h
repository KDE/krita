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

class KisDummiesFacadeBase;
class KisNodeDummy;
class KisShapeController;
class KisModelIndexConverterBase;
class KisNodeSelectionAdapter;

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

public: // from QAbstractItemModel

    KisNodeModel(QObject * parent);
    ~KisNodeModel();

    void setDummiesFacade(KisDummiesFacadeBase *dummiesFacade, KisImageWSP image, KisShapeController *shapeController, KisNodeSelectionAdapter *nodeSelectionAdapter);
    KisNodeSP nodeFromIndex(const QModelIndex &index) const;
    QModelIndex indexFromNode(KisNodeSP node) const;

    bool showGlobalSelection() const;
    

public Q_SLOTS:
    void setShowGlobalSelection(bool value);

public:

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QStringList mimeTypes() const;
    QMimeData* mimeData(const QModelIndexList & indexes) const;
    bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent);
    Qt::DropActions supportedDragActions() const;
    Qt::DropActions supportedDropActions() const;
    bool hasDummiesFacade();

    static bool belongsToIsolatedGroup(KisImageSP image, KisNodeSP node, KisDummiesFacadeBase *dummiesFacade);

Q_SIGNALS:

    void nodeActivated(KisNodeSP);
    void toggleIsolateActiveNode();

    void requestAddNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);
    void requestMoveNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);

protected Q_SLOTS:
    void slotBeginInsertDummy(KisNodeDummy *parent, int index, const QString &metaObjectType, bool isAnimated);
    void slotEndInsertDummy(KisNodeDummy *dummy);
    void slotBeginRemoveDummy(KisNodeDummy *dummy);
    void slotEndRemoveDummy();
    void slotDummyChanged(KisNodeDummy *dummy);

    void slotIsolatedModeChanged();

    void updateSettings();
    void processUpdateQueue();
    void progressPercentageChanged(int, const KisNodeSP);

protected:
    virtual KisModelIndexConverterBase *createIndexConverter();
    KisModelIndexConverterBase *indexConverter() const;
    KisDummiesFacadeBase *dummiesFacade() const;

private:
    friend class KisModelIndexConverter;
    friend class KisModelIndexConverterShowAll;
    friend class KisModelIndexConverterAnimatedLayers;

    void connectDummy(KisNodeDummy *dummy, bool needConnect);
    void connectDummies(KisNodeDummy *dummy, bool needConnect);

    void resetIndexConverter();

    bool correctNewNodeLocation(KisNodeSP node,
                                KisNodeDummy* &parentDummy,
                                KisNodeDummy* &aboveThisDummy);

    void regenerateItems(KisNodeDummy *dummy);
    bool belongsToIsolatedGroup(KisNodeSP node) const;

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

        /// This is to ensure that we can extend the data role in the future, since it's not possible to add a role after BeginThumbnailRole (due to "Hack")
        ReservedRole = 99,

        /**
         * For values of BeginThumbnailRole or higher, a thumbnail of the layer of which neither dimension
         * is larger than (int) value - (int) BeginThumbnailRole.
         * This is a hack to work around the fact that Interview doesn't have a nice way to
         * request thumbnails of arbitrary size.
         */
        BeginThumbnailRole
    };
    
    
    /**
     *  Describes a property of a document section.
     *
     * FIXME: using a QList instead of QMap and not having an untranslated identifier,
     * either enum or string, forces applications to rely on the order of properties
     * or to compare the translated strings. This makes it hard to robustly extend the
     * properties of document section items.
     */
    struct Property
    {
        /** i18n-ed name, suitable for displaying */
        QString name;

        /** Whether the property is a boolean (e.g. locked, visible) which can be toggled directly from the widget itself. */
        bool isMutable;

        /** Provide these if the property isMutable. */
        QIcon onIcon;
        QIcon offIcon;

        /** If the property isMutable, provide a boolean. Otherwise, a string suitable for displaying. */
        QVariant state;

        /** If the property is mutable, specifies whether it can be put into stasis. When a property
        is in stasis, a new state is created, and the old one is stored in stateInStasis. When
        stasis ends, the old value is restored and the new one discarded */
        bool canHaveStasis;
        
        /** If the property isMutable and canHaveStasis, indicate whether it is in stasis or not */
        bool isInStasis;

        /** If the property isMutable and canHaveStasis, provide this value to store the property's
        state while in stasis */
        bool stateInStasis;

        /// Default constructor. Use if you want to assign the members manually.
        Property(): isMutable( false ) { }

        /// Constructor for a mutable property.
        Property( const QString &n, const QIcon &on, const QIcon &off, bool isOn )
                : name( n ), isMutable( true ), onIcon( on ), offIcon( off ), state( isOn ), canHaveStasis( false ) { }
        
        /** Constructor for a mutable property accepting stasis */
        Property( const QString &n, const QIcon &on, const QIcon &off, bool isOn,
                  bool isInStasis, bool stateInStasis )
                : name( n ), isMutable( true ), onIcon( on ), offIcon( off ), state( isOn ),
                  canHaveStasis( true ), isInStasis( isInStasis ), stateInStasis( stateInStasis ) { }

        /// Constructor for a nonmutable property.
        Property( const QString &n, const QString &s )
                : name( n ), isMutable( false ), state( s ) { }
    };

    /** Return this type for PropertiesRole. */
    typedef QList<Property> PropertyList;
    
    
private:

    struct Private;
    Private * const m_d;
};


Q_DECLARE_METATYPE( KisNodeModel::PropertyList )

#endif
