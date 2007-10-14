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

#include "kis_shape_controller.h"

#include <klocale.h>

#include <KoShape.h>
#include <KoShapeContainer.h>
#include <KoShapeManager.h>
#include <KoCanvasBase.h>
#include <KoToolManager.h>
#include <KoView.h>
#include <KoSelection.h>
#include <KoShapeLayer.h>
#include <KoPathShape.h>

#include "kis_adjustment_layer.h"
#include "kis_clone_layer.h"
#include "kis_canvas2.h"
#include "kis_doc2.h"
#include "kis_image.h"
#include "kis_group_layer.h"
#include "kis_layer_container_shape.h"
#include "kis_layermap_visitor.h"
#include "kis_layer_shape.h"
#include "kis_mask_shape.h"
#include "kis_nameserver.h"
#include "kis_mask.h"
#include "kis_shape_layer.h"
#include "kis_view2.h"
#include "kis_node.h"

typedef QMap<KisNodeSP, KoShape*> KisNodeMap;

class KisShapeController::Private
{
public:
    KisImageSP image;
    KisNodeMap nodeShapes; // maps from krita/image layers to shapes
    KisDoc2 * doc;
    KisNameServer * nameServer;
};

KisShapeController::KisShapeController( KisDoc2 * doc, KisNameServer *nameServer )
    : QObject( doc )
    , m_d( new Private )
{
    m_d->doc = doc;
    m_d->nameServer = nameServer;
    m_d->image = 0;
}


KisShapeController::~KisShapeController()
{
    kDebug(41007) <<"Deleting the KisShapeController. There are" << m_d->nodeShapes.size() <<" shapes";
/*
    foreach( KoShape* shape, m_d->nodeShapes ) {
        removeShape( shape);
        delete shape; // XXX: What happes with stuff on the
                      // clipboard? And how about undo information?
    }
    m_d->nodeShapes.clear();
*/
    delete m_d;
}

void KisShapeController::setImage( KisImageSP image )
{
    if ( m_d->image ) {
        m_d->image->disconnect( this );
        // First clear the current set of shapes away
        foreach( KoShape* shape, m_d->nodeShapes ) {
            removeShape( shape );
            delete shape; // XXX: What happes with stuff on the
            // clipboard? And how about undo information?

        }
        m_d->nodeShapes.clear();

    }

    if ( image ) {
        m_d->image = image;

        KisLayerMapVisitor v( m_d->nodeShapes );
        m_d->image->rootLayer()->accept( v );
        m_d->nodeShapes = v.layerMap();

        foreach( KoView *view, m_d->doc->views() ) {
            KisCanvas2 *canvas = ((KisView2*)view)->canvasBase();
            foreach( KoShape* shape, m_d->nodeShapes ) {
                canvas->shapeManager()->add(shape);
            }
            canvas->canvasWidget()->update();
        }

        connect( m_d->image, SIGNAL( sigNodeHasBeenAdded( KisNode *, int ) ), SLOT( slotNodeAdded( KisNode*, int ) ) );
        connect( m_d->image, SIGNAL( sigNodeHasBeenRemoved( KisNode *, int ) ), SLOT( slotNodeRemoved( KisNode*, int) ) );
        connect( m_d->image, SIGNAL( sigLayersChanged( KisGroupLayerSP ) ), this, SLOT( slotLayersChanged( KisGroupLayerSP ) ) );
    }
}

void KisShapeController::removeShape( KoShape* shape )
{
    if ( !shape ) return;

    KoShapeContainer * container = shape->parent();
    kDebug(41007) <<"parent is" << container;

    if ( container ) {
        container->removeChild( shape );

        // If there are no longer children, remove the container
        // layer, too.
        if ( container->childCount() == 0 ) {
            removeShape( container );
        }
    }

    if ( shape->shapeId() == KIS_LAYER_SHAPE_ID
         || shape->shapeId() == KIS_SHAPE_LAYER_ID
         || shape->shapeId() == KIS_LAYER_CONTAINER_ID
         || shape->shapeId() == KIS_MASK_SHAPE_ID
         || shape->shapeId() == KoPathShapeId)  // selection shapes
    {
        foreach( KoView *view, m_d->doc->views() ) {
            KisCanvas2 *canvas = ((KisView2*)view)->canvasBase();
            canvas->shapeManager()->remove(shape);
            canvas->canvasWidget()->update();
        }
    }

    m_d->doc->setModified( true );
}

