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
#include <QFileDialog>
#include <QMenuBar>

EmfViewer::EmfViewer() :
    QMainWindow()
{
    setWindowTitle( "EMF Demo Viewer" );

    QMenu *fileMenu = menuBar()->addMenu( "&File" );
    m_fileOpenAction = fileMenu->addAction( "&Open", this, SLOT( slotOpenFile() ) );
    m_fileOpenAction->setShortcut( Qt::CTRL + Qt::Key_O );
    fileMenu->addSeparator();
    fileMenu->addAction( "&Quit", qApp, SLOT( closeAllWindows() ) );

    resize( 1250, 850 );

    m_label = new QLabel(this);
    setCentralWidget( m_label );
}

EmfViewer::~EmfViewer()
{
}

void EmfViewer::slotOpenFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open EMF document", QDir::homePath(), "EMF Documents (*.emf)" );
    if (fileName.isEmpty()) {
        return;
    }

    loadFile( fileName );
}

void EmfViewer::loadFile( const QString &fileName )
{
    Parser parser;

    PainterOutput output;

    parser.setOutput( &output );
    parser.load( QString( fileName ) );

    QPixmap pixmap = QPixmap::fromImage( *( output.image() ) );
    m_label->setPixmap( pixmap.scaled( 1200, 800, Qt::KeepAspectRatio, Qt::SmoothTransformation ) );
    
    m_label->show();


}

#include "EmfViewer.moc"


