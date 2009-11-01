/*
  Copyright 2008 Brad Hards  <bradh@frogmouth.net>
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

#ifndef EMFVIEWER_H
#define EMFVIEWER_H

#include "../EmfParser.h"

#include <QLabel>
#include <QMainWindow>

using namespace Libemf;

class EmfViewer : public QMainWindow
{
    Q_OBJECT

public:
    EmfViewer( QSize &size );
    ~EmfViewer();

    void loadFile( const QString &fileName );

private Q_SLOTS:
    void slotOpenFile();

private:

    // Actions
    QAction *m_fileOpenAction;
    QAction *m_fileQuitAction;

    // The central widget
    QLabel  *m_label;

    QSize    m_size;
};

#endif
