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

#include <QIcon>
#include <kicon.h>

#include "kis_types.h"
#include "kis_shared.h"
#include "krita_export.h"
#include "KoDocumentSectionModel.h"

class KisLayerVisitor;

/**
 * A KisBaseNode is the base class for all components of an image:
 * nodes, layers masks, selections. A node has a number of properties,
 * can be represented as a thumbnail and knows what to do when it gets
 * a certain paint device to process. A KisBaseNode does not know
 * anything about its peers. You should not directly inherit from a
 * KisBaseNode; inherit from KisNode instead.
 */
class KRITAIMAGE_EXPORT KisBaseNode : public QObject, public KisShared {

    Q_OBJECT

public:

    enum { Visible = 1, Hidden = 2, Locked = 4, Unlocked = 8, Linked = 16, Unlinked = 32 };

    /**
     * Create a new, empty base node. The node is unnamed, unlocked
     * visible and unlinked.
     */
    KisBaseNode();

    /**
     * Create a copy of this node.
     */
    KisBaseNode( const KisBaseNode & rhs );

    /**
     * Delete this node
     */
    virtual ~KisBaseNode();

    /**
     * return the name of this node. This is the same as the
     * QObject::objectName.
     */
    QString name() const
        {
            return objectName();
        }

    /**
     * set the QObject::objectName. This is also the user-visible name
     * of the layer. The reason for this is that we want to see the
     * layer name also when debugging.
     */
    void setName(const QString& name)
        {
            setObjectName( name );
        }

    /**
     * @return the icon used to represent the node type, for instance
     * in the layerbox and in the menu.
     */
    virtual QIcon icon() const
        {
            return QIcon();
        };

    /**
     * Return a the properties of this base node (locked, visible etc,
     * with the right icons for their representation and their state.
     *
     * Subclasses can extent this list with new properties, like
     * opacity for layers or visualized for masks.
     */
    virtual KoDocumentSectionModel::PropertyList properties() const;

    virtual void setProperties( const KoDocumentSectionModel::PropertyList &properties  );



    /**
     * Accept the KisLayerVisitor (for the Visitor design pattern),
     * should call the correct function on the KisLayerVisitor for
     * this node type.
     *
     * return false if the visitor could not succesfully act on this
     * node instance.
     */
    virtual bool accept(KisLayerVisitor &)
        {
            return false;
        }

    /**
     * @return a thumbnail in requested size. The thumbnail is a rgba
     * QImage and may have transparent parts. Returns a fully
     * transparent QImage of the requested size if the current node
     * type cannot generate a thumbnail. If the requested size is too
     * big, return a null QImage.
     */
    virtual QImage createThumbnail(qint32 w, qint32 h );

    /**
     * Ask this node to re-read the pertinent settings from the krita
     * configuration.
     */
    virtual void updateSettings()
        {
        }

    /**
     * @return true if this node is visible (i.e, active) in the graph
     */
    const bool visible() const;

    /**
     * Set the visible status of this node. Visible nodes are active
     * in the graph, that is to say, they are taken into account when
     * merging. Invisible nodes play no role in the final image, but
     * will be modified when modifying all layers, for instance when
     * cropping.
     *
     * Toggling the visibility of a node will not automatically lead
     * to recomposition.
     */
    void setVisible(bool v);

    /**
     * Return the locked status of this node. Locked nodes cannot be
     * edited.
     */
    bool locked() const;

    /**
     * Set the locked status of this node. Locked nodes cannot be
     * edited.
     */
    void setLocked(bool l);

    /**
     * @return the x-offset of this layer in the image plane.
     */
    virtual qint32 x() const
        {
            return 0;
        }

    /**
     * Set the x offset of this layer in the image place.
     * Re-implement this where it makes sense, by default it does
     * nothing.
     */
    virtual void setX(qint32)
        {
        }

    /**
     * @return the y-offset of this layer in the image plane.
     */
    virtual qint32 y() const
        {
            return 0;
        }

    /**
     * Set the y offset of this layer in the image place.
     * Re-implement this where it makes sense, by default it does
     * nothing.
     */
    virtual void setY(qint32)
        {
        }

private:

    class Private;
    Private * const m_d;

};


#endif
