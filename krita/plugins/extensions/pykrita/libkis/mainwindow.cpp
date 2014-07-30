/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "mainwindow.h"

#include <KoMainWindow.h>
#include <kis_view2.h>

#include "view.h"

MainWindow::MainWindow(QObject *mainWin, QObject *parent)
    : QObject(parent)
    , m_mainWindow(qobject_cast<KoMainWindow*>(mainWin))
{
}

QList<View *> MainWindow::views()
{
    QList<View*> ret;
    KisView2 *view = qobject_cast<KisView2*>(m_mainWindow->rootView());
    if (view) {
        ret << new View(view, this);
    }
    return ret;
}
