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
#ifndef LIBKIS_MAINWINDOW_H
#define LIBKIS_MAINWINDOW_H

#include <QObject>
#include <krita_export.h>
class KisMainWindow;

#include <view.h>

class LIBKIS_EXPORT MainWindow : public QObject
{
    Q_OBJECT
public:
    explicit MainWindow(KisMainWindow *mainWin, QObject *parent = 0);

signals:

public slots:


private:

    KisMainWindow *m_mainWindow;
};

#endif // MAINWINDOW_H
