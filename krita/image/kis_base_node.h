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
#ifndef _KIS_BASE_NODE_H
#define _KIS_BASE_NODE_H

#include <QObject>
#include <QIcon>
#include <QUuid>
#include <QString>

#include "KoDocumentSectionModel.h"

#include "kis_shared.h"
#include "kis_paint_device.h"
#include "kis_processing_visitor.h" // included, not forward declared for msvc

class KoProperties;
class KoColorSpace;
class KoCompositeOp;
class KisNodeVisitor;
class KisUndoAdapter;

#include "krita_export.h"

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
     * Create a new, empty base node. The node is unnamed, unlocked
     * visible and unlinked.
     */
    KisBaseNode();

    /**
     * Create a copy of this node.
     */
    KisBaseNode(const KisBaseNode & rhs);

    /**
     * Delete this node
     */
    virtual ~KisBaseNode();


    /**
     * Return the paintdevice you can use to change pixels on. For a
     * paint layer these will be paint pixels, for an adjustment layer or a mask
     * the selection paint device.
     *
     * @return the paint device to paint on. Can be 0 if the actual
     *         node type does not support painting.
     */
    virtual KisPaintDeviceSP paintDevice() const;

    /**
     * @return the rendered representation of a node
     * before the effect masks have had their go at it. Can be 0.
     */
    virtual KisPaintDeviceSP original() const;

    /**
     * @return the fully rendered representation of this layer: its
     * rendered original and its effect masks. Can be 0.
     */
    virtual KisPaintDeviceSP projection() const;

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
    void setCompositeOp(const QString& compositeOpId);

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
    };

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
    virtual KoDocumentSectionModel::PropertyList sectionModelProperties() const;

    /**
     * Change the section model properties.
     */
    virtual void setSectionModelProperties(const KoDocumentSectionModel::PropertyList &properties);

    /**
     * Return all the properties of this layer as a KoProperties-based
     * serializable key-value list.
     */
    KoProperties & nodeProperties() const;

    /**
     * Merge the specified properties with the properties of this
     * layer. Whereever these properties overlap, the value of the
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
    virtual QImage createThumbnail(qint32 w, qint32 h);

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
     */
    virtual void setVisible(bool v);

    /**
     * Return the locked status of this node. Locked nodes cannot be
     * edited.
     */
    bool userLocked() const;

    /**
     * Set the locked status of this node. Locked nodes cannot be
     * edited.
     */
    void setUserLocked(bool l);

    /**
     * Return the locked status of this node. System Locked nodes indicates
     * that an algorithm is processing them and that an other
     * algorithm need to wait before accessing it.
     */
    bool systemLocked() const;

    /**
     * Set the locked status of this node. System Locked nodes indicates
     * that an algorithm is processing them and that an other
     * algorithm need to wait before accessing it.
     *
     * A KisNode will update the layer model when the lock is released.
     *
     * @param l lock state
     * @param update set false if the tools shouldn't be locked
     */
    virtual void setSystemLocked(bool l, bool update = true);

    /**
     * @return true if the node can be edited: if it's visible and neither locked
     *         by the user nor by the system.
     *         It's equivalent to ( visible() and not userLocked() and not systemLocked() ).
     */
    bool isEditable() const;

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

protected:

    /**
     * FIXME: This method is a workaround for getting parent node
     * on a level of KisBaseNode. In fact, KisBaseNode should inherit
     * KisNode (in terms of current Krita) to be able to traverse
     * the node stack
     */
    virtual KisBaseNodeSP parentCallback() const {
        return 0;
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

signals:

    /**
     * This signal is emitted when the visibility of the layer is changed with \ref setVisible.
     */
    void visibilityChanged(bool);

    /**
     * This signal is emitted when the node is locked or unlocked with \ref setUserLocked.
     */
    void userLockingChanged(bool);
    /**
     * This signal is emitted when the node is locked or unlocked with \ref setSystemLocked.
     */
    void systemLockingChanged(bool);
private:

    struct Private;
    Private * const m_d;

};


#endif
