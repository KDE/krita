/*
 *  SPDX-FileCopyrightText: 2018 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    setLayout(new QHBoxLayout);
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
