/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KIS_BASE_NODE_H
#define _KIS_BASE_NODE_H

#include <QObject>
#include <QIcon>
#include <QUuid>
#include <QString>

#include <KoID.h>

#include <functional>

#include "kis_shared.h"
#include "kis_paint_device.h"
#include "kis_processing_visitor.h" // included, not forward declared for msvc

class KoProperties;
class KoColorSpace;
class KoCompositeOp;
class KisNodeVisitor;
class KisUndoAdapter;
class KisKeyframeChannel;

#include "kritaimage_export.h"

/**
 * A KisBaseNode is the base class for all components of an image:
 * nodes, layers masks, selections. A node has a number of properties,
 * can be represented as a thumbnail and knows what to do when it gets
 * a certain paint device to process. A KisBaseNode does not know
 * anything about its peers. You should not directly inherit from a
 * KisBaseNode; inherit from KisNode instead.
 */
class KRITAIMAGE_EXPORT KisBaseNode : public QObject, public KisShared
{

    Q_OBJECT

public:
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
        QString id;

        /** i18n-ed name, suitable for displaying */
        QString name;

        /** Whether the property is a boolean (e.g. locked, visible) which can be toggled directly from the widget itself. */
        bool isMutable {false};

        /** Provide these if the property isMutable. */
        QIcon onIcon;
        QIcon offIcon;

        /** If the property isMutable, provide a boolean. Otherwise, a string suitable for displaying. */
        QVariant state;

        /** If the property is mutable, specifies whether it can be put into stasis. When a property
        is in stasis, a new state is created, and the old one is stored in stateInStasis. When
        stasis ends, the old value is restored and the new one discarded */
        bool canHaveStasis {false};

        /** If the property isMutable and canHaveStasis, indicate whether it is in stasis or not */
        bool isInStasis {false};

        /** If the property isMutable and canHaveStasis, provide this value to store the property's
        state while in stasis */
        bool stateInStasis {false};

        bool operator==(const Property &rhs) const {
            return rhs.name == name && rhs.state == state && isInStasis == rhs.isInStasis;
        }

        Property(): isMutable( false ), isInStasis(false) { }

        /// Constructor for a mutable property.
        Property( const KoID &n, const QIcon &on, const QIcon &off, bool isOn )
                : id(n.id()), name( n.name() ), isMutable( true ), onIcon( on ), offIcon( off ), state( isOn ),
                  canHaveStasis( false ), isInStasis(false) { }

        /** Constructor for a mutable property accepting stasis */
        Property( const KoID &n, const QIcon &on, const QIcon &off, bool isOn,
                  bool _isInStasis, bool _stateInStasis = false )
                : id(n.id()), name(n.name()), isMutable( true ), onIcon( on ), offIcon( off ), state( isOn ),
                  canHaveStasis( true ), isInStasis( _isInStasis ), stateInStasis( _stateInStasis ) { }

        /// Constructor for a nonmutable property.
        Property( const KoID &n, const QString &s )
                : id(n.id()), name(n.name()), isMutable( false ), state( s ), isInStasis(false) { }
    };

    /** Return this type for PropertiesRole. */
    typedef QList<Property> PropertyList;

