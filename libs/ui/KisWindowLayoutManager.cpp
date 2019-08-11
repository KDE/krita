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

#include "KisWindowLayoutManager.h"

#include <QWidget>
#include <QDesktopWidget>
#include <QScreen>

#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <KisApplication.h>
#include <KisMainWindow.h>
#include <KisPart.h>
#include <kis_dom_utils.h>
#include <KisResourceServerProvider.h>
#include <KisSessionResource.h>

Q_GLOBAL_STATIC(KisWindowLayoutManager, s_instance)

struct KisWindowLayoutManager::Private {
    bool showImageInAllWindows{false};
    bool primaryWorkspaceFollowsFocus{false};
    QUuid primaryWindow;

    QVector<DisplayLayout*> displayLayouts;

    void loadDisplayLayouts() {
        KConfigGroup layoutsCfg(KSharedConfig::openConfig(), "DisplayLayouts");
        QStringList groups = layoutsCfg.groupList();

        Q_FOREACH(QString name, groups) {
            loadDisplayLayout(name, layoutsCfg.group(name));
        }
    }

    void loadDisplayLayout(const QString &name, KConfigGroup layoutCfg) {
        DisplayLayout *layout = new DisplayLayout();
        layout->name = name;

        int displayNumber = 1;

        while (true) {
            const QString displayDefinition = layoutCfg.readEntry(QString("Display%1").arg(displayNumber++), QString());
            if (displayDefinition.isEmpty()) break;

            // Just the resolution for now. Later we might want to split by a separator and include things like serial number, etc.
            const QString &resolutionStr = displayDefinition;

            QStringList dimensions = resolutionStr.split('x');
            if (dimensions.size() != 2) {
                qWarning() << "Invalid display definition: " << displayDefinition;
                break;
            }

            QSize resolution = QSize(
                KisDomUtils::toInt(dimensions[0]),
                KisDomUtils::toInt(dimensions[1])
            );

            layout->displays.append(Display{resolution});
        }

        layout->preferredWindowLayout = layoutCfg.readEntry("PreferredLayout", "");

        displayLayouts.append(layout);
    }

    void saveDisplayLayout(const DisplayLayout &layout) {
        KConfigGroup layoutsCfg(KSharedConfig::openConfig(), "DisplayLayouts");
        KConfigGroup layoutCfg = layoutsCfg.group(layout.name);
        layoutCfg.writeEntry("PreferredLayout", layout.preferredWindowLayout);
    }
};

bool KisWindowLayoutManager::Display::matches(QScreen* screen) const
{
    return resolution == screen->geometry().size();
}

bool KisWindowLayoutManager::DisplayLayout::matches(QList<QScreen*> screens) const
{
    if (screens.size() != displays.size()) return false;

    QVector<bool> matchedScreens(screens.size());
    Q_FOREACH(auto &expectedDisplay, displays) {
        int i;
        for (i = 0; i < screens.size(); i++) {
            if (matchedScreens[i]) continue;

            if (expectedDisplay.matches(screens[i])) {
                matchedScreens[i] = true;
                break;
            }
        }

        if (i == screens.size()) {
            return false;
        }
    }

    return true;
}

KisWindowLayoutManager * KisWindowLayoutManager::instance()
{
    return s_instance;
}

KisWindowLayoutManager::KisWindowLayoutManager()
    : d(new Private)
{
    d->loadDisplayLayouts();

    connect(qobject_cast<KisApplication*>(KisApplication::instance()),
            SIGNAL(focusChanged(QWidget*,QWidget*)),
            this, SLOT(slotFocusChanged(QWidget*,QWidget*)));

    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(slotScreensChanged()));
    connect(QApplication::desktop(), SIGNAL(screenCountChanged(int)), this, SLOT(slotScreensChanged()));
}

KisWindowLayoutManager::~KisWindowLayoutManager() {
    Q_FOREACH(DisplayLayout *layout, d->displayLayouts) {
        delete layout;
    }
}

void KisWindowLayoutManager::setShowImageInAllWindowsEnabled(bool showInAll)
{
    bool wasEnabled = d->showImageInAllWindows;

    d->showImageInAllWindows = showInAll;

    if (!wasEnabled && showInAll) {
        KisMainWindow *currentMainWindow = KisPart::instance()->currentMainwindow();
        if (currentMainWindow) {
            KisView *activeView = currentMainWindow->activeView();
            if (activeView) {
                KisDocument *document = activeView->document();
                if (document) {
                   activeDocumentChanged(document);
                }
            }
        }
    }
}

bool KisWindowLayoutManager::isShowImageInAllWindowsEnabled() const
{
    return d->showImageInAllWindows;
}

bool KisWindowLayoutManager::primaryWorkspaceFollowsFocus() const
{
    return d->primaryWorkspaceFollowsFocus;
}

void KisWindowLayoutManager::setPrimaryWorkspaceFollowsFocus(bool enabled, QUuid primaryWindow)
{
    d->primaryWorkspaceFollowsFocus = enabled;
    d->primaryWindow = primaryWindow;
}

QUuid KisWindowLayoutManager::primaryWindowId() const
{
    return d->primaryWindow;
}

void KisWindowLayoutManager::activeDocumentChanged(KisDocument *document)
{
    if (d->showImageInAllWindows) {
        Q_FOREACH(QPointer<KisMainWindow> window, KisPart::instance()->mainWindows()) {
            if (window->isHidden()) continue;

            const auto view = window->activeView();
            if (!view || view->document() != document) {
                window->showDocument(document);
            }
        }
    }
}

void KisWindowLayoutManager::slotFocusChanged(QWidget *old, QWidget *now)
{
    Q_UNUSED(old);

    if (!now) return;
    KisMainWindow *newMainWindow = qobject_cast<KisMainWindow*>(now->window());
    if (!newMainWindow) return;

    newMainWindow->windowFocused();
}

void KisWindowLayoutManager::setLastUsedLayout(const KisWindowLayoutResource *layout)
{
    // For automatic switching, only allow a window layout proper
    auto *session = dynamic_cast<const KisSessionResource*>(layout);
    if (session) return;

    QList<QScreen*> screens = QGuiApplication::screens();
    Q_FOREACH(DisplayLayout *displayLayout, d->displayLayouts) {
        if (displayLayout->matches(screens)) {
            displayLayout->preferredWindowLayout = layout->name();
            d->saveDisplayLayout(*displayLayout);
            break;
        }
    }
}

void KisWindowLayoutManager::slotScreensChanged()
{
    QList<QScreen*> screens = QGuiApplication::screens();

    Q_FOREACH(const DisplayLayout *displayLayout, d->displayLayouts) {
        if (displayLayout->matches(screens)) {
            KoResourceServer<KisWindowLayoutResource> *windowLayoutServer = KisResourceServerProvider::instance()->windowLayoutServer();
            KisWindowLayoutResource *layout = windowLayoutServer->resourceByName(displayLayout->preferredWindowLayout);

            if (layout) {
                setLastUsedLayout(layout);
                layout->applyLayout();
                return;
            }
        }
    }
}
