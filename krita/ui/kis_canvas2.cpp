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
#include "kis_canvas2.h"

#include <QWidget>

#include <KoUnit.h>

#include <kis_image.h>

#include "kis_view2.h"
#include "kis_config.h"
#include "kis_view_converter.h"
#include "kis_abstract_canvas_widget.h"
#include "kis_qpainter_canvas.h"
#include "kis_opengl_canvas2.h"

class KisCanvas2::KisCanvas2Private {

public:

    KisCanvas2Private( KisViewConverter * viewConverter, KisView2 * view )
        : viewConverter( viewConverter )
        , view( view )
        , canvasWidget( 0 )
        {
        }

    KisViewConverter * viewConverter;
    KisView2 * view;
    KisAbstractCanvasWidget * canvasWidget;
};

KisCanvas2::KisCanvas2(KisViewConverter * viewConverter, KisCanvasType canvasType, KisView2 * view)
    : KoCanvasBase()
{
    m_d = new KisCanvas2Private(viewConverter, view);


    switch( canvasType ) {
    case OPENGL:
        setCanvasWidget( new KisOpenGLCanvas2( this, view ) );
        break;
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
    return 0;
}


void KisCanvas2::updateCanvas(const QRectF& rc)
{
    m_d->canvasWidget->widget()->update( rc.toRect() );
}


KoViewConverter* KisCanvas2::viewConverter()
{
    return m_d->viewConverter;
}

QWidget* KisCanvas2::canvasWidget()
{
    return m_d->canvasWidget->widget();
}


KoUnit::Unit KisCanvas2::unit()
{
    return KoUnit::U_PIXEL;
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