public:

    /**
     * Create a new, empty base node. The node is unnamed, unlocked
     * visible and unlinked.
     */
    KisBaseNode(KisImageWSP image);

    /**
     * Create a copy of this node.
     */
    KisBaseNode(const KisBaseNode & rhs);

    /**
     * Delete this node
     */
    ~KisBaseNode() override;


    /**
     * Return the paintdevice you can use to change pixels on. For a
     * paint layer these will be paint pixels, for an adjustment layer or a mask
     * the selection paint device.
     *
     * @return the paint device to paint on. Can be 0 if the actual
     *         node type does not support painting.
     */
    virtual KisPaintDeviceSP paintDevice() const = 0;

    /**
     * @return the rendered representation of a node
     * before the effect masks have had their go at it. Can be 0.
     */
    virtual KisPaintDeviceSP original() const = 0;

    /**
     * @return the fully rendered representation of this layer: its
     * rendered original and its effect masks. Can be 0.
     */
    virtual KisPaintDeviceSP projection() const = 0;

    /**
     * @return a special device from where the color sampler tool should sample
     * color when in layer-only mode. For most of the nodes just shortcuts
     * to projection() device. TODO: can it be null?
     */
    virtual KisPaintDeviceSP colorSampleSourceDevice() const;

    virtual const KoColorSpace *colorSpace() const = 0;

    /**
     * Return the opacity of this layer, scaled to a range between 0
     * and 255.
     * XXX: Allow true float opacity
     */
    quint8 opacity() const; //0-255

    /**
     * Set the opacity for this layer. The range is between 0 and 255.
     * The layer will be marked dirty.
     *
     * XXX: Allow true float opacity
     */
    void setOpacity(quint8 val); //0-255

    /**
     * return the 8-bit opacity of this layer scaled to the range
     * 0-100
     *
     * XXX: Allow true float opacity
     */
    quint8 percentOpacity() const; //0-100

    /**
     * Set the opacity of this layer with a number between 0 and 100;
     * the number will be scaled to between 0 and 255.
     * XXX: Allow true float opacity
     */
    void setPercentOpacity(quint8 val); //0-100

    /**
     * Return the composite op associated with this layer.
     */
    virtual const KoCompositeOp *compositeOp() const = 0;
    const QString& compositeOpId() const;

    /**
     * Set a new composite op for this layer. The layer will be marked
     * dirty.
     */
    void setCompositeOpId(const QString& compositeOpId);

    /**
     * @return unique id, which is now used by clone layers.
     */
    QUuid uuid() const;

    /**
     * Set the uuid of node. This should only be used when loading
     * existing node and in constructor.
     */
    void setUuid(const QUuid& id);

    /**
     * return the name of this node. This is the same as the
     * QObject::objectName.
     */
    QString name() const {
        return objectName();
    }

    /**
     * set the QObject::objectName. This is also the user-visible name
     * of the layer. The reason for this is that we want to see the
     * layer name also when debugging.
     */
    void setName(const QString& name) {
        setObjectName(name);
        baseNodeChangedCallback();
    }

    /**
     * @return the icon used to represent the node type, for instance
     * in the layerbox and in the menu.
     */
    virtual QIcon icon() const {
        return QIcon();
    }

    /**
     * Return a the properties of this base node (locked, visible etc,
     * with the right icons for their representation and their state.
     *
     * Subclasses can extend this list with new properties, like
     * opacity for layers or visualized for masks.
     *
     * The order of properties is, unfortunately, for now, important,
     * so take care which properties superclasses of your class
     * define.
     *
     * KisBaseNode defines visible = 0, locked = 1
     * KisLayer defines  opacity = 2, compositeOp = 3
     * KisMask defines active = 2 (KisMask does not inherit kislayer)
     */
    virtual PropertyList sectionModelProperties() const;

    /**
     * Change the section model properties.
     */
    virtual void setSectionModelProperties(const PropertyList &properties);

    /**
     * Return all the properties of this layer as a KoProperties-based
     * serializable key-value list.
     */
    const KoProperties & nodeProperties() const;

    /**
     * Set a node property.
     * @param name name of the property to be set.
     * @param value value to set the property to.
     */
    void setNodeProperty(const QString & name, const QVariant & value);

    /**
     * Merge the specified properties with the properties of this
     * layer. Wherever these properties overlap, the value of the
     * node properties is changed. No properties on the node are
     * deleted. If there are new properties in this list, they will be
     * added on the node.
     */
    void mergeNodeProperties(const KoProperties & properties);

    /**
     * Compare the given properties list with the properties of this
     * node.
     *
     * @return false only if the same property exists in both lists
     * but with a different value. Properties that are not in both
     * lists are disregarded.
     */
    bool check(const KoProperties & properties) const;

    /**
     * Accept the KisNodeVisitor (for the Visitor design pattern),
     * should call the correct function on the KisNodeVisitor for this
     * node type, so you need to override it for all leaf classes in
     * the node inheritance hierarchy.
     *
     * return false if the visitor could not successfully act on this
     * node instance.
     */
    virtual bool accept(KisNodeVisitor &) {
        return false;
    }

    /**
     * Accept the KisNodeVisitor (for the Visitor design pattern),
     * should call the correct function on the KisProcessingVisitor
     * for this node type, so you need to override it for all leaf
     * classes in the node inheritance hierarchy.
     *
     * The processing visitor differs from node visitor in the way
     * that it accepts undo adapter, that allows the processing to
     * be multithreaded
     */
    virtual void accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter) {
        Q_UNUSED(visitor);
        Q_UNUSED(undoAdapter);
    }

    /**
     * @return a thumbnail in requested size. The thumbnail is a rgba
     * QImage and may have transparent parts. Returns a fully
     * transparent QImage of the requested size if the current node
     * type cannot generate a thumbnail. If the requested size is too
     * big, return a null QImage.
     */
    virtual QImage createThumbnail(qint32 w, qint32 h, Qt::AspectRatioMode aspectRatioMode = Qt::IgnoreAspectRatio);

    /**
     * @return a thumbnail in requested size for the defined timestamp.
     * The thumbnail is a rgba Image and may have transparent parts.
     * Returns a fully transparent QImage of the requested size if the
     * current node type cannot generate a thumbnail. If the requested
     * size is too big, return a null QImage.
     */
    virtual QImage createThumbnailForFrame(qint32 w, qint32 h, int time, Qt::AspectRatioMode aspectRatioMode = Qt::IgnoreAspectRatio);

    /**
     * Ask this node to re-read the pertinent settings from the krita
     * configuration.
     */
    virtual void updateSettings() {
    }

    /**
     * @return true if this node is visible (i.e, active (except for
     * selection masks where visible and active properties are
     * different)) in the graph
     *
     * @param bool recursive if true, check whether all parents of
     * this node are visible as well.
     */
    virtual bool visible(bool recursive = false) const;

    /**
     * Set the visible status of this node. Visible nodes are active
     * in the graph (except for selections masks which can be active
     * while hidden), that is to say, they are taken into account
     * when merging. Invisible nodes play no role in the final image
     *, but will be modified when modifying all layers, for instance
     * when cropping.
     *
     * Toggling the visibility of a node will not automatically lead
     * to recomposition.
     *
     * @param visible the new visibility state
     * @param isLoading if true, the property is set during loading.
     */
    virtual void setVisible(bool visible, bool loading = false);

    /**
     * Return the locked status of this node. Locked nodes cannot be
     * edited.
     */
    bool userLocked() const;

    /**
     * Return whether or not the given node is isolated.
     */
    bool belongsToIsolatedGroup() const;

    /**
     * Return whether or not the given node is the root of
     * isolation.
     */
    bool isIsolatedRoot() const;

    /**
     * Set the locked status of this node. Locked nodes cannot be
     * edited.
     */
    virtual void setUserLocked(bool l);

    /**
     * @return true if the node can be edited:
     *
     * if checkVisibility is true, then the node is only editable if it is visible and not locked.
     * if checkVisibility is false, then the node is editable if it's not locked.
     */
    bool isEditable(bool checkVisibility = true) const;

    /**
     * @return true if the node is editable and has a paintDevice()
     *         which which can be used for accessing pixels. It is an
     *         equivalent to (isEditable() && paintDevice())
     */
    bool hasEditablePaintDevice() const;

    /**
     * @return the x-offset of this layer in the image plane.
     */
    virtual qint32 x() const {
        return 0;
    }

    /**
     * Set the x offset of this layer in the image place.
     * Re-implement this where it makes sense, by default it does
     * nothing. It should not move child nodes.
     */
    virtual void setX(qint32) {
    }

    /**
     * @return the y-offset of this layer in the image plane.
     */
    virtual qint32 y() const {
        return 0;
    }

    /**
     * Set the y offset of this layer in the image place.
     * Re-implement this where it makes sense, by default it does
     * nothing. It should not move child nodes.
     */
    virtual void setY(qint32) {
    }

    /**
    * Returns an approximation of where the bounds on actual data are
    * in this node.
    */
    virtual QRect extent() const {
        return QRect();
    }

    /**
     * Returns the exact bounds of where the actual data resides in
     * this node.
     */
    virtual QRect exactBounds() const {
        return QRect();
    }

    /**
     * Sets the state of the node to the value of @param collapsed
     */
    void setCollapsed(bool collapsed);

    /**
     * returns the collapsed state of this node
     */
    bool collapsed() const;

    /**
     * Sets a color label index associated to the layer.  The actual
     * color of the label and the number of available colors is
     * defined by Krita GUI configuration.
     */
    void setColorLabelIndex(int index);

    /**
     * \see setColorLabelIndex
     */
    int colorLabelIndex() const;

    /**
     * Returns true if the offset of the node can be changed in a LodN
     * stroke. Currently, all the nodes except shape layers support that.
     */
    bool supportsLodMoves() const;

    /**
     * Returns true if the node can be painted via KisPaintDevice. Notable
     * exceptions are selection-based layers and masks.
     */
    virtual bool supportsLodPainting() const;

    /**
     * Return the keyframe channels associated with this node
     * @return list of keyframe channels
     */
    QMap<QString, KisKeyframeChannel*> keyframeChannels() const;

    /**
     * Get the keyframe channel with given id.
     * If the channel does not yet exist and the node supports the requested
     * channel, it will be created if create is true.
     * @param id internal name for channel
     * @param create attempt to create the channel if it does not exist yet
     * @return keyframe channel with the id, or null if not found
     */
    KisKeyframeChannel *getKeyframeChannel(const QString &id, bool create);
    KisKeyframeChannel *getKeyframeChannel(const QString &id) const;

    /**
     * @return If true, node will be visible on animation timeline even when inactive.
     */
    bool isPinnedToTimeline() const;

    /**
     * Set whether node should be visible on animation timeline even when inactive.
     */
    void setPinnedToTimeline(bool pinned);

    bool isAnimated() const;
    void enableAnimation();

    virtual void setImage(KisImageWSP image);
    KisImageWSP image() const;


    /**
     * Fake node is not present in the layer stack and is not used
     * for normal projection rendering algorithms.
     */
    virtual bool isFakeNode() const;

