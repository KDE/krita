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

SvmPainterBackend::SvmPainterBackend()
    : mPainter(0)
{
}

SvmPainterBackend::~SvmPainterBackend()
{
}


void SvmPainterBackend::init( /*const Header *header*/ )
{
}

void SvmPainterBackend::cleanup( /*const Header *header*/ )
{
}

void SvmPainterBackend::eof()
{
}

void SvmPainterBackend::polyLine( SvmGraphicsContext &context, const QPolygon &polygon )
{
    kDebug(31000) << polygon;
}


}
