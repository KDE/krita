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
#include "kis_shape_layer.h"
#include "kis_view2.h"

typedef QMap<KisLayerSP, KoShape*> KisLayerMap;


class KisShapeController::Private
{
public:
    KisImageSP image;
    KisLayerMap layerShapes; // maps from krita/image layers to shapes
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
    kDebug(41007) << "Deleting the KisShapeController. There are " << m_d->layerShapes.size() << " shapes" << endl;
/*
    foreach( KoShape* shape, m_d->layerShapes ) {
        removeShape( shape);
        delete shape; // XXX: What happes with stuff on the
                      // clipboard? And how about undo information?
    }
    m_d->layerShapes.clear();
*/
    delete m_d;
}

void KisShapeController::setImage( KisImageSP image )
{
    if ( m_d->image ) {
        m_d->image->disconnect( this );
        // First clear the current set of shapes away
        foreach( KoShape* shape, m_d->layerShapes ) {
            removeShape( shape );
            delete shape; // XXX: What happes with stuff on the
            // clipboard? And how about undo information?

        }
        m_d->layerShapes.clear();
    }

    m_d->image = image;

    KisLayerMapVisitor v( m_d->layerShapes );
    m_d->image->rootLayer()->accept( v );
    m_d->layerShapes = v.layerMap();

    foreach( KoView *view, m_d->doc->views() ) {
        KisCanvas2 *canvas = ((KisView2*)view)->canvasBase();
        foreach( KoShape* shape, m_d->layerShapes ) {
            canvas->shapeManager()->add(shape);
        }
        canvas->canvasWidget()->update();
    }

    connect( m_d->image.data(), SIGNAL(sigLayerAdded( KisLayerSP )), this, SLOT(slotLayerAdded( KisLayerSP )) );
    connect( m_d->image.data(), SIGNAL(sigLayerRemoved( KisLayerSP, KisGroupLayerSP, KisLayerSP )), this, SLOT(slotLayerRemoved( KisLayerSP, KisGroupLayerSP, KisLayerSP) ));
    connect( m_d->image.data(), SIGNAL(sigLayerMoved( KisLayerSP, KisGroupLayerSP, KisLayerSP )), this, SLOT(slotLayerMoved( KisLayerSP, KisGroupLayerSP, KisLayerSP )) );
    connect( m_d->image.data(), SIGNAL(sigLayersChanged( KisGroupLayerSP )), this, SLOT(slotLayersChanged( KisGroupLayerSP )) );

}

void KisShapeController::removeShape( KoShape* shape )
{
    if ( !shape ) return;

    KoShapeContainer * container = shape->parent();
    kDebug(41007) << "parent is " << container << endl;

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
         || shape->shapeId() == KIS_MASK_SHAPE_ID )
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

        if ( !shapeLayer ) {
            // There is no parent layer set, which means that when
            // dropping, there was no shape layer active. Create one
            // and add it on top of the image stack.

            KisLayerContainerShape * container = dynamic_cast<KisLayerContainerShape*>( m_d->image->rootLayer().data() );
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
        shapeLayer->addChild( shape );
    }
    else {
        kDebug() << "Eeek -- we tried to add a krita layer shape without going through KisImage" << endl;
    }

    m_d->doc->setModified( true );
}


void KisShapeController::setInitialShapeForView( KisView2 * view )
{
    if(! m_d->layerShapes.isEmpty()) {
        Q_ASSERT(view->canvasBase());
        Q_ASSERT(view->canvasBase()->shapeManager());
        KoSelection *selection = view->canvasBase()->shapeManager()->selection();
        if ( selection ) {
            selection->select(m_d->layerShapes.values().first());
            KoToolManager::instance()->switchToolRequested(KoToolManager::instance()->preferredToolForSelection(selection->selectedShapes()));
        }
    }
}

void KisShapeController::slotLayerAdded( KisLayerSP layer )
{
    // Check whether the layer is already in the map
    if ( m_d->layerShapes.contains( layer ) ) {
        kDebug(41007) << "The document already contains layer " << layer->name() << endl;
        return;
    }

    // Get the parent -- there is always one, and it should be in the layermap already
    KoShapeContainer * parent = dynamic_cast<KoShapeContainer*>( shapeForLayer( static_cast<KisLayer*>( layer->parentLayer().data() ) ) );
    Q_ASSERT( parent );

    KoShape * shape = 0;

    // Create a shape around the layer (Bjarne frowns upon promiscuous
    // dynamic casts, though, and I dare say he's right. I still like
    // it better than adding layerId's everywhere.
    kDebug(41007) << "Is grouplayer?: " << dynamic_cast<KisGroupLayer*>( layer.data() ) << endl;
    kDebug(41007) << "Is adjustmentlayer?: " << dynamic_cast<KisAdjustmentLayer*>( layer.data() ) << endl;
    kDebug(41007) << "Is paintlayer?: " << dynamic_cast<KisPaintLayer*>( layer.data() ) << endl;
    kDebug(41007) << "Is shapelayer?: " << dynamic_cast<KisShapeLayer*>( layer.data() ) << endl;

    if ( dynamic_cast<KisGroupLayer*>( layer.data() ) ) {
        shape = new KisLayerContainerShape(parent, layer);
    }
    else if ( dynamic_cast<KisPaintLayer*>( layer.data() )  ||
              dynamic_cast<KisAdjustmentLayer*>( layer.data() ) ||
              dynamic_cast<KisCloneLayer*>( layer.data() ) ) {
        shape = new KisLayerShape( parent, layer );
    }
    else if ( dynamic_cast<KisShapeLayer*>( layer.data() ) ) {
        shape = dynamic_cast<KisShapeLayer*>( layer.data() );
    }

    Q_ASSERT( shape );

    // Put the layer in the right place in the hierarchy
    shape->setParent( parent );
    parent->addChild( shape );

    m_d->layerShapes[layer] = shape;

    foreach( KoView *view, m_d->doc->views() ) {
        KisCanvas2 *canvas = ((KisView2*)view)->canvasBase();
        canvas->shapeManager()->add(shape);
        canvas->canvasWidget()->update();
    }

}

void KisShapeController::slotLayerRemoved( KisLayerSP layer,  KisGroupLayerSP wasParent,  KisLayerSP wasAboveThis )
{
    Q_UNUSED( wasParent );
    Q_UNUSED( wasAboveThis );

    removeShape( shapeForLayer( layer ) );
}

void KisShapeController::slotLayerMoved( KisLayerSP layer,  KisGroupLayerSP previousParent, KisLayerSP wasAboveThis )
{
    Q_UNUSED( previousParent );
    Q_UNUSED( wasAboveThis );

    KoShape * shape = shapeForLayer( layer );
    removeShape( shape );
    slotLayerAdded( layer );
}

void KisShapeController::slotLayersChanged( KisGroupLayerSP rootLayer )
{
    Q_UNUSED( rootLayer );

    setImage( m_d->image );
}

KoShape * KisShapeController::shapeForLayer( KisLayerSP layer )
{

    if ( m_d->layerShapes.contains( layer ) )
        return m_d->layerShapes[layer];
    else {
        kDebug() << "KisShapeController::shapeForLayer does not find a shape for layer " << layer << ", this should never happen!" << endl;
        return 0;
    }
}

#include "kis_shape_controller.moc"
