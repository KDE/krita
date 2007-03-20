/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
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

#include "kis_nameserver.h"
#include "kis_canvas2.h"
#include "kis_layer_container_shape.h"
#include "kis_layer_shape.h"
#include "kis_shape_layer.h"
#include "kis_mask_shape.h"
#include "kis_layermap_visitor.h"
#include "kis_view2.h"
#include "kis_image.h"
#include "kis_doc2.h"

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
    foreach( KoShape* shape, m_d->layerShapes ) {
        removeShape( shape);
        delete shape; // XXX: What happes with stuff on the
                      // clipboard? And how about undo information?
    }
    m_d->layerShapes.clear();

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

    connect( m_d->image.data(), SIGNAL(sigLayerActivated(KisLayerSP) ), this, SLOT( slotLayerActivated( KisLayerSP ) ) );
    connect( m_d->image.data(), SIGNAL(sigLayerAdded( KisLayerSP )), this, SLOT(slotLayerAdded( KisLayerSP )) );
    connect( m_d->image.data(), SIGNAL(sigLayerRemoved( KisLayerSP, KisGroupLayerSP, KisLayerSP )), this, SLOT(slotLayerRemoved( KisLayerSP, KisGroupLayerSP, KisLayerSP) ));
    connect( m_d->image.data(), SIGNAL(sigLayerMoved( KisLayerSP, KisGroupLayerSP, KisLayerSP )), this, SLOT(slotLayerMoved( KisLayerSP, KisGroupLayerSP, KisLayerSP )) );
    connect( m_d->image.data(), SIGNAL(sigLayersChanged( KisGroupLayerSP )), this, SLOT(slotLayersChanged( KisGroupLayerSP )) );
    m_d->image->activateLayer( m_d->image->rootLayer()->firstChild() );
}

void KisShapeController::removeShape( KoShape* shape )
{
    if ( !shape ) return;

    kDebug(41007) << ">>>>> KisDoc2::removeShape: " << shape->shapeId()  << ", active layer: " << activeLayerShape() << endl;

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

    foreach( KoView *view, m_d->doc->views() ) {
        KisCanvas2 *canvas = ((KisView2*)view)->canvasBase();
        canvas->shapeManager()->remove(shape);
        canvas->canvasWidget()->update();
    }

    m_d->doc->setModified( true );
}





void KisShapeController::addShape( KoShape* shape )
{
    kDebug(41007) << ">>>>> KisShapeController::addShape: " << shape->shapeId()  << ", active layer: " << activeLayerShape() << endl;
    if ( activeLayerShape() ) kDebug(41007) << "active layer shape id: " << activeLayerShape()->shapeId() << endl;

    if (   shape->shapeId() != KIS_LAYER_SHAPE_ID
           && shape->shapeId() != KIS_SHAPE_LAYER_ID
           && shape->shapeId() != KIS_LAYER_CONTAINER_ID
           && shape->shapeId() != KIS_MASK_SHAPE_ID )
    {
        // An ordinary shape, if the active layer is a KisShapeLayer,
        // add it there, otherwise, create a new KisShapeLayer on top
        // of the active layer.
        KoShape * activeShape = activeLayerShape();

        kDebug(41007) << "Active shape = " << ( activeShape ? activeShape->shapeId() : "0" )  << endl;

        KisShapeLayer * shapeLayer = dynamic_cast<KisShapeLayer*>( activeShape );
        if ( !shapeLayer ) {

            kDebug(41007) << "creating a new shape layer for shape " << shape << endl;
            KoShape * currentLayer = activeLayerShape();

            kDebug(41007) << "Active layer shape: " << currentLayer << endl;
            KisLayerContainerShape * container = 0;

            while ( container == 0 && currentLayer != 0 ) {
                kDebug(41007) << currentLayer->shapeId() << endl;
                container = dynamic_cast<KisLayerContainerShape *>( currentLayer );
                if ( currentLayer->parent() )
                    currentLayer = currentLayer->parent();
                else
                    break;
            }

            shapeLayer = new KisShapeLayer(container,
                                           m_d->image,
                                           i18n( "Flake shapes %1", m_d->nameServer->number() ),
                                           OPACITY_OPAQUE);

            // Add the shape layer to the image. The image then emits
            // a signal that is caught by us (the document) and the
            // layerbox and makes sure the new layer is in the
            // layer-shape map and in the layerbox

            // XXX: This casting around of layers is a blight and
            // a blot on the landscape. Especially when I have to
            // wriggle the pointer out of the shared pointer.

            if ( container )
                m_d->image->addLayer( shapeLayer,
                                        qobject_cast<KisGroupLayer*>( container->groupLayer().data() ) );
            else
                m_d->image->addLayer( shapeLayer, 0 );

        }
        shapeLayer->addChild( shape );
    }

    m_d->doc->setModified( true );
}