protected:

    void setSupportsLodMoves(bool value);

    /**
     * FIXME: This method is a workaround for getting parent node
     * on a level of KisBaseNode. In fact, KisBaseNode should inherit
     * KisNode (in terms of current Krita) to be able to traverse
     * the node stack
     */
    virtual KisBaseNodeSP parentCallback() const {
        return KisBaseNodeSP();
    }

    virtual void notifyParentVisibilityChanged(bool value) {
        Q_UNUSED(value);
    }

    /**
     * This callback is called when some meta state of the base node
     * that can be interesting to the UI has changed. E.g. visibility,
     * lockness, opacity, compositeOp and etc. This signal is
     * forwarded by the KisNode and KisNodeGraphListener to the model
     * in KisLayerBox, so it can update its controls when information
     * changes.
     */
    virtual void baseNodeChangedCallback() {
    }

    /**
     * This callback is called when collapsed state of the base node
     * has changed. This signal is forwarded by the KisNode and
     * KisNodeGraphListener to the model in KisLayerBox, so it can
     * update its controls when information changes.
     */
    virtual void baseNodeCollapsedChangedCallback() {
    }


    virtual void baseNodeInvalidateAllFramesCallback() {
    }

    /**
     * Add a keyframe channel for this node. The channel will be added
     * to the common hash table which will be available to the UI.
     *
     * WARNING: the \p channel object *NOT* become owned by the node!
     *          The caller must ensure manually that the lifetime of
     *          the object coincide with the lifetime of the node.
     */
    virtual void addKeyframeChannel(KisKeyframeChannel* channel);

    /**
     * Attempt to create the requested channel. Used internally by getKeyframeChannel.
     * Subclasses should implement this method to catch any new channel types they support.
     * @param id channel to create
     * @return newly created channel or null
     */
    virtual KisKeyframeChannel * requestKeyframeChannel(const QString &id);

public:
    /**
     * Ideally, this function would be used to query for keyframe support
     * before trying to create channels. The ability to query would help
     * in cases such as animation curves where you might want to ask
     * which channels it supports before allowing the user to add.
     *
     * @param id querried channel
     * @return bool whether it supports said channel or not.
     */
    virtual bool supportsKeyframeChannel(const QString &id);

Q_SIGNALS:
    void keyframeChannelAdded(KisKeyframeChannel *channel);
    void opacityChanged(quint8 value);

private:

    struct Private;
    Private * const m_d;

};


Q_DECLARE_METATYPE( KisBaseNode::PropertyList )



#endif
