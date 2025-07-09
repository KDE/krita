/*
 * This file is part of the KDE Libraries
 * SPDX-FileCopyrightText: 1999-2000 Espen Sand (espen@kde.org)
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 */

// I (espen) prefer that header files are included alphabetically

#include "khelpmenu.h"
#include "config-xmlgui.h"
#include <QTimer>
#include <QAction>
#include <QApplication>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMenu>
#include <QStyle>
#include <QWidget>
#include <QWhatsThis>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QBoxLayout>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QDebug>

#include "kaboutkdedialog_p.h"
#include "kbugreport.h"
#include "kswitchlanguagedialog_p.h"

#include <kaboutdata.h>
#include <klocalizedstring.h>
#include <kstandardaction.h>

#ifdef Q_OS_MACOS
#include "libs/macosutils/KisMacosEntitlements.h"
#endif

using namespace KDEPrivate;

class KisKHelpMenuPrivate
{
public:
    KisKHelpMenuPrivate()
        : mAboutData(KAboutData::applicationData())
    {
    }
    ~KisKHelpMenuPrivate()
    {
        delete mMenu;
        delete mAboutApp;
        delete mAboutKDE;
        delete mBugReport;
        delete mSwitchApplicationLanguage;
    }

    void createActions(KisKHelpMenu *q);

    QMenu *mMenu {nullptr};
    QDialog *mAboutApp {nullptr};
    KisKAboutKdeDialog *mAboutKDE {nullptr};
    KisKBugReport *mBugReport {nullptr};
    KisKSwitchLanguageDialog *mSwitchApplicationLanguage {nullptr};
    // TODO evaluate if we use static_cast<QWidget*>(parent()) instead of mParent to win that bit of memory
    QWidget *mParent {nullptr};
    QString mAboutAppText;

    bool mShowWhatsThis {false};
    bool mActionsCreated {false};

    QAction *mHandBookAction {nullptr};
    QAction *mWhatsThisAction {nullptr};
    QAction *mReportBugAction {nullptr};
    QAction *mSwitchApplicationLanguageAction {nullptr};
    QAction *mAboutAppAction {nullptr};
    QAction *mAboutKDEAction {nullptr};

    KAboutData mAboutData;
};

KisKHelpMenu::KisKHelpMenu(QWidget *parent, const QString &aboutAppText,
                     bool showWhatsThis)
    : QObject(parent), d(new KisKHelpMenuPrivate)
{
    d->mAboutAppText = aboutAppText;
    d->mShowWhatsThis = showWhatsThis;
    d->mParent = parent;
    d->createActions(this);
}

KisKHelpMenu::KisKHelpMenu(QWidget *parent, const KAboutData &aboutData,
                     bool showWhatsThis)
    : QObject(parent), d(new KisKHelpMenuPrivate)
{
    d->mShowWhatsThis = showWhatsThis;
    d->mParent = parent;
    d->mAboutData = aboutData;
    d->createActions(this);
}

KisKHelpMenu::~KisKHelpMenu()
{
    delete d;
}

void KisKHelpMenuPrivate::createActions(KisKHelpMenu *q)
{
    if (mActionsCreated) {
        return;
    }
    mActionsCreated = true;
    mHandBookAction = KStandardAction::helpContents(q, SLOT(appHelpActivated()), q);
    if (mShowWhatsThis) {
        mWhatsThisAction = KStandardAction::whatsThis(q, SLOT(contextHelpActivated()), q);
    }

    if (!mAboutData.bugAddress().isEmpty()) {
        mReportBugAction = KStandardAction::reportBug(q, SLOT(reportBug()), q);
    }

    mSwitchApplicationLanguageAction = KStandardAction::create(KStandardAction::SwitchApplicationLanguage, q, SLOT(switchApplicationLanguage()), q);
    mAboutAppAction = KStandardAction::aboutApp(q, SLOT(aboutApplication()), q);
    mAboutKDEAction = KStandardAction::aboutKDE(q, SLOT(aboutKDE()), q);
}

