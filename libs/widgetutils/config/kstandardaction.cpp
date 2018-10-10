/* This file is part of the KDE libraries
   Copyright (C) 1999,2000 Kurt Granroth <granroth@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kstandardaction.h"
#include "kstandardaction_p.h"
#include "moc_kstandardaction_p.cpp"

#include <QMutableStringListIterator>
#include <QToolButton>

#include <QApplication>
#include <klocalizedstring.h>
#include <kstandardshortcut.h>
#include <kacceleratormanager.h>

#include "kdualaction.h"
#include "krecentfilesaction.h"
#include "ktogglefullscreenaction.h"

#include <kis_icon_utils.h>

namespace KStandardAction
{
AutomaticAction::AutomaticAction(const QIcon &icon, const QString &text, const QList<QKeySequence> &shortcut, const char *slot,
                                 QObject *parent)
    : QAction(parent)
{
    setText(text);
    setIcon(icon);
    setShortcuts(shortcut);
    setProperty("defaultShortcuts", QVariant::fromValue(shortcut));
    connect(this, SIGNAL(triggered()), this, slot);
}

QStringList stdNames()
{
    return internal_stdNames();
}

QList<StandardAction> actionIds()
{
    QList<StandardAction> result;

    for (uint i = 0; g_rgActionInfo[i].id != ActionNone; i++) {
        result.append(g_rgActionInfo[i].id);
    }

    return result;
}

KRITAWIDGETUTILS_EXPORT KStandardShortcut::StandardShortcut shortcutForActionId(StandardAction id)
{
    const KStandardActionInfo *pInfo = infoPtr(id);
    return (pInfo) ? pInfo->idAccel : KStandardShortcut::AccelNone;
}

QAction *create(StandardAction id, const QObject *recvr, const char *slot, QObject *parent)
{
    static bool stdNamesInitialized = false;

    if (!stdNamesInitialized) {
        KAcceleratorManager::addStandardActionNames(stdNames());
        stdNamesInitialized = true;
    }

    QAction *pAction = 0;
    const KStandardActionInfo *pInfo = infoPtr(id);

    // qDebug() << "KStandardAction::create( " << id << "=" << (pInfo ? pInfo->psName : (const char*)0) << ", " << parent << " )"; // ellis

    if (pInfo) {
        QString sLabel, iconName = pInfo->psIconName;
        switch (id) {
        case Back:
            sLabel = i18nc("go back", "&Back");
            if (QApplication::isRightToLeft()) {
                iconName = "go-next";
            }
            break;

        case Forward:
            sLabel = i18nc("go forward", "&Forward");
            if (QApplication::isRightToLeft()) {
                iconName = "go-previous";
            }
            break;

        case Home:
            sLabel = i18nc("home page", "&Home");
            break;
        case Help:
            sLabel = i18nc("show help", "&Help");
            break;
        case Preferences:
        case AboutApp:
        case HelpContents: {
            QString appDisplayName = QGuiApplication::applicationDisplayName();
            if (appDisplayName.isEmpty()) {
                appDisplayName = QCoreApplication::applicationName();
            }
            sLabel = i18n(pInfo->psLabel, appDisplayName);
        }
        break;
        default:
            sLabel = i18n(pInfo->psLabel);
        }

        if (QApplication::isRightToLeft()) {
            switch (id) {
            case Prior:           iconName = "go-next-view-page"; break;
            case Next:            iconName = "go-previous-view-page"; break;
            case FirstPage:       iconName = "go-last-view-page"; break;
            case LastPage:        iconName = "go-first-view-page"; break;
            case DocumentBack:    iconName = "go-next"; break;
            case DocumentForward: iconName = "go-previous"; break;
            default: break;
            }
        }

        QIcon icon = iconName.isEmpty() ? QIcon() : KisIconUtils::loadIcon(iconName);

        switch (id) {
        case OpenRecent:
            pAction = new KRecentFilesAction(parent);
            break;
        case ShowMenubar:
        case ShowToolbar:
        case ShowStatusbar:
            pAction = new QAction(parent);
            pAction->setCheckable(true);
            pAction->setChecked(true);
            break;
        case FullScreen:
            pAction = new KToggleFullScreenAction(parent);
            pAction->setCheckable(true);
            break;
        case PasteText:
            pAction = new QAction(parent);
            break;
        // Same as default, but with the app icon
        case AboutApp:
        {
            pAction = new QAction(parent);
            icon = qApp->windowIcon();
            break;
        }

        default:
            pAction = new QAction(parent);
            break;
        }

        switch (id) {
        case Quit:
            pAction->setMenuRole(QAction::QuitRole);
            break;

        case Preferences:
            pAction->setMenuRole(QAction::PreferencesRole);
            break;

        case AboutApp:
            pAction->setMenuRole(QAction::AboutRole);
            break;

        default:
            pAction->setMenuRole(QAction::NoRole);
            break;
        }

        pAction->setText(sLabel);
        if (pInfo->psToolTip) {
            pAction->setToolTip(i18n(pInfo->psToolTip));
        }
        pAction->setIcon(icon);

        QList<QKeySequence> cut = KStandardShortcut::shortcut(pInfo->idAccel);
        if (!cut.isEmpty()) {
            // emulate KActionCollection::setDefaultShortcuts to allow the use of "configure shortcuts"
            pAction->setShortcuts(cut);
            pAction->setProperty("defaultShortcuts", QVariant::fromValue(cut));
        }

        pAction->setObjectName(pInfo->psName);
    }

    if (recvr && slot) {
        if (id == OpenRecent) {
            // FIXME QAction port: probably a good idea to find a cleaner way to do this
            // Open Recent is a special case - provide the selected URL
            QObject::connect(pAction, SIGNAL(urlSelected(QUrl)), recvr, slot);
        } else if (id == ConfigureToolbars) { // #200815
            QObject::connect(pAction, SIGNAL(triggered(bool)), recvr, slot, Qt::QueuedConnection);
        } else {
            QObject::connect(pAction, SIGNAL(triggered(bool)), recvr, slot);
        }
    }

    if (pAction && parent && parent->inherits("KActionCollection")) {
        QMetaObject::invokeMethod(parent, "addAction", Q_ARG(QString, pAction->objectName()), Q_ARG(QAction *, pAction));
    }

    return pAction;
}

const char *name(StandardAction id)
{
    const KStandardActionInfo *pInfo = infoPtr(id);
    return (pInfo) ? pInfo->psName : 0;
}

QAction *openNew(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(New, recvr, slot, parent);
}

QAction *open(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Open, recvr, slot, parent);
}

KRecentFilesAction *openRecent(const QObject *recvr, const char *slot, QObject *parent)
{
    return (KRecentFilesAction *) KStandardAction::create(OpenRecent, recvr, slot, parent);
}

QAction *save(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Save, recvr, slot, parent);
}

QAction *saveAs(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(SaveAs, recvr, slot, parent);
}

QAction *revert(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Revert, recvr, slot, parent);
}

QAction *print(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Print, recvr, slot, parent);
}

QAction *printPreview(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(PrintPreview, recvr, slot, parent);
}

QAction *close(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Close, recvr, slot, parent);
}

QAction *mail(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Mail, recvr, slot, parent);
}

QAction *quit(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Quit, recvr, slot, parent);
}

QAction *undo(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Undo, recvr, slot, parent);
}

QAction *redo(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Redo, recvr, slot, parent);
}

QAction *cut(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Cut, recvr, slot, parent);
}

QAction *copy(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Copy, recvr, slot, parent);
}

QAction *paste(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Paste, recvr, slot, parent);
}

QAction *pasteText(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(PasteText, recvr, slot, parent);
}

QAction *clear(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Clear, recvr, slot, parent);
}

QAction *selectAll(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(SelectAll, recvr, slot, parent);
}

QAction *deselect(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Deselect, recvr, slot, parent);
}

QAction *find(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Find, recvr, slot, parent);
}

QAction *findNext(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(FindNext, recvr, slot, parent);
}

QAction *findPrev(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(FindPrev, recvr, slot, parent);
}

QAction *replace(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Replace, recvr, slot, parent);
}

QAction *actualSize(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(ActualSize, recvr, slot, parent);
}

QAction *fitToPage(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(FitToPage, recvr, slot, parent);
}

QAction *fitToWidth(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(FitToWidth, recvr, slot, parent);
}

QAction *fitToHeight(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(FitToHeight, recvr, slot, parent);
}

QAction *zoomIn(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(ZoomIn, recvr, slot, parent);
}

QAction *zoomOut(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(ZoomOut, recvr, slot, parent);
}

QAction *zoom(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Zoom, recvr, slot, parent);
}

QAction *redisplay(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Redisplay, recvr, slot, parent);
}

QAction *up(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Up, recvr, slot, parent);
}

QAction *back(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Back, recvr, slot, parent);
}

QAction *forward(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Forward, recvr, slot, parent);
}

QAction *home(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Home, recvr, slot, parent);
}

QAction *prior(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Prior, recvr, slot, parent);
}

QAction *next(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Next, recvr, slot, parent);
}

QAction *goTo(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Goto, recvr, slot, parent);
}

QAction *gotoPage(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(GotoPage, recvr, slot, parent);
}

QAction *gotoLine(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(GotoLine, recvr, slot, parent);
}

QAction *firstPage(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(FirstPage, recvr, slot, parent);
}

QAction *lastPage(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(LastPage, recvr, slot, parent);
}

QAction *documentBack(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(DocumentBack, recvr, slot, parent);
}

QAction *documentForward(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(DocumentForward, recvr, slot, parent);
}

QAction *addBookmark(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(AddBookmark, recvr, slot, parent);
}

QAction *editBookmarks(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(EditBookmarks, recvr, slot, parent);
}

QAction *spelling(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Spelling, recvr, slot, parent);
}

static QAction *buildAutomaticAction(QObject *parent, StandardAction id, const char *slot)
{
    const KStandardActionInfo *p = infoPtr(id);
    if (!p) {
        return 0;
    }

    AutomaticAction *action = new AutomaticAction(
        KisIconUtils::loadIcon(p->psIconName),
        i18n(p->psLabel),
        KStandardShortcut::shortcut(p->idAccel),
        slot,
        parent);

    action->setObjectName(p->psName);
    if (p->psToolTip) {
        action->setToolTip(i18n(p->psToolTip));
    }

    if (parent && parent->inherits("KActionCollection")) {
        QMetaObject::invokeMethod(parent, "addAction", Q_ARG(QString, action->objectName()), Q_ARG(QAction *, action));
    }

    return action;
}

QAction *cut(QObject *parent)
{
    return buildAutomaticAction(parent, Cut, SLOT(cut()));
}

QAction *copy(QObject *parent)
{
    return buildAutomaticAction(parent, Copy, SLOT(copy()));
}

QAction *paste(QObject *parent)
{
    return buildAutomaticAction(parent, Paste, SLOT(paste()));
}

QAction *clear(QObject *parent)
{
    return buildAutomaticAction(parent, Clear, SLOT(clear()));
}

QAction *selectAll(QObject *parent)
{
    return buildAutomaticAction(parent, SelectAll, SLOT(selectAll()));
}

KToggleAction *showMenubar(const QObject *recvr, const char *slot, QObject *parent)
{
    KToggleAction *ret = new KToggleAction(i18n("Show &Menubar"), parent);
    ret->setObjectName(name(ShowMenubar));
    ret->setIcon(KisIconUtils::loadIcon("show-menu"));

    // emulate KActionCollection::setDefaultShortcuts to allow the use of "configure shortcuts"
// This shortcut is dangerous and should not be enabled by default.
//    ret->setShortcuts(KStandardShortcut::shortcut(KStandardShortcut::ShowMenubar));
//    ret->setProperty("defaultShortcuts", QVariant::fromValue(KStandardShortcut::shortcut(KStandardShortcut::ShowMenubar)));

    ret->setWhatsThis(i18n("Show Menubar<p>"
                           "Shows the menubar again after it has been hidden</p>"));

    ret->setChecked(true);

    if (recvr && slot) {
        QObject::connect(ret, SIGNAL(triggered(bool)), recvr, slot);
    }

    if (parent && parent->inherits("KActionCollection")) {
        QMetaObject::invokeMethod(parent, "addAction", Q_ARG(QString, ret->objectName()), Q_ARG(QAction *, ret));
    }

    return ret;
}

KToggleAction *showStatusbar(const QObject *recvr, const char *slot, QObject *parent)
{
    KToggleAction *ret = new KToggleAction(i18n("Show St&atusbar"), parent);
    ret->setObjectName(name(ShowStatusbar));

    ret->setWhatsThis(i18n("Show Statusbar<p>"
                           "Shows the statusbar, which is the bar at the bottom of the window used for status information.</p>"));

    ret->setChecked(true);

    if (recvr && slot) {
        QObject::connect(ret, SIGNAL(triggered(bool)), recvr, slot);
    }

    if (parent && parent->inherits("KActionCollection")) {
        QMetaObject::invokeMethod(parent, "addAction", Q_ARG(QString, ret->objectName()), Q_ARG(QAction *, ret));
    }

    return ret;
}

KToggleFullScreenAction *fullScreen(const QObject *recvr, const char *slot, QWidget *window, QObject *parent)
{
    KToggleFullScreenAction *ret;
    ret = static_cast< KToggleFullScreenAction * >(KStandardAction::create(FullScreen, recvr, slot, parent));
    ret->setWindow(window);

    return ret;
}

QAction *saveOptions(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(SaveOptions, recvr, slot, parent);
}

QAction *keyBindings(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(KeyBindings, recvr, slot, parent);
}

QAction *preferences(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Preferences, recvr, slot, parent);
}

QAction *configureToolbars(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(ConfigureToolbars, recvr, slot, parent);
}

QAction *configureNotifications(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(ConfigureNotifications, recvr, slot, parent);
}

QAction *help(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(Help, recvr, slot, parent);
}

QAction *helpContents(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(HelpContents, recvr, slot, parent);
}

QAction *whatsThis(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(WhatsThis, recvr, slot, parent);
}

QAction *tipOfDay(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(TipofDay, recvr, slot, parent);
}

QAction *reportBug(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(ReportBug, recvr, slot, parent);
}

QAction *switchApplicationLanguage(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(SwitchApplicationLanguage, recvr, slot, parent);
}

QAction *aboutApp(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(AboutApp, recvr, slot, parent);
}

QAction *aboutKDE(const QObject *recvr, const char *slot, QObject *parent)
{
    return KStandardAction::create(AboutKDE, recvr, slot, parent);
}

}

