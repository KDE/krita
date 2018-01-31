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
#include "KisWindowLayoutResource.h"

#include <QVector>
#include <QFile>
#include <QDomDocument>

#include <KisPart.h>
#include <KisDocument.h>

static const int WINDOW_LAYOUT_VERSION = 1;

struct KisWindowLayoutResource::Private
{
    struct Window {
        QByteArray geometry;
        QByteArray windowState;
    };

    QVector<Window> windows;

    Private() = default;

    explicit Private(QVector<Window> windows)
        : windows(std::move(windows))
    {}

    void openNecessaryWindows(QList<QPointer<KisMainWindow>> &currentWindows) {
        auto *kisPart = KisPart::instance();

        while (currentWindows.length() < windows.length()) {
            KisMainWindow *window = kisPart->createMainWindow();
            currentWindows.append(window);
            window->show();
        }
    }

    void closeUnneededWindows(QList<QPointer<KisMainWindow>> &currentWindows) {
        if (currentWindows.length() <= windows.length()) return;

        migrateViewsFromClosingWindows(currentWindows);

        while (currentWindows.length() > windows.length()) {
            KisMainWindow *window = currentWindows.takeLast();
            window->close();
        }
    }

    void migrateViewsFromClosingWindows(QList<QPointer<KisMainWindow>> &currentWindows) const
    {
        auto *kisPart = KisPart::instance();

        QVector<KisDocument*> visibleDocuments;
        Q_FOREACH(KisView *view, kisPart->views()) {
            KisMainWindow *window = view->mainWindow();
            int index = currentWindows.indexOf(window);

            if (index >= 0 && index < windows.length()) {
                visibleDocuments.append(view->document());
            }
        }

        Q_FOREACH(KisDocument *document, kisPart->documents()) {
            if (!visibleDocuments.contains(document)) {
                visibleDocuments.append(document);
                currentWindows.first()->newView(document);
            }
        }
    }
};

KisWindowLayoutResource::KisWindowLayoutResource(const QString &filename)
    : KoResource(filename)
    , d(new Private)
{}

KisWindowLayoutResource::KisWindowLayoutResource(const QString &filename, KisWindowLayoutResource::Private *d)
    : KoResource(filename)
    , d(d)
{}

KisWindowLayoutResource::~KisWindowLayoutResource()
{}

KisWindowLayoutResource * KisWindowLayoutResource::fromCurrentWindows(const QString &filename, const QList<QPointer<KisMainWindow>> &mainWindows)
{
    QVector<Private::Window> windows;

    Q_FOREACH(auto window, mainWindows) {
        Private::Window state;
        state.geometry = window->saveGeometry();
        state.windowState = window->saveState();

        windows.append(state);
    }

    return new KisWindowLayoutResource(filename, new Private(windows));
}

void KisWindowLayoutResource::applyLayout()
{
    KisPart *kisPart = KisPart::instance();
    QList<QPointer<KisMainWindow>> currentWindows = kisPart->mainWindows();

    // TODO? map the open windows to ones saved... somehow

    d->openNecessaryWindows(currentWindows);

    int index = 0;
    Q_FOREACH(const auto &window, d->windows) {
        currentWindows.at(index)->restoreGeometry(window.geometry);
        currentWindows.at(index)->restoreWorkspace(window.windowState);
        index++;
    }

    d->closeUnneededWindows(currentWindows);
}

bool KisWindowLayoutResource::save()
{
    if (filename().isEmpty())
        return false;

    QFile file(filename());
    file.open(QIODevice::WriteOnly);
    bool res = saveToDevice(&file);
    file.close();
    return res;
}

bool KisWindowLayoutResource::load()
{
    if (filename().isEmpty())
         return false;

    QFile file(filename());
    if (file.size() == 0) return false;
    if (!file.open(QIODevice::ReadOnly)) {
        warnKrita << "Can't open file " << filename();
        return false;
    }

    bool res = loadFromDevice(&file);
    file.close();
    return res;
}

bool KisWindowLayoutResource::saveToDevice(QIODevice *dev) const
{
    QDomDocument doc;
    QDomElement root = doc.createElement("WindowLayout");
    root.setAttribute("name", name());
    root.setAttribute("version", WINDOW_LAYOUT_VERSION);

    Q_FOREACH(const auto &window, d->windows) {
        QDomElement elem = doc.createElement("window");

        QDomElement geometry = doc.createElement("geometry");
        geometry.appendChild(doc.createCDATASection(window.geometry.toBase64()));
        elem.appendChild(geometry);

        QDomElement state = doc.createElement("windowState");
        state.appendChild(doc.createCDATASection(window.windowState.toBase64()));
        elem.appendChild(state);
        root.appendChild(elem);
    }

    doc.appendChild(root);

    QTextStream textStream(dev);
    textStream.setCodec("UTF-8");
    doc.save(textStream, 4);

    KoResource::saveToDevice(dev);

    return true;
}

bool KisWindowLayoutResource::loadFromDevice(QIODevice *dev)
{
    QDomDocument doc;
    if (!doc.setContent(dev)) {
        return false;
    }

    QDomElement element = doc.documentElement();
    setName(element.attribute("name"));

    d->windows.clear();

    for (auto windowElement = element.firstChildElement("window");
         !windowElement.isNull();
         windowElement = windowElement.nextSiblingElement()) {

        Private::Window window;

        QDomElement geometry = windowElement.firstChildElement("geometry");
        QDomElement state = windowElement.firstChildElement("windowState");

        window.geometry = QByteArray::fromBase64(geometry.text().toLatin1());
        window.windowState = QByteArray::fromBase64(state.text().toLatin1());

        d->windows.append(window);
    }

    setValid(true);
    return true;
}

QString KisWindowLayoutResource::defaultFileExtension() const
{
    return QString(".kwl");
}


