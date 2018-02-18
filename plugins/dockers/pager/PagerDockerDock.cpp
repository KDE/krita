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

#include "PagerDockerDock.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QDir>
#include <kis_canvas2.h>
#include <KisDocument.h>
#include <KisPart.h>
#include <QtWidgets/QtWidgets>

PagerDockerDock::PagerDockerDock()
    : QDockWidget(i18n("Pager"))
{
    auto *widget = new QWidget(this);
    setWidget(widget);

    auto *layout = new QHBoxLayout(widget);
    widget->setLayout(layout);

    auto *nextButton = new QPushButton(">>", widget);
    auto *previousButton = new QPushButton("<<", widget);

    layout->addWidget(previousButton);
    layout->addWidget(nextButton);

    connect(nextButton, SIGNAL(clicked()), this, SLOT(slotNextPage()));
    connect(previousButton, SIGNAL(clicked()), this, SLOT(slotPreviousPage()));
}

PagerDockerDock::~PagerDockerDock() {}

void PagerDockerDock::setCanvas(KoCanvasBase *canvas)
{
    m_canvas = qobject_cast<KisCanvas2*>(canvas);
}
void PagerDockerDock::unsetCanvas()
{
    m_canvas = nullptr;
}

void PagerDockerDock::slotNextPage()
{
    flipPage(true);
}

void PagerDockerDock::slotPreviousPage()
{
    flipPage(false);
}

void PagerDockerDock::flipPage(bool forward)
{
    if (!m_canvas) return;

    KisDocument *activeDocument = m_canvas->imageView()->document();

    QUrl url = activeDocument->url();
    if (url.isEmpty() || !url.isLocalFile()) return;
    QFileInfo fileInfo(url.toLocalFile());
    QDir directory = fileInfo.absoluteDir();

    QStringList openSiblingFiles = getOpenFilesInDirectory(directory.path());
    QStringList allSiblingFiles = getKraFilesInDirectory(directory);

    int step = chooseStepSize(fileInfo.fileName(), openSiblingFiles, allSiblingFiles, forward);
    if (step == 0) return;

    pageViews(directory, allSiblingFiles, step);
}

QStringList PagerDockerDock::getOpenFilesInDirectory(const QString &path)
{
    QStringList openSiblingFiles;
    Q_FOREACH(KisDocument *document, KisPart::instance()->documents()) {
        if (!document->url().isLocalFile()) continue;

        QFileInfo docFile(document->url().toLocalFile());
        if (docFile.absoluteDir() == path) {
            openSiblingFiles.append(docFile.fileName());
        }
    }

    return openSiblingFiles;
}

QStringList PagerDockerDock::getKraFilesInDirectory(QDir directory)
{
    directory.setNameFilters({"*.kra"});
    directory.setFilter(QDir::Files);
    directory.setSorting(QDir::Name | QDir::IgnoreCase);
    return directory.entryList();
}

int PagerDockerDock::chooseStepSize(const QString &baseFile,
                                    const QStringList &openSiblingFiles, const QStringList &allSiblingFiles, bool forward) const
{
    int baseIndex = allSiblingFiles.indexOf(baseFile);
    if (baseIndex < 0) return 0;

    int step = 1;
    while (baseIndex > 0 && openSiblingFiles.contains(allSiblingFiles[baseIndex - 1])) {
        baseIndex--;
    }

    while (baseIndex + step < allSiblingFiles.count() && openSiblingFiles.contains(allSiblingFiles[baseIndex + step])) {
        step++;
    }

    return forward ? step : -step;
}

void PagerDockerDock::pageViews(const QDir &directory, const QStringList &allSiblingFiles, int step) const
{
    QList<QPointer<KisView>> views = KisPart::instance()->views();

    Q_FOREACH(KisView *view, views) {
        QString newFilename = getSteppedFilename(view->document(), directory, allSiblingFiles, step);
        if (!newFilename.isEmpty()) {
            if (!view->queryClose()) return;
        }
    }

    Q_FOREACH(KisView *view, views) {
        QString newFilename = getSteppedFilename(view->document(), directory, allSiblingFiles, step);
        if (newFilename.isEmpty()) continue;

        view->mainWindow()->openDocument(QUrl::fromLocalFile(newFilename), KisMainWindow::None);
        view->closeView();
    }
}

QString PagerDockerDock::getSteppedFilename(const KisDocument *document, const QDir &directory,
                                             const QStringList &allSiblingFiles, int step) const
{
    QFileInfo file(document->url().toLocalFile());
    if (file.absoluteDir() != directory.path()) return QString();

    int index = allSiblingFiles.indexOf(file.fileName());
    if (index < 0) return QString();

    int newIndex = index + step;
    if (newIndex < 0 || newIndex >= allSiblingFiles.size()) return QString();

    return directory.filePath(allSiblingFiles[newIndex]);
}
