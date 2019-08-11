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
#include "KisWindowLayoutManager.h"

#include <QVector>
#include <QList>
#include <QFile>
#include <QDomDocument>
#include <QApplication>
#include <QEventLoop>
#include <QWindow>
#include <QScreen>

#include <KisPart.h>
#include <KisDocument.h>
#include <kis_dom_utils.h>

static const int WINDOW_LAYOUT_VERSION = 1;

struct KisWindowLayoutResource::Private
{
    struct WindowGeometry{
        int screen = -1;
        Qt::WindowStates stateFlags = Qt::WindowNoState;
        QByteArray data;

        static WindowGeometry fromWindow(const QWidget *window, QList<QScreen*> screens)
        {
            WindowGeometry geometry;
            QWindow *windowHandle = window->windowHandle();

            geometry.data = window->saveGeometry();
            geometry.stateFlags = windowHandle->windowState();

            int index = screens.indexOf(windowHandle->screen());
            if (index >= 0) {
                geometry.screen = index;
            }

            return geometry;
        }

        void forceOntoCorrectScreen(QWidget *window, QList<QScreen*> screens)
        {
            QWindow *windowHandle = window->windowHandle();

            if (screens.indexOf(windowHandle->screen()) != screen) {
                QScreen *qScreen = screens[screen];
                windowHandle->setScreen(qScreen);
                windowHandle->setPosition(qScreen->availableGeometry().topLeft());
            }

            if (stateFlags) {
                window->setWindowState(stateFlags);
            }
        }

        void save(QDomDocument &doc, QDomElement &elem) const
        {
            if (screen >= 0) {
                elem.setAttribute("screen", screen);
            }

            if (stateFlags & Qt::WindowMaximized) {
                elem.setAttribute("maximized", "1");
            }

            QDomElement geometry = doc.createElement("geometry");
            geometry.appendChild(doc.createCDATASection(data.toBase64()));
            elem.appendChild(geometry);
        }

        static WindowGeometry load(const QDomElement &element)
        {
            WindowGeometry geometry;
            geometry.screen = element.attribute("screen", "-1").toInt();

            if (element.attribute("maximized", "0") != "0") {
                geometry.stateFlags |= Qt::WindowMaximized;
            }

            QDomElement dataElement = element.firstChildElement("geometry");
            geometry.data = QByteArray::fromBase64(dataElement.text().toLatin1());

            return geometry;
        }
    };

    struct Window {
        QUuid windowId;
        QByteArray windowState;
        WindowGeometry geometry;

        bool canvasDetached = false;
        WindowGeometry canvasWindowGeometry;
    };

    QVector<Window> windows;
    bool showImageInAllWindows;
    bool primaryWorkspaceFollowsFocus;
    QUuid primaryWindow;

    Private() = default;

    explicit Private(QVector<Window> windows)
        : windows(std::move(windows))
    {}

