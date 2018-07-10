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


#include <EnhMetaFile.h>
#include <QApplication>

using namespace EnhancedMetafile;

bool testOneFile( const QString &filename )
{
    Parser parser;
    PainterOutput output;
    parser.setOutput( &output );
    if( parser.load( filename ) == false ) {
        debugVectorImage() << "failed to load" << filename;
        return false;
    } else {
        debugVectorImage() << "successfully loaded" << filename;
        return true;
    }
}

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    
    QStringList filesToTest;
    filesToTest << "pyemf-1.emf" << "pyemf-arc-chord-pie.emf" << "pyemf-deleteobject.emf";
    filesToTest << "pyemf-drawing1.emf" << "pyemf-fontbackground.emf" << "pyemf-optimize16bit.emf";
    filesToTest << "pyemf-paths1.emf" << "pyemf-poly1.emf" << "pyemf-poly2.emf" << "pyemf-setpixel.emf";
    filesToTest << "snp-1.emf" << "snp-2.emf" << "snp-3.emf";
    filesToTest << "visio-1.emf";
    
    
    Q_FOREACH ( const QString &fileToTest, filesToTest ) {
        if ( testOneFile( fileToTest ) == false ) {
            return -1;
        }
    }
    return 0;
}

