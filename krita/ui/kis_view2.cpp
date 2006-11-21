/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "kis_view2.h"

#include <QGridLayout>
#include <QRect>
#include <QWidget>

#include <kstdaction.h>
#include <kxmlguifactory.h>
#include <klocale.h>
//#include <kparts/plugin.h>
//#include <kservice.h>
//#include <kservicetypetrader.h>
//#include <kparts/componentfactory.h>

#include <KoMainWindow.h>
#include <KoCanvasController.h>
#include <KoShapeManager.h>
#include <KoShape.h>
#include <KoRuler.h>
#include <KoSelection.h>
#include <KoToolBoxFactory.h>
#include <KoShapeSelectorFactory.h>
#include <KoZoomHandler.h>
#include <KoViewConverter.h>
#include <KoView.h>
#include <KoToolDocker.h>

#include <kis_image.h>
#include <kis_undo_adapter.h>
#include <kis_layer.h>

#include "kis_statusbar.h"
#include "kis_canvas2.h"
#include "kis_doc2.h"
#include "kis_dummy_shape.h"
#include "kis_factory2.h"
#include "kis_filter_manager.h"
#include "kis_opengl_canvas2.h"
#include "kis_qpainter_canvas.h"
#include "kis_resource_provider.h"
#include "kis_resource_provider.h"
#include "kis_selection_manager.h"
#include "kis_controlframe.h"
#include "kis_birdeye_box.h"
#include "kis_layerbox.h"
#include "kis_layer_manager.h"
#include "kis_zoom_manager.h"

class KisView2::KisView2Private {

public:

    KisView2Private(KisView2 * view)
        : filterManager( 0 )
        , horizontalRuler( 0 )
        , verticalRuler( 0 )
        {
            viewConverter = new KoZoomHandler( );

            canvas = new KisCanvas2( viewConverter, QPAINTER, view );
            shapeManager = canvas->shapeManager();
            // The canvas controller handles the scrollbars
            canvasController = new KoCanvasController( view );
            canvasController->setCanvas( canvas );

        }

    ~KisView2Private()
        {
            KoToolManager::instance()->removeCanvasController( canvasController );
            delete viewConverter;
            delete canvas;
            delete filterManager;
            delete selectionManager;
            delete layerManager;
            delete zoomManager;
        }

public:

    KisCanvas2 *canvas;
    KisDoc2 *doc;
    KoViewConverter *viewConverter;
    KoShapeManager * shapeManager;
    KoCanvasController * canvasController;
    KisResourceProvider * resourceProvider;
    KisFilterManager * filterManager;
    KoRuler * horizontalRuler;
    KoRuler * verticalRuler;
    KisStatusBar * statusBar;

    KisSelectionManager *selectionManager;
    KisControlFrame * controlFrame;
    KisBirdEyeBox * birdEyeBox;
    KisLayerBox * layerBox;
    KisLayerManager * layerManager;
    KisZoomManager * zoomManager;
};


KisView2::KisView2(KisDoc2 * doc,  QWidget * parent)
    : KoView(doc, parent)
{

    setInstance(KisFactory2::instance(), false);

    if (!doc->isReadWrite())
        setXMLFile("krita_readonly.rc");
    else
        setXMLFile("krita.rc");

    KStdAction::keyBindings( mainWindow()->guiFactory(),
                             SLOT( configureShortcuts() ),
                             actionCollection() );

    m_d = new KisView2Private(this);

    m_d->doc = doc;
    m_d->resourceProvider = new KisResourceProvider( this );

    // Add the image and select it immediately (later, we'll select
    // the first layer)
    m_d->shapeManager->add( doc->imageShape() );

    createActions();
    createManagers();
    createGUI();


    // Wait for the async image to have loaded
    if ( m_d->doc->isLoading() ) {
        connect( m_d->doc, SIGNAL( sigLoadingFinished() ),
                 this, SLOT( slotInitializeCanvas() ) );
    }
    else {
        slotInitializeCanvas();
    }

}


KisView2::~KisView2()
{
    delete m_d;
}

void KisView2::slotChildActivated(bool a) {

    // It should be so that the only part (child) we can activate, is
    // the current layer:
    KisImageSP img = image();
    if ( img && img->activeLayer())
    {
        if (a) {
            img->activeLayer()->activate();
        } else {
            img->activeLayer()->deactivate();
        }
    }

    KoView::slotChildActivated(a);
}



