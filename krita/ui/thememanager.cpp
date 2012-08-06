/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : theme manager
 *
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "thememanager.moc"

// Qt includes

#include <QStringList>
#include <QFileInfo>
#include <QFile>
#include <QApplication>
#include <QPalette>
#include <QColor>
#include <QActionGroup>
#include <QBitmap>
#include <QPainter>
#include <QPixmap>
#include <QDate>
#include <QDesktopWidget>

// KDE includes

#include <kmenu.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcolorscheme.h>
#include <kactioncollection.h>
#include <kstandarddirs.h>
#include <kactionmenu.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <kxmlguiwindow.h>
#include <ktoolinvocation.h>

// Calligra
#include <KoIcon.h>

namespace Digikam
{

// ---------------------------------------------------------------


class ThemeManager::ThemeManagerPriv
{
public:

    ThemeManagerPriv()
        : defaultThemeName(i18nc("default theme name", "Default")),
          themeMenuActionGroup(0),
          themeMenuAction(0)
    {
    }

    const QString          defaultThemeName;
    QMap<QString, QString> themeMap;            // map<theme name, theme config path>

    QActionGroup*          themeMenuActionGroup;
    KActionMenu*           themeMenuAction;
};

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
    , d(new ThemeManagerPriv)
{
    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
            this, SLOT(slotSettingsChanged()));
}

ThemeManager::~ThemeManager()
{
    delete d;
}

QString ThemeManager::defaultThemeName() const
{
    return d->defaultThemeName;
}

QString ThemeManager::currentThemeName() const
{
    if (!d->themeMenuAction || !d->themeMenuActionGroup) return defaultThemeName();
    qDebug() << d->themeMenuActionGroup;
    QAction* action = d->themeMenuActionGroup->checkedAction();
    return !action ? defaultThemeName() : action->text().remove('&');
}

void ThemeManager::setCurrentTheme(const QString& name)
{
    if (!d->themeMenuAction || !d->themeMenuActionGroup) return;
    QList<QAction*> list = d->themeMenuActionGroup->actions();
    foreach(QAction* action, list)
    {
        if (action->text().remove('&') == name)
        {
            action->setChecked(true);
            slotChangePalette();
        }
    }
}

void ThemeManager::slotChangePalette()
{
    updateCurrentKDEdefaultThemePreview();

    QString theme(currentThemeName());

    if (theme == defaultThemeName() || theme.isEmpty())
        theme = currentKDEdefaultTheme();

    kDebug() << theme;

    QString filename        = d->themeMap.value(theme);
    KSharedConfigPtr config = KSharedConfig::openConfig(filename);

    /*
    TODO: with recent KDE4 api, we can use KGlobalSettings::createNewApplicationPalette()
    d->palette = KGlobalSettings::createNewApplicationPalette(config);
    */

    QPalette palette               = kapp->palette();
    QPalette::ColorGroup states[3] = { QPalette::Active, QPalette::Inactive, QPalette::Disabled };
    kDebug() << filename;
    // TT thinks tooltips shouldn't use active, so we use our active colors for all states
    KColorScheme schemeTooltip(QPalette::Active, KColorScheme::Tooltip, config);

    for ( int i = 0; i < 3 ; ++i )
    {
        QPalette::ColorGroup state = states[i];
        KColorScheme schemeView(state,      KColorScheme::View,      config);
        KColorScheme schemeWindow(state,    KColorScheme::Window,    config);
        KColorScheme schemeButton(state,    KColorScheme::Button,    config);
        KColorScheme schemeSelection(state, KColorScheme::Selection, config);

        palette.setBrush(state, QPalette::WindowText,      schemeWindow.foreground());
        palette.setBrush(state, QPalette::Window,          schemeWindow.background());
        palette.setBrush(state, QPalette::Base,            schemeView.background());
        palette.setBrush(state, QPalette::Text,            schemeView.foreground());
        palette.setBrush(state, QPalette::Button,          schemeButton.background());
        palette.setBrush(state, QPalette::ButtonText,      schemeButton.foreground());
        palette.setBrush(state, QPalette::Highlight,       schemeSelection.background());
        palette.setBrush(state, QPalette::HighlightedText, schemeSelection.foreground());
        palette.setBrush(state, QPalette::ToolTipBase,     schemeTooltip.background());
        palette.setBrush(state, QPalette::ToolTipText,     schemeTooltip.foreground());

        palette.setColor(state, QPalette::Light,           schemeWindow.shade(KColorScheme::LightShade));
        palette.setColor(state, QPalette::Midlight,        schemeWindow.shade(KColorScheme::MidlightShade));
        palette.setColor(state, QPalette::Mid,             schemeWindow.shade(KColorScheme::MidShade));
        palette.setColor(state, QPalette::Dark,            schemeWindow.shade(KColorScheme::DarkShade));
        palette.setColor(state, QPalette::Shadow,          schemeWindow.shade(KColorScheme::ShadowShade));

        palette.setBrush(state, QPalette::AlternateBase,   schemeView.background(KColorScheme::AlternateBackground));
        palette.setBrush(state, QPalette::Link,            schemeView.foreground(KColorScheme::LinkText));
        palette.setBrush(state, QPalette::LinkVisited,     schemeView.foreground(KColorScheme::VisitedText));
    }

    kapp->setPalette(palette);
    emit signalThemeChanged();
}

void ThemeManager::setThemeMenuAction(KActionMenu* const action)
{
    d->themeMenuAction = action;
    populateThemeMenu();
}

void ThemeManager::registerThemeActions(KActionCollection *actionCollection)
{
    if (!d->themeMenuAction) return;
    actionCollection->addAction("theme_menu", d->themeMenuAction);
}

