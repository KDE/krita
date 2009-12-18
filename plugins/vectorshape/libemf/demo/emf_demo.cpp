/*
  Copyright 2008 Brad Hards <bradh@frogmouth.net>

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

#include "EmfViewer.h"

#include <QApplication>

using namespace Libemf;

int main( int argc, char **argv )
{
    QApplication app( argc, argv );

    QSize     size( 1280, 800 );
    EmfViewer viewer( size );
    viewer.show();

    if ( argc > 1 ) {
        QString  filename( argv[1] );
        viewer.loadFile( filename );
    }

    app.exec();
}
