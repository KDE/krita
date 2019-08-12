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

#include <QHBoxLayout>

#include "KisCanvasWindow.h"
#include "KisMainWindow.h"

struct KisCanvasWindow::Private {
    KisMainWindow *mainWindow;

    Private(KisMainWindow *mainWindow)
        : mainWindow(mainWindow)
    {}
};

KisCanvasWindow::KisCanvasWindow(KisMainWindow *mainWindow)
    : QWidget(mainWindow)
      , d(new Private(mainWindow))
{
    setWindowFlags(Qt::Window);

    QLayout *layout = new QHBoxLayout(this);
    setLayout(layout);
}

KisCanvasWindow::~KisCanvasWindow() = default;

void KisCanvasWindow::closeEvent(QCloseEvent *event)
{
    d->mainWindow->setCanvasDetached(false);
    QWidget::closeEvent(event);
}

QWidget * KisCanvasWindow::swapMainWidget(QWidget *newWidget)
{
    QWidget *oldWidget = (layout()->count() > 0) ? (layout()->takeAt(0)->widget()) : nullptr;
    if (newWidget) {
        layout()->addWidget(newWidget);
    }
    return oldWidget;
}
