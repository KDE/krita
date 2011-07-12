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
#include <QFont>

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
    GCLineColor     = 0x0001,
    GCFillColor     = 0x0002,
    GCTextColor     = 0x0004,
    GCTextFillColor = 0x0008,
    GCTextAlign     = 0x0010,
    GCMapMode       = 0x0020,
    GCFont          = 0x0040,
    GCOverlineColor = 0x0080
    // ...more here
};


struct SvmGraphicsContext {
    SvmGraphicsContext();

    QColor    lineColor;
    bool      lineColorSet; // true: use lineColor, false: set penStyle to Qt::NoPen.
    QColor    fillColor;
    bool      fillColorSet; // true: use fillColor, false: set brushStyle to Qt::NoBrush.
    QColor    textColor;
    QColor    textFillColor;
    bool      textFillColorSet;
    TextAlign textAlign;
    MapMode   mapMode;
    quint32   layoutMode;
    QFont     font;
    QColor    overlineColor;
    bool      overlineColorSet;
    // ... much more here;

    quint32  changedItems;      // bitmap
};


}

#endif