void KisShapeController::setInitialShapeForView( KisView2 * view )
{
    if(! m_d->layerShapes.isEmpty()) {
        Q_ASSERT(view->canvasBase());
        KoSelection *selection = view->canvasBase()->shapeManager()->selection();
        selection->select(m_d->layerShapes.values().first());
        KoToolManager::instance()->switchToolRequested(KoToolManager::instance()->preferredToolForSelection(selection->selectedShapes()));
    }

}

void KisShapeController::slotLayerAdded( KisLayerSP layer )
{
    kDebug(41007) << ">>>>> KisShapeController::slotLayerAdded "
                  << ( layer ? layer->name() : "empty layer" ) << ", active layer type: "
                  << activeLayerShape()  << endl;

    // Check whether the layer is already in the map
    if ( m_d->layerShapes.contains( layer ) ) {
        kDebug(41007) << "The document already contains layer " << layer->name() << endl;
        return;
    }

    // Get the parent -- there is always one
    KoShapeContainer * parent = 0;
    if ( activeLayerShape() && activeLayerShape()->shapeId() == KIS_LAYER_CONTAINER_ID ) {
        parent = dynamic_cast<KoShapeContainer*>( activeLayerShape() );
        Q_ASSERT( parent );
    }
    else {
        parent = activeLayerShape()->parent();
    }

    kDebug(41007) << "We found the parent: " << parent << endl;

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
    else if ( dynamic_cast<KisPaintLayer*>( layer.data() )  || dynamic_cast<KisAdjustmentLayer*>( layer.data() ) ) {
        shape = new KisLayerShape( parent, layer );
    }
    else if ( dynamic_cast<KisShapeLayer*>( layer.data() ) ) {
        shape = dynamic_cast<KisShapeLayer*>( layer.data() );
    }
    Q_ASSERT( shape );

    // Put the layer in the right place in the hierarchy
    shape->setParent( parent );
    parent->addChild(shape);

    m_d->layerShapes[layer] = shape;

    foreach( KoView *view, m_d->doc->views() ) {
        KisCanvas2 *canvas = ((KisView2*)view)->canvasBase();
        canvas->shapeManager()->add(shape);
        canvas->canvasWidget()->update();
    }

}

void KisShapeController::slotLayerRemoved( KisLayerSP layer,  KisGroupLayerSP wasParent,  KisLayerSP wasAboveThis )
{
//    kDebug(41007) << ">>>>> KisShapeController::slotLayerRemoved " << layer->name() << ", wasParent: " << wasParent->name() << ", wasAboveThis: " << wasAboveThis->name() << ", active layer: " << activeLayerShape()  << endl;
}

void KisShapeController::slotLayerMoved( KisLayerSP layer,  KisGroupLayerSP previousParent, KisLayerSP wasAboveThis )
{
//    kDebug(41007) << ">>>>> KisShapeController::slotLayerMmoved " << layer->name() << ", previousParent: " << previousParent->name() << ", wasAboveThis: " << wasAboveThis->name()  << ", active layer: " << activeLayerShape() << endl;
}

void KisShapeController::slotLayersChanged( KisGroupLayerSP rootLayer )
{
//    kDebug(41007) << ">>>>> KisShapeController::slotLayersChanged " << rootLayer->name()  << ", active layer: " << activeLayerShape() << endl;
}

void KisShapeController::slotLayerActivated( KisLayerSP layer )
{
    if ( layer )
        kDebug(41007) << ">>>>> KisShapeController::slotLayerActivated. Activating layer " << layer->name() << ", active layer: " << activeLayerShape()  << endl;
    else {
        kDebug(41007) << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>> layer is 0";
        return;
    }
    if (!m_d->layerShapes.contains( layer ) ) {
        kDebug(41007) << "A layer activated that is _not_ in the layer-shapes map!\n";
        return;
    }

    if ( activeLayerShape() )
        kDebug(41007) << "Active layer shape has id: " << activeLayerShape()->shapeId() << endl;
    else
        kDebug( 41007 ) << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> no active layer shape\n";

    if ( activeLayerShape()->shapeId() == KIS_SHAPE_LAYER_ID ) {
        // Automatically select all shapes in this newly selected shape layer?
    }

    foreach( KoView *view, m_d->doc->views() ) {
        KisCanvas2 * canvas = static_cast<KisView2*>( view )->canvasBase();
        KoSelection * selection = canvas->shapeManager()->selection();
        selection->deselectAll();
        selection->select( activeLayerShape() );
    }

}


KoShape * KisShapeController::activeLayerShape()
{
    if ( !m_d->image ) {
        kDebug(41007) << "activeLayerShape() current image = " << m_d->image << endl;
        return 0;
    }

    KisLayerSP activeLayer = m_d->image->activeLayer();
    if ( activeLayer ) kDebug(41007) << "Active layer is not 0 and has name: " << activeLayer->name() << endl;

    if ( m_d->layerShapes.contains( m_d->image->activeLayer() ) ) {

        return m_d->layerShapes[m_d->image->activeLayer()];
    }
    else {
        kDebug(41007) << "Could not find layer " << m_d->image->activeLayer() << " in the layer-shape map\n";
        return 0;
    }
}


#include "kis_shape_controller.moc"
