/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 * Copyright (C) 2012 KO GmbH. Contact: Boudewijn Rempt <boud@kogmbh.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "MainWindow.h"

#include "opengl/kis_opengl.h"

#include <QApplication>
#include <QResizeEvent>
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QFileInfo>
#include <QGLWidget>

#include <kcmdlineargs.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>

#include "kis_config.h"

#include "SketchDeclarativeView.h"
#include "RecentFileManager.h"
#include "DocumentManager.h"

class MainWindow::Private
{
public:
    Private() : allowClose(true) { }
    bool allowClose;
    QString currentSketchPage;
};

MainWindow::MainWindow(QStringList fileNames, QWidget* parent, Qt::WindowFlags flags )
    : QMainWindow( parent, flags ), d( new Private )
{
    qApp->setActiveWindow( this );

    setWindowTitle(i18n("Krita Sketch"));

    KisConfig cfg;
    cfg.setCursorStyle(CURSOR_STYLE_NO_CURSOR);
    cfg.setUseOpenGL(true);

    foreach(QString fileName, fileNames) {
        DocumentManager::instance()->recentFileManager()->addRecent(fileName);
    }


    QDeclarativeView* view = new SketchDeclarativeView();
    view->engine()->rootContext()->setContextProperty("mainWindow", this);

    QDir appdir(qApp->applicationDirPath());
    // for now, the app in bin/ and we still use the env.bat script
    appdir.cdUp();

    view->engine()->addImportPath(appdir.canonicalPath() + "/lib/calligra/imports");
    view->engine()->addImportPath(appdir.canonicalPath() + "/lib64/calligra/imports");
    QString mainqml = appdir.canonicalPath() + "/share/apps/kritasketch/kritasketch.qml";

    Q_ASSERT(QFile::exists(mainqml));
    if (!QFile::exists(mainqml)) {
        QMessageBox::warning(0, "No QML found", mainqml + " doesn't exist.");
    }
    QFileInfo fi(mainqml);

    view->setSource(QUrl::fromLocalFile(fi.canonicalFilePath()));
    view->setResizeMode( QDeclarativeView::SizeRootObjectToView );

    if (view->errors().count() > 0) {
        foreach(const QDeclarativeError &error, view->errors()) {
            kDebug() << error.toString();
        }
    }

    setCentralWidget(view);
}

bool MainWindow::allowClose() const
{
    return d->allowClose;
}

void MainWindow::setAllowClose(bool allow)
{
    d->allowClose = allow;
}

QString MainWindow::currentSketchPage() const
{
    return d->currentSketchPage;
}

void MainWindow::setCurrentSketchPage(QString newPage)
{
    d->currentSketchPage = newPage;
    emit currentSketchPageChanged();
}

void MainWindow::minimize()
{
    setWindowState(windowState() ^ Qt::WindowMinimized);
}

void MainWindow::closeWindow()
{
    //For some reason, close() does not work even if setAllowClose(true) was called just before this method.
    //So instead just completely quit the application, since we are using a single window anyway.
    QApplication::exit();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    // TODO this needs setting somewhere...
//     d->constants->setGridWidth( event->size().width() / d->constants->gridColumns() );
//     d->constants->setGridHeight( event->size().height() / d->constants->gridRows() );
    QWidget::resizeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (!d->allowClose) {
        event->ignore();
        emit closeRequested();
    } else {
        event->accept();
    }
}

MainWindow::~MainWindow()
{
    delete d;
}

#include "MainWindow.moc"
