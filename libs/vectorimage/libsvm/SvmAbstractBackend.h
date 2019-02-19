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

#ifndef SVMABSTRACTBACKEND_H
#define SVMABSTRACTBACKEND_H

#include "kritavectorimage_export.h"

#include "SvmEnums.h"
#include "SvmStructs.h"
#include "SvmGraphicsContext.h"


class QPoint;
class QRect;
class QPolygon;
class QString;


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
    Abstract output strategy for SVM Parser
*/
class KRITAVECTORIMAGE_EXPORT SvmAbstractBackend
{
public:
    SvmAbstractBackend() {};
    virtual ~SvmAbstractBackend() {};

    /**
       Initialisation routine

       \param header the SVM Header record
    */
    virtual void init(const SvmHeader &header) = 0;

    /**
       Cleanup routine

       This function is called when the parsing is done.  Any
       initializations that are done in init() can be undone here if
       necessary.
    */
    virtual void cleanup() = 0;

    /**
       Close-out routine
    */
    virtual void eof() = 0;

    virtual void rect(SvmGraphicsContext &context, const QRect &rect) = 0;

    /**
       Handler META_POLYLINE_ACTION

       This action type specifies how to output a multi-segment line
       (unfilled polyline).

       \param context the graphics context to be used when drawing the polyline
       \param polyline the sequence of points that describe the line

       \note the line is not meant to be closed nor filled, i.e. do
       not connect the last point to the first point.
    */
    virtual void polyLine(SvmGraphicsContext &context, const QPolygon &polyline) = 0;

    virtual void polygon(SvmGraphicsContext &context, const QPolygon &polygon) = 0;

    virtual void polyPolygon(SvmGraphicsContext &context, const QList<QPolygon> &polyPolygon) = 0;

    virtual void textArray(SvmGraphicsContext &context,
                           const QPoint &point, const QString &string,
                           quint16 startIndex, quint16 len,
                           quint32 dxArrayLen, qint32 *dxArray) = 0;
};


}

#endif
