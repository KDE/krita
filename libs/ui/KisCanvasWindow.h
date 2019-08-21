/*
 *  Copyright (c) 2018 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KISCANVASWINDOW_H
#define KISCANVASWINDOW_H

#include <QWidget>

class KisMainWindow;

/**
 * Window for the canvas (mdi) area. Used when detached canvas mode is enabled.
 */
class KisCanvasWindow : public QWidget
{
public:
    explicit KisCanvasWindow(KisMainWindow *mainWindow);
    ~KisCanvasWindow() override;

    QWidget * swapMainWidget(QWidget *widget);

    void closeEvent(QCloseEvent *event) override;
private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif
