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

#include "visio_tests.h"
#include <EnhMetaFile.h>

using namespace EnhancedMetafile;

void VisioTests::test1()
{
    Parser parser;
    DebugOutput output;
    parser.setOutput( &output );
    QVERIFY( parser.load( QString("visio-1.emf") ) );
}

void VisioTests::test_kde41()
{
    Parser parser;
    DebugOutput output;
    parser.setOutput( &output );
    QVERIFY( parser.load( QString("visio-kde41.emf") ) );
}

QTEST_MAIN( VisioTests )

#include "visio_tests.moc"
