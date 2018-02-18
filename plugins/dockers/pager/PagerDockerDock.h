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

#ifndef PAGERDOCKERDOCK_H
#define PAGERDOCKERDOCK_H

#include <QObject>
#include <QDir>
#include <QString>
#include <QDockWidget>

#include <KoCanvasObserverBase.h>

class KisDocument;
class KisCanvas2;

class PagerDockerDock : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT

public:
    PagerDockerDock();
    ~PagerDockerDock() override;

    QString observerName() override { return "PagerDockerDock"; }

protected:
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

private Q_SLOTS:
    void slotNextPage();
    void slotPreviousPage();

private:
    void flipPage(bool forward);
    QStringList getOpenFilesInDirectory(const QString &path);
    QStringList getKraFilesInDirectory(QDir directory);
    int chooseStepSize(const QString &baseFile, const QStringList &openSiblingFiles, const QStringList &allSiblingFiles, bool forward) const;
    void pageViews(const QDir &directory, const QStringList &allSiblingFiles, int step) const;
    QString getSteppedFilename(const KisDocument *document, const QDir &directory, const QStringList &allSiblingFiles, int step) const;

    KisCanvas2 *m_canvas;
};

#endif