KisImageSP KisView2::image()
{
    return m_d->doc->currentImage();
}

KisResourceProvider * KisView2::resourceProvider()
{
    return m_d->resourceProvider;
}

KoCanvasBase * KisView2::canvasBase() const
{
    return m_d->canvas;
}

QWidget* KisView2::canvas() const
{
    return m_d->canvas->canvasWidget();
}

KisStatusBar * KisView2::statusBar() const
{
    return m_d->statusBar;
}

KisSelectionManager * KisView2::selectionManager()
{
    return m_d->selectionManager;
}

KisLayerManager * KisView2::layerManager()
{
    return m_d->layerManager;
}

KisZoomManager * KisView2::zoomManager()
{
    return m_d->zoomManager;
}

KisFilterManager * KisView2::filterManager()
{
    return m_d->filterManager;
}

KisUndoAdapter * KisView2::undoAdapter()
{
    return m_d->doc->undoAdapter();
}

void KisView2::slotInitializeCanvas()
{

    m_d->canvas->setCanvasSize( image()->width(), image()->height() );
    m_d->filterManager->updateGUI();
    m_d->selectionManager->updateGUI();
    //m_d->layerManager->updateGUI(true);
    m_d->zoomManager->updateGUI();


    KoToolDockerFactory toolDockerFactory(m_d->canvas);
    KoToolDocker * d =  dynamic_cast<KoToolDocker*>( createDockWidget( &toolDockerFactory ) );
    if(d)
        m_d->canvasController->setToolOptionDocker( d );
    else
        kDebug() << "Could not create tool docker: " << d << endl;

    KoSelection *select = m_d->shapeManager->selection();
    select->select( m_d->doc->imageShape() );
}


void KisView2::createGUI()
{

    KoToolBoxFactory toolBoxFactory( "Krita" );
    createDockWidget( &toolBoxFactory );

    KoShapeSelectorFactory shapeSelectorFactory;
    createDockWidget( &shapeSelectorFactory );

    // Put the canvascontroller in a layout so it resizes with us
    QGridLayout * layout = new QGridLayout( this );
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);

    m_d->horizontalRuler = new KoRuler(this, Qt::Horizontal, (KoZoomHandler*)m_d->viewConverter);
    m_d->horizontalRuler->setShowMousePosition(true);
    m_d->verticalRuler = new KoRuler(this, Qt::Vertical, (KoZoomHandler*)m_d->viewConverter);
    m_d->verticalRuler->setShowMousePosition(true);

    layout->addWidget(m_d->horizontalRuler, 0, 1);
    layout->addWidget(m_d->verticalRuler, 1, 0);
    layout->addWidget(m_d->canvasController, 1, 1);

    KoToolManager::instance()->addControllers(m_d->canvasController,
                                              static_cast<KoShapeControllerBase*>( m_d->doc->imageShape()) );

    connect(m_d->canvasController, SIGNAL(canvasOffsetXChanged(int)),
            m_d->horizontalRuler, SLOT(setOffset(int)));

    connect(m_d->canvasController, SIGNAL(canvasOffsetYChanged(int)),
            m_d->verticalRuler, SLOT(setOffset(int)));

    KisBirdEyeBoxFactory birdeyeFactory(this);
    m_d->birdEyeBox = qobject_cast<KisBirdEyeBox*>( createDockWidget( &birdeyeFactory ) );

    KisLayerBoxFactory layerboxFactory( this );
    m_d->layerBox = qobject_cast<KisLayerBox*>( createDockWidget( &layerboxFactory ) );

    m_d->statusBar = new KisStatusBar( KoView::statusBar() );
    m_d->controlFrame = new KisControlFrame( mainWindow(), this );

    show();

}


void KisView2::createActions()
{
}


void KisView2::createManagers()
{
    // Create the managers for filters, selections, layers etc.
    // XXX: When the currentlayer changes, call updateGUI on all
    // managers

    m_d->filterManager = new KisFilterManager(this, m_d->doc);
    m_d->filterManager->setup(actionCollection());

    m_d->selectionManager = new KisSelectionManager( this, m_d->doc );
    m_d->selectionManager->setup( actionCollection() );

    m_d->layerManager = new KisLayerManager( this, m_d->doc );
    m_d->layerManager->setup( actionCollection() );

    m_d->zoomManager = new KisZoomManager( this, m_d->viewConverter );
    m_d->zoomManager->setup( actionCollection() );


}

#include "kis_view2.moc"
