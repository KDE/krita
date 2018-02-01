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
        QUuid windowId;
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

        Q_FOREACH(const Window &window, windows) {
            bool isOpen = false;
            Q_FOREACH(QPointer<KisMainWindow> mainWindow, currentWindows) {
                if (mainWindow->id() == window.windowId) {
                    isOpen = true;
                    break;
                }
            }

            if (!isOpen) {
                KisMainWindow *mainWindow = kisPart->createMainWindow(window.windowId);
                currentWindows.append(mainWindow);
                mainWindow->show();
            }
        }
    }

    void closeUnneededWindows(QList<QPointer<KisMainWindow>> &currentWindows) {
        QVector<QPointer<KisMainWindow>> windowsToClose;

        Q_FOREACH(KisMainWindow *mainWindow, currentWindows) {
            bool keep = false;
            Q_FOREACH(const Window &window, windows) {
                if (window.windowId == mainWindow->id()) {
                    keep = true;
                    break;
                }
            }

            if (!keep) {
                windowsToClose.append(mainWindow);
            }
        }

        migrateViewsFromClosingWindows(windowsToClose);

        Q_FOREACH(QPointer<KisMainWindow> mainWindow, windowsToClose) {
            mainWindow->close();
        }
    }

    void migrateViewsFromClosingWindows(QVector<QPointer<KisMainWindow>> &closingWindows) const
    {
        auto *kisPart = KisPart::instance();
        KisMainWindow *migrationTarget = nullptr;

        Q_FOREACH(KisMainWindow *mainWindow, kisPart->mainWindows()) {
            if (!closingWindows.contains(mainWindow)) {
                migrationTarget = mainWindow;
                break;
            }
        }

        if (!migrationTarget) {
            qWarning() << "Problem: window layout with no windows would leave user with zero main windows.";
            migrationTarget = closingWindows.takeLast();
            migrationTarget->show();
        }

        QVector<KisDocument*> visibleDocuments;
        Q_FOREACH(KisView *view, kisPart->views()) {
            KisMainWindow *window = view->mainWindow();
            if (!closingWindows.contains(window)) {
                visibleDocuments.append(view->document());
            }
        }

        Q_FOREACH(KisDocument *document, kisPart->documents()) {
            if (!visibleDocuments.contains(document)) {
                visibleDocuments.append(document);
                migrationTarget->newView(document);
            }
        }
    }
};

KisWindowLayoutResource::KisWindowLayoutResource(const QString &filename)
    : KoResource(filename)
    , d(new Private)
{}

KisWindowLayoutResource::~KisWindowLayoutResource()
{}

KisWindowLayoutResource * KisWindowLayoutResource::fromCurrentWindows(const QString &filename, const QList<QPointer<KisMainWindow>> &mainWindows)
{
    auto resource = new KisWindowLayoutResource(filename);
    resource->setWindows(mainWindows);
    return resource;
}

void KisWindowLayoutResource::applyLayout()
{
    KisPart *kisPart = KisPart::instance();
    QList<QPointer<KisMainWindow>> currentWindows = kisPart->mainWindows();

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

    saveXml(doc, root);

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

    loadXml(element);

    setValid(true);
    return true;
}

void KisWindowLayoutResource::saveXml(QDomDocument &doc, QDomElement &root) const
{
    Q_FOREACH(const auto &window, d->windows) {
        QDomElement elem = doc.createElement("window");
        elem.setAttribute("id", window.windowId.toString());

        QDomElement geometry = doc.createElement("geometry");
        geometry.appendChild(doc.createCDATASection(window.geometry.toBase64()));
        elem.appendChild(geometry);

        QDomElement state = doc.createElement("windowState");
        state.appendChild(doc.createCDATASection(window.windowState.toBase64()));
        elem.appendChild(state);
        root.appendChild(elem);
    }
}

void KisWindowLayoutResource::loadXml(const QDomElement &element) const
{
    for (auto windowElement = element.firstChildElement("window");
         !windowElement.isNull();
         windowElement = windowElement.nextSiblingElement("window")) {

        Private::Window window;

        window.windowId = QUuid(windowElement.attribute("id", QUuid().toString()));
        if (window.windowId.isNull()) {
            window.windowId = QUuid::createUuid();
        }

        QDomElement geometry = windowElement.firstChildElement("geometry");
        QDomElement state = windowElement.firstChildElement("windowState");

        window.geometry = QByteArray::fromBase64(geometry.text().toLatin1());
        window.windowState = QByteArray::fromBase64(state.text().toLatin1());

        d->windows.append(window);
    }
}

QString KisWindowLayoutResource::defaultFileExtension() const
{
    return QString(".kwl");
}

void KisWindowLayoutResource::setWindows(const QList<QPointer<KisMainWindow>> &mainWindows)
{
    d->windows.clear();

    Q_FOREACH(auto window, mainWindows) {
        Private::Window state;
        state.windowId = window->id();
        state.geometry = window->saveGeometry();
        state.windowState = window->saveState();

        d->windows.append(state);
    }
}


