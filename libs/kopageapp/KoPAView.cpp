/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoPAView.h"

#include <QGridLayout>
#include <QToolBar>
#include <QScrollBar>
#include <QTimer>

#include <KoCanvasController.h>
#include <KoToolManager.h>
#include <KoToolProxy.h>
#include <KoZoomHandler.h>
#include <KoToolBoxFactory.h>
#include <KoShapeSelectorFactory.h>
#include <KoShapeController.h>
#include <KoShapeManager.h>
#include <KoZoomAction.h>
#include <KoTextSelectionHandler.h>
#include <KoSelection.h>
#include <KoToolDockerFactory.h>
#include <KoToolDocker.h>
#include <KoShapeLayer.h>

#include <KoPACanvas.h>
#include <KoPADocument.h>
#include <KoPAPage.h>

#include <klocale.h>
#include <kicon.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>

KoPAView::KoPAView( KoPADocument *document, QWidget *parent )
: KoView( document, parent )
, m_doc( document )
, m_activePage( 0 )
{
    initGUI();
    initActions();

    if ( m_doc->pageCount() > 0 )
        setActivePage( m_doc->pageByIndex( 0 ) );
}

KoPAView::~KoPAView()
{
    KoToolManager::instance()->removeCanvasController( m_canvasController );
}


KoPAPageBase* KoPAView::activePage() const
{
    return m_activePage;
}

void KoPAView::updateReadWrite( bool readwrite )
{
    Q_UNUSED( readwrite );
}

