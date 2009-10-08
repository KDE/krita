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

#ifndef EMFVIEWER_H
#define EMFVIEWER_H

#include "../EmfParser.h"

#include <QLabel>
#include <QMainWindow>

using namespace QEmf;

class EmfViewer : public QMainWindow
{
    Q_OBJECT

public:
    EmfViewer();
    ~EmfViewer();

private Q_SLOTS:
    void slotOpenFile();

private:
    void loadFile( const QString &fileName );

    QAction *m_fileOpenAction;
    QLabel *m_label;
};

#endif