void KisShapeController::addShape( KoShape* shape )
{
    if ( !m_d->image ) return;
    // Only non-krita shapes get added through this method; krita
    // layer shapes are added to kisimage and then end up in
    // slotLayerAdded
    if ( shape->shapeId() != KIS_LAYER_SHAPE_ID  &&
         shape->shapeId() != KIS_SHAPE_LAYER_ID  &&
         shape->shapeId() != KIS_LAYER_CONTAINER_ID &&
         shape->shapeId() != KIS_MASK_SHAPE_ID ) {

        // An ordinary shape, if the active layer is a KisShapeLayer,
        // add it there, otherwise, create a new KisShapeLayer on top
        // of the active layer.

        // Check whether the shape is part of a layer -- that would be our
        // shape layers. The parent is set KoShapeController using the
        // KoSelection object returned by the KoShapeManager that is
        // returned by KoCanvasBase -- and the shape manager in the
        // KisShapeLayer always sets the layer as parent using
        // KoSelection::setActiveLayer. The inheritance is:
        // KisShapeLayer::KoShapeLayer::KoShapeContainer::KoShape.

        KisShapeLayer * shapeLayer = dynamic_cast<KisShapeLayer*>( shape->parent() );
        kDebug(41007) <<"shape:" << shape;
        kDebug(41007) <<"shape parent:" << shape->parent();
        kDebug(41007) <<"shape layer:" << shapeLayer;

//TODO this doesn't work with shape selections
#if 0
        if ( !shapeLayer ) {
            // There is no parent layer set, which means that when
            // dropping, there was no shape layer active. Create one
            // and add it on top of the image stack.

            KisLayerContainerShape * container = dynamic_cast<KisLayerContainerShape*>( m_d->image->rootLayer().data() );
            kDebug(41007) <<"container:" << container;
            shapeLayer = new KisShapeLayer(container,
                                           m_d->image,
                                           i18n( "Flake shapes %1", m_d->nameServer->number() ),
                                           OPACITY_OPAQUE);

            // Add the shape layer to the image. The image then emits
            // a signal that is caught by us (the document) and the
            // layerbox and makes sure the new layer is in the
            // layer-shape map and in the layerbox

            m_d->image->addLayer( shapeLayer, m_d->image->rootLayer());
        }
#endif
        // XXX: What happens if the shape is added embedded in another
        // shape?
        if ( shapeLayer )
            shapeLayer->addChild( shape );

        foreach( KoView *view, m_d->doc->views() ) {
            KisCanvas2 *canvas = static_cast<KisView2*>(view)->canvasBase();
            canvas->globalShapeManager()->add(shape);
        }
    }
    else {
        kWarning() <<"Eeek -- we tried to add a krita layer shape without going through KisImage";
    }

    m_d->doc->setModified( true );
}


void KisShapeController::setInitialShapeForView( KisView2 * view )
{
    if ( !m_d->image ) return;

    if(! m_d->nodeShapes.isEmpty()) {
        Q_ASSERT(view->canvasBase());
        Q_ASSERT(view->canvasBase()->shapeManager());
        KoSelection *selection = view->canvasBase()->shapeManager()->selection();
        if ( selection ) {
            selection->select(m_d->nodeShapes.values().first());
            KoToolManager::instance()->switchToolRequested(KoToolManager::instance()->preferredToolForSelection(selection->selectedShapes()));
        }
    }
}

void KisShapeController::slotNodeAdded( KisNode* parentNode, int index )
{
    kDebug(41007) << "parentnode: " << parentNode << ", index: " << index;

    KisNodeSP node = parentNode->at( index );

    // Check whether the layer is already in the map
    if ( m_d->nodeShapes.contains( node ) ) {
        kDebug(41007) <<"The document already contains node" << node->name();
        return;
    }

    // Get the parent -- there is always one, and it should be in the nodemap already
    KoShapeContainer * parent = dynamic_cast<KoShapeContainer*>( shapeForNode( node->parent().data() ) );
    Q_ASSERT( parent );

    KoShape * shape = 0;

    kDebug(41007) << "Going to add node of type " << node->metaObject()->className();
    if ( node->inherits("KisGroupLayer" ) ) {
        shape = new KisLayerContainerShape(parent, static_cast<KisGroupLayer*>(node.data()));
    }
    else if ( node->inherits("KisPaintLayer")  ||
              node->inherits("KisAdjustmentLayer") ||
              node->inherits("KisCloneLayer") ) {
        shape = new KisLayerShape( parent, static_cast<KisLayer*>( node.data() ) );
    }
    else if ( node->inherits("KisShapeLayer") ) {
        shape = static_cast<KisShapeLayer*>( node.data() );
    }
    else if ( node->inherits("KisMask") ) {
        shape = new KisMaskShape( parent, static_cast<KisMask*>( node.data() ) );
    }

    Q_ASSERT( shape );

    // Put the layer in the right place in the hierarchy
    shape->setParent( parent );
    parent->addChild( shape );

    m_d->nodeShapes[node] = shape;

    foreach( KoView *view, m_d->doc->views() ) {
        KisCanvas2 *canvas = ((KisView2*)view)->canvasBase();
        canvas->shapeManager()->add(shape);
        canvas->canvasWidget()->update();
    }

}

void KisShapeController::slotNodeRemoved( KisNode* parent, int index )
{
    KisNodeSP node = parent->at( index );

    removeShape( shapeForNode( node ) );
}

void KisShapeController::slotLayersChanged( KisGroupLayerSP rootLayer )
{
    Q_UNUSED( rootLayer );

    setImage( m_d->image );
}

KoShape * KisShapeController::shapeForNode( KisNodeSP node )
{

    if ( m_d->nodeShapes.contains( node) )
        return m_d->nodeShapes[node];
    else {
        kDebug(41007) <<"KisShapeController::shapeForNode does not find a shape for node " << node << ", this should never happen!";
        return 0;
    }
}


int KisShapeController::layerMapSize()
{
    return m_d->nodeShapes.size();
}


#include "kis_shape_controller.moc"
