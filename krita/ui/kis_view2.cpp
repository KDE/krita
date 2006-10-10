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

#include <kstdaction.h>
#include <kxmlguifactory.h>

#include <KoMainWindow.h>
#include <KoCanvasController.h>

#include <kis_image.h>

#include "kis_factory2.h"
#include "kis_view_converter.h"
#include "kis_canvas2.h"
#include "kis_opengl_canvas2.h"
#include "kis_qpainter_canvas.h"
#include "kis_doc2.h"

class KisView2::KisView2Private {

public:

    KisView2Private(KisView2 * view)
        {
            viewConverter = new KisViewConverter( 1.0, 100, 96, 96 );

            canvas = new KisCanvas2( viewConverter, QPAINTER, view );

            // The canvas controller handles the scrollbars
            canvasController = new KoCanvasController( view );
            canvasController->setCanvas( canvas );
        }

    ~KisView2Private()
        {
            delete viewConverter;
            delete canvas;
        }

public:

    KisCanvas2 * canvas;
    KisDoc2 * doc;
    KisViewConverter * viewConverter;
    KoCanvasController * canvasController;
    QScrollArea * scrollArea;
};

KisView2::KisView2(KisDoc2 * doc,  QWidget * parent)
    : KoView(doc, parent)
{
    m_d = new KisView2Private(this);
    m_d->doc = doc;

    // Part stuff
    setInstance(KisFactory2::instance(), false);
    if (!doc->isReadWrite())
        setXMLFile("krita_readonly.rc");
    else
        setXMLFile("krita.rc");
    KStdAction::keyBindings( mainWindow()->guiFactory(),
                             SLOT( configureShortcuts() ),
                             actionCollection() );

    // Put the canvascontroller in a layout so it resizes with us
    QHBoxLayout * layout = new QHBoxLayout( this );
    layout->addWidget( m_d->canvasController );

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

void KisView2::slotInitializeCanvas()
{
    kDebug() << "Image completely loaded! W: "
             << image()->width() << ", H: "
             << image()->height() << endl;
    QRegion rg = image()->extent();
    QRect rc = rg.boundingRect();
    m_d->canvas->setCanvasSize( rc.width(), rc.height() );
}

#include "kis_view2.moc"