void ThemeManager::populateThemeMenu()
{
    if (!d->themeMenuAction) return;

    QString theme(currentThemeName());

    d->themeMenuAction->menu()->clear();
    delete d->themeMenuActionGroup;

    d->themeMenuActionGroup = new QActionGroup(d->themeMenuAction);
    connect(d->themeMenuActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotChangePalette()));

    KAction* action = new KAction(defaultThemeName(), d->themeMenuActionGroup);
    action->setCheckable(true);
    d->themeMenuAction->addAction(action);

    const QStringList schemeFiles = KGlobal::dirs()->findAllResources("data", "color-schemes/*.colors", KStandardDirs::NoDuplicates);

    QMap<QString, QAction*> actionMap;
    for (int i = 0; i < schemeFiles.size(); ++i)
    {
        const QString filename  = schemeFiles.at(i);
        const QFileInfo info(filename);
        KSharedConfigPtr config = KSharedConfig::openConfig(filename);
        QIcon icon              = createSchemePreviewIcon(config);
        KConfigGroup group(config, "General");
        const QString name      = group.readEntry("Name", info.baseName());
        action                  = new KAction(name, d->themeMenuActionGroup);
        d->themeMap.insert(name, filename);
        action->setIcon(icon);
        action->setCheckable(true);
        actionMap.insert(name, action);
    }

    // sort the list
    QStringList actionMapKeys = actionMap.keys();
    actionMapKeys.sort();

    foreach(const QString& name, actionMapKeys)
    {
        d->themeMenuAction->addAction(actionMap.value(name));
    }

    updateCurrentKDEdefaultThemePreview();
    setCurrentTheme(theme);

    d->themeMenuAction->addSeparator();
    KAction* config = new KAction(i18n("Configuration..."), d->themeMenuAction);
    config->setIcon(koIcon("preferences-desktop-theme"));
    d->themeMenuAction->addAction(config);

    connect(config, SIGNAL(triggered()),
            this, SLOT(slotConfigColors()));
}

void ThemeManager::slotConfigColors()
{
    int ret = KToolInvocation::kdeinitExec("kcmshell4", QStringList() << "colors");
    if (ret > 0)
    {
        KMessageBox::error(0, i18n("Cannot start Colors Settings panel from KDE Control Center. "
                                   "Please check your system..."));
    }
}

void ThemeManager::updateCurrentKDEdefaultThemePreview()
{
    QList<QAction*> list = d->themeMenuActionGroup->actions();
    foreach(QAction* action, list)
    {
        if (action->text().remove('&') == defaultThemeName())
        {
            KSharedConfigPtr config = KSharedConfig::openConfig(d->themeMap.value(currentKDEdefaultTheme()));
            QIcon icon              = createSchemePreviewIcon(config);
            action->setIcon(icon);
        }
    }
}

QPixmap ThemeManager::createSchemePreviewIcon(const KSharedConfigPtr& config)
{
    // code taken from kdebase/workspace/kcontrol/colors/colorscm.cpp
    const uchar bits1[] = { 0xff, 0xff, 0xff, 0x2c, 0x16, 0x0b };
    const uchar bits2[] = { 0x68, 0x34, 0x1a, 0xff, 0xff, 0xff };
    const QSize bitsSize(24, 2);
    const QBitmap b1    = QBitmap::fromData(bitsSize, bits1);
    const QBitmap b2    = QBitmap::fromData(bitsSize, bits2);

    QPixmap pixmap(23, 16);
    pixmap.fill(Qt::black); // FIXME use some color other than black for borders?

    KConfigGroup group(config, "WM");
    QPainter p(&pixmap);
    KColorScheme windowScheme(QPalette::Active, KColorScheme::Window, config);
    p.fillRect(1,  1, 7, 7, windowScheme.background());
    p.fillRect(2,  2, 5, 2, QBrush(windowScheme.foreground().color(), b1));

    KColorScheme buttonScheme(QPalette::Active, KColorScheme::Button, config);
    p.fillRect(8,  1, 7, 7, buttonScheme.background());
    p.fillRect(9,  2, 5, 2, QBrush(buttonScheme.foreground().color(), b1));

    p.fillRect(15,  1, 7, 7, group.readEntry("activeBackground", QColor(96, 148, 207)));
    p.fillRect(16,  2, 5, 2, QBrush(group.readEntry("activeForeground", QColor(255, 255, 255)), b1));

    KColorScheme viewScheme(QPalette::Active, KColorScheme::View, config);
    p.fillRect(1,  8, 7, 7, viewScheme.background());
    p.fillRect(2, 12, 5, 2, QBrush(viewScheme.foreground().color(), b2));

    KColorScheme selectionScheme(QPalette::Active, KColorScheme::Selection, config);
    p.fillRect(8,  8, 7, 7, selectionScheme.background());
    p.fillRect(9, 12, 5, 2, QBrush(selectionScheme.foreground().color(), b2));

    p.fillRect(15,  8, 7, 7, group.readEntry("inactiveBackground", QColor(224, 223, 222)));
    p.fillRect(16, 12, 5, 2, QBrush(group.readEntry("inactiveForeground", QColor(20, 19, 18)), b2));

    p.end();
    return pixmap;
}

QString ThemeManager::currentKDEdefaultTheme() const
{
    KSharedConfigPtr config = KSharedConfig::openConfig("kdeglobals");
    KConfigGroup group(config, "General");
    return group.readEntry("ColorScheme");
}

void ThemeManager::slotSettingsChanged()
{
    populateThemeMenu();
    slotChangePalette();
}

}  // namespace Digikam
