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

#include "svm_export.h"

#include "SvmEnums.h"
#include "SvmStructs.h"

class QPolygon;

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
class SVM_EXPORT SvmAbstractBackend
{
public:
    SvmAbstractBackend() {};
    virtual ~SvmAbstractBackend() {};

    /**
       Initialisation routine

       \param header the SVM Header record
    */
    virtual void init( /*const Header *header*/ ) = 0;

    /**
       Cleanup routine

       This function is called when the painting is done.  Any
       initializations that are done in init() can be undone here if
       necessary.

       \param header the SVM Header record
    */
    virtual void cleanup( /*const Header *header*/ ) = 0;

    /**
       Close-out routine
    */
    virtual void eof() = 0;

    /**
       Handler META_POLYLINE_ACTION

       This action type specifies how to output a multi-segment line
       (unfilled polyline).

       \param bounds the bounding rectangle for the line segments
       \param points the sequence of points that describe the line

       \note the line is not meant to be closed (i.e. do not connect
       the last point to the first point) or filled.
    */
    virtual void polyLine( SvmGraphicsContext &context, const QPolygon &polygon ) = 0;
};


}

#endif
