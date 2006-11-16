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

#include <QHBoxLayout>
#include <QScrollArea>
#include <QRegion>
#include <QRect>
#include <QStringList>

#include <kstdaction.h>
#include <kxmlguifactory.h>
#include <kicon.h>
#include <klocale.h>
#include <kparts/plugin.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kparts/componentfactory.h>

#include <KoMainWindow.h>
#include <KoCanvasController.h>
#include <KoZoomAction.h>
#include <KoZoomHandler.h>
#include <KoToolRegistry.h>
#include <KoShapeManager.h>
#include <KoShape.h>
#include <KoSelection.h>

#include <kis_image.h>

#include "kis_canvas2.h"
#include "kis_doc2.h"
#include "kis_dummy_shape.h"
#include "kis_factory2.h"
#include "kis_filter_manager.h"
#include "kis_opengl_canvas2.h"
#include "kis_qpainter_canvas.h"
#include "kis_resource_provider.h"
#include "kis_resource_provider.h"

class KisView2::KisView2Private {

public:

    KisView2Private(KisView2 * view)
        : filterManager( 0 )
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
            delete viewConverter;
            delete canvas;
            delete filterManager;
        }

public:

    KisCanvas2 *canvas;
    KisDoc2 *doc;
    KoViewConverter *viewConverter;
    KoShapeManager * shapeManager;
    KoCanvasController * canvasController;
    KisResourceProvider * resourceProvider;
    KisFilterManager * filterManager;
    KAction *zoomAction;
    KAction *zoomIn;
    KAction *zoomOut;
    KAction *actualPixels;
    KAction *actualSize;
    KAction *fitToCanvas;
};



KisView2::KisView2(KisDoc2 * doc,  QWidget * parent)
    : KoView(doc, parent)
{
    m_d = new KisView2Private(this);

    m_d->doc = doc;
    m_d->resourceProvider = new KisResourceProvider( this );

    KoToolManager::instance()->addControllers(m_d->canvasController,
                                              static_cast<KoShapeControllerBase*>( m_d->doc->imageShape()) );

    // Add the image and select it immediately (later, we'll select
    // the first layer)
    m_d->shapeManager->add( doc->imageShape() );

    // Part stuff
    setInstance(KisFactory2::instance(), false);

    if (!doc->isReadWrite())
        setXMLFile("krita_readonly.rc");
    else
        setXMLFile("krita.rc");

    KStdAction::keyBindings( mainWindow()->guiFactory(),
                             SLOT( configureShortcuts() ),
                             actionCollection() );


    createActions();
    createManagers();

    // Put the canvascontroller in a layout so it resizes with us
    QHBoxLayout * layout = new QHBoxLayout( this );
    layout->addWidget( m_d->canvasController );

    show();

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

void KisView2::slotInitializeCanvas()
{
    kDebug() << ">>>>>>>>>>>>>>>>>>>>>>>>>> Image completely loaded! W: "
             << image()->width() << ", H: "
             << image()->height() << endl;

    m_d->canvas->setCanvasSize( image()->width(), image()->height() );
    m_d->filterManager->updateGUI();

    KoSelection *select = m_d->shapeManager->selection();
    select->select( m_d->doc->imageShape() );
}

void KisView2::slotZoomChanged(KoZoomMode::Mode mode, int zoom)
{
    KoZoomHandler *zoomHandler = (KoZoomHandler*)m_d->viewConverter;

    if(mode == KoZoomMode::ZOOM_CONSTANT)
    {
        double zoomF = zoom / 100.0;
        if(zoomF == 0.0) return;
        KoView::setZoom(zoomF);
        zoomHandler->setZoom(zoomF);
    }
    kDebug() << "zoom changed to: " << zoom <<  endl;

    zoomHandler->setZoomMode(mode);
//    QRectF imageRect = QRectF(0, 0, m_d->canvas->width(), m_d->canvas->height());
//    m_d->canvas->updateCanvas(imageRect);
    m_d->canvas->updateCanvas(QRectF(0, 0, 300,300));
}

void KisView2::createActions()
{

    // view actions
    m_d->zoomAction = new KoZoomAction(0, i18n("Zoom"), KIcon("14_zoom"), 0, actionCollection(), "zoom" );
    connect(m_d->zoomAction, SIGNAL(zoomChanged(KoZoomMode::Mode, int)),
          this, SLOT(slotZoomChanged(KoZoomMode::Mode, int)));
    m_d->zoomIn = KStdAction::zoomIn(this, SLOT(slotZoomIn()), actionCollection(), "zoom_in");
    m_d->zoomOut = KStdAction::zoomOut(this, SLOT(slotZoomOut()), actionCollection(), "zoom_out");

/*
    m_d->actualPixels = new KAction(i18n("Actual Pixels"), actionCollection(), "actual_pixels");
    m_d->actualPixels->setShortcut(Qt::CTRL+Qt::Key_0);
    connect(m_d->actualPixels, SIGNAL(triggered()), this, SLOT(slotActualPixels()));

    m_d->actualSize = KStdAction::actualSize(this, SLOT(slotActualSize()), actionCollection(), "actual_size");
    m_d->actualSize->setEnabled(false);
    m_d->fitToCanvas = KStdAction::fitToPage(this, SLOT(slotFitToCanvas()), actionCollection(), "fit_to_canvas");
*/

}


void KisView2::createManagers()
{
    // Create the managers for filters, selections, layers etc.
    // XXX: When the currentlayer changes, call updateGUI on all managers
    m_d->filterManager = new KisFilterManager(this, m_d->doc);

}

#include "kis_view2.moc"
