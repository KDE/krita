/* This file is part of the KDE project
 *
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA..
 */
#include "kis_canvas2.h"

#include <QWidget>

#include <kdebug.h>

#include <KoUnit.h>
#include <KoViewConverter.h>
#include <KoShapeManager.h>
#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>

#include <kis_image.h>

#include "kis_view2.h"
#include "kis_config.h"
#include "kis_abstract_canvas_widget.h"
#include "kis_qpainter_canvas.h"
#include "kis_opengl_canvas2.h"


class KisCanvas2::KisCanvas2Private {

public:

    KisCanvas2Private( KoCanvasBase * parent, KoViewConverter * viewConverter, KisView2 * view )
        : viewConverter( viewConverter )
        , view( view )
        , canvasWidget( 0 )
        , shapeManager( new KoShapeManager(parent) )
        , monitorProfile( 0 )
        {
        }

    ~KisCanvas2Private()
        {
            delete shapeManager;
            delete monitorProfile;
        }

    KoViewConverter * viewConverter;
    KisView2 * view;
    KisAbstractCanvasWidget * canvasWidget;
    KoShapeManager * shapeManager;
    KoColorProfile * monitorProfile;
};

KisCanvas2::KisCanvas2(KoViewConverter * viewConverter, KisCanvasType canvasType, KisView2 * view, KoShapeControllerBase * sc)
    : KoCanvasBase(sc)
{
    m_d = new KisCanvas2Private(this, viewConverter, view);

    switch( canvasType ) {
    case OPENGL:
#ifdef HAVE_OPENGL
        //setCanvasWidget( new KisOpenGLCanvas2( this, view ) );
        break;
#else
        kWarning() << "OpenGL requested while its not available";
        Q_ASSERT(false);
#endif
    case MITSHM:
    case QPAINTER:
    default:
        setCanvasWidget( new KisQPainterCanvas( this, view ) );
    }
}

void KisCanvas2::setCanvasWidget(QWidget * widget)
{
    KisAbstractCanvasWidget * tmp = dynamic_cast<KisAbstractCanvasWidget*>( widget );
    Q_ASSERT_X( tmp, "setCanvasWidget", "Cannot cast the widget to a KisAbstractCanvasWidget" );
    m_d->canvasWidget = tmp;
}

KisCanvas2::~KisCanvas2()
{
    delete m_d;
}

void KisCanvas2::gridSize(double *horizontal, double *vertical) const
{
    Q_UNUSED( horizontal );
    Q_UNUSED( vertical );

}

bool KisCanvas2::snapToGrid() const
{
    return true;
}

void KisCanvas2::addCommand(KCommand *command, bool execute)
{
    Q_UNUSED( command );
    Q_UNUSED( execute );
}

KoShapeManager* KisCanvas2::shapeManager() const
{
    return m_d->shapeManager;
}


void KisCanvas2::updateCanvas(const QRectF& rc)
{
    // First convert from document coordinated to widget coordinates
    QRectF viewRect  = m_d->viewConverter->documentToView(rc);
    viewRect.adjust(-1,-1,1,1); // to avoid rounding errors
    m_d->canvasWidget->widget()->update( viewRect.toRect() );
}


KoViewConverter* KisCanvas2::viewConverter()
{
    return m_d->viewConverter;
}

QWidget* KisCanvas2::canvasWidget()
{
    return m_d->canvasWidget->widget();
}


KoUnit KisCanvas2::unit()
{
    return KoUnit(KoUnit::Pixel);
}

KoToolProxy * KisCanvas2::toolProxy() {
    return m_d->canvasWidget->toolProxy();
}

void KisCanvas2::setCanvasSize(int w, int h)
{
    m_d->canvasWidget->widget()->setMinimumSize( w, h );
}

KisImageSP KisCanvas2::image()
{
    return m_d->view->image();

}

void KisCanvas2::updateCanvas()
{
    m_d->canvasWidget->widget()->update();
}

void KisCanvas2::updateCanvas( const QRect rc )
{
    updateCanvas( QRectF( rc ) );
}

KoColorProfile *  KisCanvas2::monitorProfile()
{
    if (m_d->monitorProfile == 0) {
        resetMonitorProfile();
    }
    return m_d->monitorProfile;
}


void KisCanvas2::resetMonitorProfile()
{
    m_d->monitorProfile = KoColorProfile::getScreenProfile();

    if (m_d->monitorProfile == 0) {
        KisConfig cfg;
        QString monitorProfileName = cfg.monitorProfile();
        m_d->monitorProfile = KoColorSpaceRegistry::instance()->profileByName(monitorProfileName);
    }

}

KisImageSP KisCanvas2::currentImage()
{
    return m_d->view->image();
}


#include "kis_canvas2.moc"
