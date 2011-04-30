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
#include <QPoint>
#include <QRect>
#include <QPolygon>
#include <QString>
#include <QPainter>

// KDE
#include <KDebug>

// Libsvm
#include "SvmEnums.h"
#include "SvmStructs.h"
#include "SvmGraphicsContext.h"


#define DEBUG_SVMPAINT 0


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

#if DEBUG_SVMPAINT
    kDebug(31000) << "scale before:" << scaleX << ", " << scaleY;
#endif

    // Keep aspect ratio.  Use the smaller value so that we don't get
    // an overflow in any direction.
#if 0   // Set this to 1 to keep aspect ratio.
    if ( scaleX > scaleY )
        scaleX = scaleY;
    else
        scaleY = scaleX;
#endif
#if DEBUG_SVMPAINT
    kDebug(31000) << "shape size:" << m_outputSize;
    kDebug(31000) << "scale after:" << scaleX << ", " << scaleY;
#endif

    // Transform the SVM object so that it fits in the shape as much
    // as possible.  The topleft will be the top left of the shape.
    m_painter->scale( scaleX, scaleY );
    //m_painter->translate(-header->bounds().left(), -header->bounds().top());

    m_outputTransform = m_painter->transform();
    //m_worldTransform = QTransform();
    
    m_painter->setRenderHint(QPainter::Antialiasing);
    m_painter->setRenderHint(QPainter::TextAntialiasing);
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

void SvmPainterBackend::polygon( SvmGraphicsContext &context, const QPolygon &polygon )
{
    updateFromGraphicscontext(context);
    m_painter->drawPolygon(polygon);
}

void SvmPainterBackend::textArray(SvmGraphicsContext &context,
                                  const QPoint &point, const QString &string)
{
    updateFromGraphicscontext(context);

    m_painter->save();
    m_painter->setPen(context.textColor);
    m_painter->drawText(point, string);
    m_painter->restore();
}


// ----------------------------------------------------------------
//                         Private functions

void SvmPainterBackend::updateFromGraphicscontext(SvmGraphicsContext &context)
{
    if (context.changedItems & GCLineColor) {
        m_painter->setPen(context.lineColor);
#if DEBUG_SVMPAINT
        kDebug(31000) << "*** Setting line color to" << context.lineColor;
#endif
    }
    if (context.changedItems & GCFillBrush) {
        m_painter->setBrush(context.fillBrush);
#if DEBUG_SVMPAINT
        kDebug(31000) << "*** Setting fill brush to" << context.fillBrush;
#endif
    }
    if (context.changedItems & GCTextColor) {
        m_painter->setPen(context.textColor);
#if DEBUG_SVMPAINT
        kDebug(31000) << "*** Setting text color to" << context.textColor;
#endif
    }
    if (context.changedItems & GCMapMode) {
        // FIXME
    }
    if (context.changedItems & GCFont) {
        m_painter->setFont(context.font);
#if DEBUG_SVMPAINT
        kDebug(31000) << "*** Setting font to" << context.font;
#endif
    }

    // Reset all changes until next time.
    context.changedItems = 0;
}



}
