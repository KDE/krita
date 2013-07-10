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

#ifndef SVMPAINTERBACKEND_H
#define SVMPAINTERBACKEND_H

#include "SvmAbstractBackend.h"
#include "kovectorimage_export.h"

#include <QSize>
#include <QTransform>

#include "SvmEnums.h"
#include "SvmStructs.h"
#include "SvmGraphicsContext.h"

class QRect;
class QPolygon;
class QPainter;


/**
   \file

   Primary definitions for SVM output backend
*/

/**
   Namespace for StarView Metafile (SVM) classes
*/
namespace Libsvm
{

/**
    Painter output strategy for SVM Parser
*/
class KOVECTORIMAGE_EXPORT SvmPainterBackend : public SvmAbstractBackend
{
public:
    SvmPainterBackend(QPainter *painter, const QSize &outputSize);
    virtual ~SvmPainterBackend();

    /**
       Initialisation routine

       \param header the SVM Header record
    */
    virtual void init(const SvmHeader &header);

    /**
       Cleanup routine

       This function is called when the painting is done.  Any
       initializations that are done in init() can be undone here if
       necessary.
    */
    virtual void cleanup();

    /**
       Close-out routine
    */
    virtual void eof();

    virtual void rect( SvmGraphicsContext &context, const QRect &rect );

    /**
       Handler META_POLYLINE_ACTION

       This action type specifies how to output a multi-segment line
       (unfilled polyline).

       \param context the graphics context to be used when drawing the polyline
       \param polygon the sequence of points that describe the line

       \note the line is not meant to be closed (i.e. do not connect
       the last point to the first point) or filled.
    */
    virtual void polyLine(SvmGraphicsContext &context, const QPolygon &polyline);

    virtual void polygon(SvmGraphicsContext &context, const QPolygon &polygon);

    virtual void polyPolygon(SvmGraphicsContext &context, const QList<QPolygon> &polyPolygon);

    virtual void textArray(SvmGraphicsContext &context,
                           const QPoint &point, const QString &string,
                           quint16 startIndex, quint16 len,
                           quint32 dxArrayLen, qint32 *dxArray);

 private:
    void updateFromGraphicscontext(SvmGraphicsContext &context);

 private:
    QPainter *m_painter;
    QSize     m_outputSize;

    QTransform m_outputTransform;
};


}

#endif