// Used in the non-xml-gui case, like kfind or ksnapshot's help button.
QMenu *KisKHelpMenu::menu()
{
    if (!d->mMenu) {
        d->mMenu = new QMenu();
        connect(d->mMenu, SIGNAL(destroyed()), this, SLOT(menuDestroyed()));

        d->mMenu->setTitle(i18n("&Help"));

        d->createActions(this);

        bool need_separator = false;
        if (d->mHandBookAction) {
            d->mMenu->addAction(d->mHandBookAction);
            need_separator = true;
        }

        if (d->mWhatsThisAction) {
            d->mMenu->addAction(d->mWhatsThisAction);
            need_separator = true;
        }

        if (d->mReportBugAction) {
            if (need_separator) {
                d->mMenu->addSeparator();
            }
            d->mMenu->addAction(d->mReportBugAction);
            need_separator = true;
        }

        if (d->mSwitchApplicationLanguageAction) {
            if (need_separator) {
                d->mMenu->addSeparator();
            }
            d->mMenu->addAction(d->mSwitchApplicationLanguageAction);
            need_separator = true;
        }

        if (need_separator) {
            d->mMenu->addSeparator();
        }

        if (d->mAboutAppAction) {
            d->mMenu->addAction(d->mAboutAppAction);
        }

        if (d->mAboutKDEAction) {
            d->mMenu->addAction(d->mAboutKDEAction);
        }
    }

    return d->mMenu;
}

QAction *KisKHelpMenu::action(MenuId id) const
{
    switch (id) {
    case menuHelpContents:
        return d->mHandBookAction;
        break;

    case menuWhatsThis:
        return d->mWhatsThisAction;
        break;

    case menuReportBug:
        return d->mReportBugAction;
        break;

    case menuSwitchLanguage:
        return d->mSwitchApplicationLanguageAction;
        break;

    case menuAboutApp:
        return d->mAboutAppAction;
        break;

    case menuAboutKDE:
#ifdef Q_OS_MACOS
        // MACOS store version should not contain external links to donations
        // support KDE tab has been seen as a call to action.
        if (!KisMacosEntitlements().sandbox()) {
#endif
        return d->mAboutKDEAction;
#ifdef Q_OS_MACOS
        }
#endif
        break;
    }

    return 0;
}

void KisKHelpMenu::appHelpActivated()
{
    QDesktopServices::openUrl(QUrl(QStringLiteral("help:/")));
}

void KisKHelpMenu::aboutApplication()
{
    if (receivers(SIGNAL(showAboutApplication())) > 0) {
        Q_EMIT showAboutApplication();
    }
}

void KisKHelpMenu::aboutKDE()
{
    if (!d->mAboutKDE) {
        d->mAboutKDE = new KisKAboutKdeDialog(d->mParent);
        connect(d->mAboutKDE, SIGNAL(finished(int)), this, SLOT(dialogFinished()));
    }
    d->mAboutKDE->show();
}

void KisKHelpMenu::reportBug()
{
#ifdef KRITA_STABLE
    QDesktopServices::openUrl(QUrl("https://docs.krita.org/en/untranslatable_pages/reporting_bugs.html"));
#else
    if (!d->mBugReport) {
        d->mBugReport = new KisKBugReport(d->mAboutData, d->mParent);
        connect(d->mBugReport, SIGNAL(finished(int)), this, SLOT(dialogFinished()));
    }
    d->mBugReport->show();
#endif
}

void KisKHelpMenu::switchApplicationLanguage()
{
    if (!d->mSwitchApplicationLanguage) {
        d->mSwitchApplicationLanguage = new KisKSwitchLanguageDialog(d->mParent);
        connect(d->mSwitchApplicationLanguage, SIGNAL(finished(int)), this, SLOT(dialogFinished()));
    }
    d->mSwitchApplicationLanguage->show();
}

void KisKHelpMenu::dialogFinished()
{
    QTimer::singleShot(0, this, SLOT(timerExpired()));
}

void KisKHelpMenu::timerExpired()
{
    if (d->mAboutKDE && !d->mAboutKDE->isVisible()) {
        delete d->mAboutKDE; d->mAboutKDE = 0;
    }

    if (d->mBugReport && !d->mBugReport->isVisible()) {
        delete d->mBugReport; d->mBugReport = 0;
    }
    if (d->mSwitchApplicationLanguage && !d->mSwitchApplicationLanguage->isVisible()) {
        delete d->mSwitchApplicationLanguage; d->mSwitchApplicationLanguage = 0;
    }
    if (d->mAboutApp && !d->mAboutApp->isVisible()) {
        delete d->mAboutApp; d->mAboutApp = 0;
    }
}

void KisKHelpMenu::menuDestroyed()
{
    d->mMenu = 0;
}

void KisKHelpMenu::contextHelpActivated()
{
    QWhatsThis::enterWhatsThisMode();
}