void KoPAView::initGUI()
{
    QGridLayout * gridLayout = new QGridLayout( this );
    gridLayout->setMargin( 0 );
    gridLayout->setSpacing( 0 );
    setLayout( gridLayout );

    m_canvas = new KoPACanvas( this, m_doc );
    m_canvasController = new KoCanvasController( this );
    m_canvasController->setCanvas( m_canvas );
    KoToolManager::instance()->addController( m_canvasController );
    KoToolManager::instance()->registerTools( actionCollection(), m_canvasController );

    //Ruler
    m_horizontalRuler = new KoRuler(this, Qt::Horizontal, viewConverter());
    m_horizontalRuler->setShowMousePosition(true);
    m_horizontalRuler->setUnit(m_doc->unit());
    m_verticalRuler = new KoRuler(this, Qt::Vertical, viewConverter());
    m_verticalRuler->setUnit(m_doc->unit());
    m_verticalRuler->setShowMousePosition(true);

    connect(m_doc, SIGNAL(unitChanged(KoUnit)), m_horizontalRuler, SLOT(setUnit(KoUnit)));
    connect(m_doc, SIGNAL(unitChanged(KoUnit)), m_verticalRuler, SLOT(setUnit(KoUnit)));

    gridLayout->addWidget(m_horizontalRuler, 0, 1);
    gridLayout->addWidget(m_verticalRuler, 1, 0);
    gridLayout->addWidget( m_canvasController, 1, 1 );

    connect(m_canvasController, SIGNAL(canvasOffsetXChanged(int)),
            m_horizontalRuler, SLOT(setOffset(int)));
    connect(m_canvasController, SIGNAL(canvasOffsetYChanged(int)),
            m_verticalRuler, SLOT(setOffset(int)));
    connect(m_canvasController, SIGNAL(sizeChanged(const QSize&)),
             this, SLOT(canvasControllerResized()));
    connect(m_canvasController, SIGNAL(canvasMousePositionChanged(const QPoint&)),
             this, SLOT(updateMousePosition(const QPoint&)));

    KoToolBoxFactory toolBoxFactory(m_canvasController, "Tools" );
    createDockWidget( &toolBoxFactory );
    KoShapeSelectorFactory shapeSelectorFactory;
    createDockWidget( &shapeSelectorFactory );
    KoToolDockerFactory toolDockerFactory;
    KoToolDocker* toolDocker = qobject_cast<KoToolDocker*>(createDockWidget(&toolDockerFactory));

    connect(m_canvasController, SIGNAL(setToolOptionDocker(QWidget*)), toolDocker, SLOT(newOptionWidget(QWidget*)));
    connect(shapeManager(), SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    connect(m_canvas, SIGNAL(documentSize(const QSize&)), m_canvasController, SLOT(setDocumentSize(const QSize&)));
    connect(m_canvasController, SIGNAL(moveDocumentOffset(const QPoint&)),
            m_canvas, SLOT(setDocumentOffset(const QPoint&)));

    show();
}

void KoPAView::initActions()
{
    m_actionViewShowGrid  = new KToggleAction(i18n("Show &Grid"), this);
    actionCollection()->addAction("view_grid", m_actionViewShowGrid );

    m_actionViewSnapToGrid = new KToggleAction(i18n("Snap to Grid"), this);
    actionCollection()->addAction("view_snaptogrid", m_actionViewSnapToGrid);

    m_viewRulers  = new KToggleAction(i18n("Show Rulers"), this);
    actionCollection()->addAction("view_rulers", m_viewRulers );
    m_viewRulers->setToolTip(i18n("Show/hide the view's rulers"));
    connect(m_viewRulers, SIGNAL(triggered(bool)), this, SLOT(setShowRulers(bool)));
    setShowRulers(true);

    m_viewZoomAction = new KoZoomAction(KoZoomMode::ZOOM_WIDTH | KoZoomMode::ZOOM_PAGE,
                                        i18n("Zoom"), KIcon("zoom-original"), KShortcut(),
                                        actionCollection(), "view_zoom");
    connect(m_viewZoomAction, SIGNAL(zoomChanged(KoZoomMode::Mode, int)),
            this, SLOT(viewZoom(KoZoomMode::Mode, int)));
    // plug the zoom action into view bar
    viewBar()->addAction(m_viewZoomAction);
}

void KoPAView::viewSnapToGrid()
{
}

void KoPAView::viewGrid()
{

}

void KoPAView::viewZoom(KoZoomMode::Mode mode, int zoom)
{
    // No point trying to zoom something that isn't there...
    if ( m_activePage == 0)
        return;

    int newZoom = zoom;
    QString zoomString;

    if ( mode == KoZoomMode::ZOOM_WIDTH) 
    {
        zoomString = KoZoomMode::toString( mode );
        KoPageLayout layout = m_activePage->pageLayout();
        newZoom = qRound(static_cast<double>(m_canvasController->viewport()->size().width() * 100 ) /
            ( m_zoomHandler.resolutionX() * layout.width ) ) - 1;

        if ( ( newZoom != m_zoomHandler.zoomInPercent() ) &&
            m_canvasController->verticalScrollBar()->isHidden() )
        {
            QTimer::singleShot( 0, this, SLOT( recalculateZoom() ) );
        }
    } 
    else if ( mode == KoZoomMode::ZOOM_PAGE ) 
    {
        zoomString = KoZoomMode::toString( mode );
        KoPageLayout layout = m_activePage->pageLayout();
        double height = m_zoomHandler.resolutionY() * layout.height;
        double width = m_zoomHandler.resolutionX() * layout.width;
        QSize viewportSize = m_canvasController->viewport()->size();
        newZoom = qMin(qRound(static_cast<double>(viewportSize.height() * 100) / height ),
                    qRound(static_cast<double>( viewportSize.width() * 100) / width ) ) - 1;

    } 
    else 
    {
        zoomString = i18n("%1%", newZoom);
    }

    // Don't allow smaller zoom then 10% or bigger then 2000%
    if ( ( newZoom < 10 ) || ( newZoom > 2000 ) || ( newZoom == m_zoomHandler.zoomInPercent() ) ) 
    {
        return;
    }

    m_zoomHandler.setZoomMode( mode );
    m_viewZoomAction->setZoom( zoomString );
    setZoom( newZoom );
    m_canvas->setFocus();
}

void KoPAView::setZoom(int zoom)
{
    m_zoomHandler.setZoom(zoom);
    m_canvas->updateSize();
    m_viewZoomAction->setEffectiveZoom(zoom);
}

KoShapeManager* KoPAView::shapeManager() const
{
    return m_canvas->shapeManager();
}


void KoPAView::setActivePage( KoPAPageBase* page )
{
    if ( !page )
        return;

    m_activePage = page;
    QList<KoShape*> shapes = page->iterator();
    shapeManager()->setShapes( shapes );
    //Make the top most layer active
    if(!shapes.isEmpty()) {
        KoShapeLayer* layer = dynamic_cast<KoShapeLayer*>(shapes.last());
        shapeManager()->selection()->setActiveLayer(layer);
    }

    m_canvas->updateSize();
    KoPageLayout layout = page->pageLayout();
    m_horizontalRuler->setRulerLength(layout.width);
    m_verticalRuler->setRulerLength(layout.height);
    m_horizontalRuler->setActiveRange(layout.left, layout.width - layout.right);
    m_verticalRuler->setActiveRange(layout.top, layout.height - layout.bottom);

}

void KoPAView::canvasControllerResized()
{
    if ( m_zoomHandler.zoomMode() != KoZoomMode::ZOOM_CONSTANT )
    {
        recalculateZoom();
    }

    m_horizontalRuler->setOffset( m_canvasController->canvasOffsetX() );
    m_verticalRuler->setOffset( m_canvasController->canvasOffsetY() );
}

void KoPAView::recalculateZoom()
{
    viewZoom(m_zoomHandler.zoomMode(), m_zoomHandler.zoomInPercent());
}

void KoPAView::updateMousePosition(const QPoint& position)
{
    m_horizontalRuler->updateMouseCoordinate(position.x());
    m_verticalRuler->updateMouseCoordinate(position.y());

    // Update the selection borders in the rulers while moving with the mouse
    if(m_canvas->shapeManager()->selection() && (m_canvas->shapeManager()->selection()->count() > 0)) {
        QRectF boundingRect = m_canvas->shapeManager()->selection()->boundingRect();
        m_horizontalRuler->updateSelectionBorders(boundingRect.x(), boundingRect.right());
        m_verticalRuler->updateSelectionBorders(boundingRect.y(), boundingRect.bottom());
    }
}

void KoPAView::selectionChanged()
{
    // Show the borders of the selection in the rulers
    if(m_canvas->shapeManager()->selection() && (m_canvas->shapeManager()->selection()->count() > 0)) {
        QRectF boundingRect = m_canvas->shapeManager()->selection()->boundingRect();
        m_horizontalRuler->setShowSelectionBorders(true);
        m_verticalRuler->setShowSelectionBorders(true);
        m_horizontalRuler->updateSelectionBorders(boundingRect.x(), boundingRect.right());
        m_verticalRuler->updateSelectionBorders(boundingRect.y(), boundingRect.bottom());
    } else {
        m_horizontalRuler->setShowSelectionBorders(false);
        m_verticalRuler->setShowSelectionBorders(false);
    }
}

void KoPAView::setShowRulers(bool show)
{
    m_horizontalRuler->setVisible(show);
    m_verticalRuler->setVisible(show);

    m_viewRulers->setChecked(show);
}

#include "KoPAView.moc"
