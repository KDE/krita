/*
  Copyright 2009 Inge Wallin <inge@lysator.liu.se>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either 
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public 
  License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/


// Own
#include "SvmPainterBackend.h"

// Qt
#include <QPainter>
#include <QRect>
#include <QPolygon>

// KDE
#include <KDebug>

// Libsvm
#include "SvmEnums.h"
#include "SvmStructs.h"
#include "SvmGraphicsContext.h"


/**
   Namespace for StarView Metafile (SVM) classes
*/
namespace Libsvm
{

SvmPainterBackend::SvmPainterBackend(QPainter *painter, const QSize &outputSize)
    : m_painter(painter)
    , m_outputSize(outputSize)
{
}

SvmPainterBackend::~SvmPainterBackend()
{
}


void SvmPainterBackend::init(const SvmHeader &header)
{
    // This is restored in cleanup().
    m_painter->save();

    qreal  scaleX = qreal( m_outputSize.width() )  / header.width;
    qreal  scaleY = qreal( m_outputSize.height() ) / header.height;

    // Keep aspect ration.  Use the smaller value so that we don't get
    // an overflow in any direction.
    if ( scaleX > scaleY )
        scaleX = scaleY;
    else
        scaleY = scaleX;
#if DEBUG_SVMPAINT
    kDebug(31000) << "scale = " << scaleX << ", " << scaleY;
#endif

    // Transform the SVM object so that it fits in the shape as much
    // as possible.  The topleft will be the top left of the shape.
    m_painter->scale( scaleX, scaleY );
    //m_painter->translate(-header->bounds().left(), -header->bounds().top());

    m_outputTransform = m_painter->transform();
    //m_worldTransform = QTransform();
}

void SvmPainterBackend::cleanup()
{
    // Restore the painter to what it was before init() was called.
    m_painter->restore();
}

void SvmPainterBackend::eof()
{
}


// ----------------------------------------------------------------
//                         Graphics output


void SvmPainterBackend::rect( SvmGraphicsContext &context, const QRect &rect )
{
    updateFromGraphicscontext(context);
    m_painter->drawRect(rect);
}

void SvmPainterBackend::polyLine( SvmGraphicsContext &context, const QPolygon &polyline )
{
    updateFromGraphicscontext(context);
    m_painter->drawPolyline(polyline);
}


// ----------------------------------------------------------------
//                         Private functions

void SvmPainterBackend::updateFromGraphicscontext(SvmGraphicsContext &context)
{
    if (context.changedItems & GCLineColor) {
        m_painter->setPen(context.lineColor);
        //kDebug(31000) << "*** Setting line color to" << context.lineColor;
    }
    if (context.changedItems & GCFillBrush) {
        m_painter->setBrush(context.fillBrush);
        //kDebug(31000) << "*** Setting fill brush to" << context.fillBrush;
    }
    if (context.changedItems & GCMapMode) {
        // FIXME
    }

    // Reset all changes until next time.
    context.changedItems = 0;
}



}
