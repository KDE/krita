/* This file is part of the Calligra project

  Copyright 2011 Inge Wallin <inge@lysator.liu.se>

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

#ifndef SVMGRAPHICSCONTEXT_H
#define SVMGRAPHICSCONTEXT_H


#include <QColor>
#include <QBrush>

#include "SvmEnums.h"
#include "SvmStructs.h"


/**
 * @file
 *
 * Graphics Context that is used in the backends of the SVM parser.
 */

/**
   Namespace for StarView Metafile (SVM) classes
*/
namespace Libsvm
{


enum GraphicsContextMembers {
    GCLineColor = 0x0001,
    GCFillBrush = 0x0002,
    GCMapMode   = 0x0004
    // ...more here
};


struct SvmGraphicsContext {
    SvmGraphicsContext();

    QColor   lineColor;
    QBrush   fillBrush;
    MapMode  mapMode;
    // ... much more here;

    quint32  changedItems;      // bitmap
};


}

#endif