    void openNecessaryWindows(QList<QPointer<KisMainWindow>> &currentWindows) {
        auto *kisPart = KisPart::instance();

        Q_FOREACH(const Window &window, windows) {
            QPointer<KisMainWindow> mainWindow = kisPart->windowById(window.windowId);

            if (mainWindow.isNull()) {
                mainWindow = kisPart->createMainWindow(window.windowId);
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

                // Set the window hidden to prevent "show image in all windows" feature from opening new views on it
                // while we migrate views onto the remaining windows
                mainWindow->hide();
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


    QList<QScreen*> getScreensInConsistentOrder() {
        QList<QScreen*> screens = QGuiApplication::screens();

        std::sort(screens.begin(), screens.end(), [](const QScreen *a, const QScreen *b) {
            QRect aRect = a->geometry();
            QRect bRect = b->geometry();

            if (aRect.y() == bRect.y()) return aRect.x() < bRect.x();
            return (aRect.y() < aRect.y());
        });

        return screens;
    }
};

KisWindowLayoutResource::KisWindowLayoutResource(const QString &filename)
    : KoResource(filename)
    , d(new Private)
{}

KisWindowLayoutResource::~KisWindowLayoutResource()
{}

KisWindowLayoutResource * KisWindowLayoutResource::fromCurrentWindows(
    const QString &filename, const QList<QPointer<KisMainWindow>> &mainWindows, bool showImageInAllWindows,
    bool primaryWorkspaceFollowsFocus, KisMainWindow *primaryWindow
)
{
    auto resource = new KisWindowLayoutResource(filename);
    resource->setWindows(mainWindows);
    resource->d->showImageInAllWindows = showImageInAllWindows;
    resource->d->primaryWorkspaceFollowsFocus = primaryWorkspaceFollowsFocus;
    resource->d->primaryWindow = primaryWindow->id();
    return resource;
}

void KisWindowLayoutResource::applyLayout()
{
    auto *kisPart = KisPart::instance();
    auto *layoutManager= KisWindowLayoutManager::instance();

    layoutManager->setLastUsedLayout(this);

    QList<QPointer<KisMainWindow>> currentWindows = kisPart->mainWindows();

    if (d->windows.isEmpty()) {
        // No windows defined (e.g. fresh new session). Leave things as they are, but make sure there's at least one visible main window
        if (kisPart->mainwindowCount() == 0) {
            kisPart->createMainWindow();
        } else {
            kisPart->mainWindows().first()->show();
        }
    } else {
        d->openNecessaryWindows(currentWindows);
        d->closeUnneededWindows(currentWindows);
    }

    // Wait for the windows to finish opening / closing before applying saved geometry.
    // If we don't, the geometry may get reset after we apply it.
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    Q_FOREACH(const auto &window, d->windows) {
        QPointer<KisMainWindow> mainWindow = kisPart->windowById(window.windowId);
        KIS_SAFE_ASSERT_RECOVER_BREAK(mainWindow);

        mainWindow->restoreGeometry(window.geometry.data);
        mainWindow->restoreWorkspaceState(window.windowState);

        mainWindow->setCanvasDetached(window.canvasDetached);
        if (window.canvasDetached) {
            QWidget *canvasWindow = mainWindow->canvasWindow();
            canvasWindow->restoreGeometry(window.canvasWindowGeometry.data);
        }
    }

    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    QList<QScreen*> screens = d->getScreensInConsistentOrder();
    Q_FOREACH(const auto &window, d->windows) {
        Private::WindowGeometry geometry = window.geometry;
        QPointer<KisMainWindow> mainWindow = kisPart->windowById(window.windowId);
        KIS_SAFE_ASSERT_RECOVER_BREAK(mainWindow);

        if (geometry.screen >= 0 && geometry.screen < screens.size()) {
            geometry.forceOntoCorrectScreen(mainWindow, screens);
        }
        if (window.canvasDetached) {
            Private::WindowGeometry canvasWindowGeometry = window.canvasWindowGeometry;
            if (canvasWindowGeometry.screen >= 0 && canvasWindowGeometry.screen < screens.size()) {
                canvasWindowGeometry.forceOntoCorrectScreen(mainWindow->canvasWindow(), screens);
            }
        }
    }

    layoutManager->setShowImageInAllWindowsEnabled(d->showImageInAllWindows);
    layoutManager->setPrimaryWorkspaceFollowsFocus(d->primaryWorkspaceFollowsFocus, d->primaryWindow);
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
    root.setAttribute("showImageInAllWindows", (int)d->showImageInAllWindows);
    root.setAttribute("primaryWorkspaceFollowsFocus", (int)d->primaryWorkspaceFollowsFocus);
    root.setAttribute("primaryWindow", d->primaryWindow.toString());

    Q_FOREACH(const auto &window, d->windows) {
        QDomElement elem = doc.createElement("window");
        elem.setAttribute("id", window.windowId.toString());

        window.geometry.save(doc, elem);

        if (window.canvasDetached) {
            QDomElement canvasWindowElement = doc.createElement("canvasWindow");
            window.canvasWindowGeometry.save(doc, canvasWindowElement);
            elem.appendChild(canvasWindowElement);
        }

        QDomElement state = doc.createElement("windowState");
        state.appendChild(doc.createCDATASection(window.windowState.toBase64()));
        elem.appendChild(state);
        root.appendChild(elem);
    }
}

void KisWindowLayoutResource::loadXml(const QDomElement &element) const
{
    d->showImageInAllWindows = KisDomUtils::toInt(element.attribute("showImageInAllWindows", "0"));
    d->primaryWorkspaceFollowsFocus = KisDomUtils::toInt(element.attribute("primaryWorkspaceFollowsFocus", "0"));
    d->primaryWindow = element.attribute("primaryWindow");

    for (auto windowElement = element.firstChildElement("window");
         !windowElement.isNull();
         windowElement = windowElement.nextSiblingElement("window")) {

        Private::Window window;

        window.windowId = QUuid(windowElement.attribute("id", QUuid().toString()));
        if (window.windowId.isNull()) {
            window.windowId = QUuid::createUuid();
        }

        window.geometry = Private::WindowGeometry::load(windowElement);

        QDomElement canvasWindowElement = windowElement.firstChildElement("canvasWindow");
        if (!canvasWindowElement.isNull()) {
            window.canvasDetached = true;
            window.canvasWindowGeometry = Private::WindowGeometry::load(canvasWindowElement);
        }

        QDomElement state = windowElement.firstChildElement("windowState");
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

    QList<QScreen*> screens = d->getScreensInConsistentOrder();

    Q_FOREACH(auto window, mainWindows) {
        if (!window->isVisible()) continue;

        Private::Window state;
        state.windowId = window->id();
        state.windowState = window->saveState();
        state.geometry = Private::WindowGeometry::fromWindow(window, screens);

        state.canvasDetached = window->canvasDetached();
        if (state.canvasDetached) {
            state.canvasWindowGeometry = Private::WindowGeometry::fromWindow(window->canvasWindow(), screens);
        }

        d->windows.append(state);
    }
}
