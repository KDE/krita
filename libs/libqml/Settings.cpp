/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
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

#include "Settings.h"
#include <QApplication>


#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include "Theme.h"
#include "PropertyContainer.h"
#include <kis_config.h>

class Settings::Private
{
public:
    Private() : temporaryFile(false), focusItem(0), theme(0){ }

    QString currentFile;
    bool temporaryFile;
    QQuickItem *focusItem;
    Theme* theme;
};

Settings::Settings( QObject* parent )
    : QObject( parent ), d( new Private )
{
    // QT5TODO: Settings object is constructed in KritaSketchPlugin::initializeEngine(), where
    // creation of other qml components is not possible. But Theme::load() does that.
    // This needs some refactoring to resolve the deps. For now creating lazily in Settings::theme(), error-prone.
//     QString theme = KSharedConfig::openConfig()->group("General").readEntry<QString>("theme", "default");
//     d->theme = Theme::load(theme, this);
//     connect(d->theme, SIGNAL(fontCacheRebuilt()), SIGNAL(themeChanged()));
}

Settings::~Settings()
{
    delete d;
}


QString Settings::currentFile() const
{
    return d->currentFile;
}

void Settings::setCurrentFile(const QString& fileName)
{
    qApp->processEvents();
    if (fileName != d->currentFile) {
        d->currentFile = fileName;
        emit currentFileChanged();
    }
}

bool Settings::isTemporaryFile() const
{
    return d->temporaryFile;
}

void Settings::setTemporaryFile(bool temp)
{
    if (temp != d->temporaryFile) {
        d->temporaryFile = temp;
        emit temporaryFileChanged();
    }
}

QQuickItem* Settings::focusItem()
{
    return d->focusItem;
}

void Settings::setFocusItem(QQuickItem* item)
{
    if (item != d->focusItem) {
        d->focusItem = item;
        emit focusItemChanged();
    }
}

QObject* Settings::theme() const
{
    // create lazily for now, see constructor notes
    if (!d->theme) {
    QString theme = KSharedConfig::openConfig()->group("General").readEntry<QString>("theme", "default");
    d->theme = Theme::load(theme, const_cast<Settings*>(this));
    connect(d->theme, SIGNAL(fontCacheRebuilt()), SIGNAL(themeChanged()));
    }
    return d->theme;
}

QString Settings::themeID() const
{
    if(d->theme)
        return d->theme->id();

    return QString();
}

void Settings::setThemeID(const QString& id)
{
    if(!d->theme || id != d->theme->id()) {
        if(d->theme) {
            delete d->theme;
            d->theme = 0;
        }

        d->theme = Theme::load(id, this);
        KSharedConfig::openConfig()->group("General").writeEntry<QString>("theme", id);

        emit themeChanged();
    }
}

QObject* Settings::customImageSettings() const
{
    QObject* settings = new PropertyContainer("customImageSettings", qApp);
    KisConfig cfg;
    settings->setProperty("Width", cfg.defImageWidth());
    settings->setProperty("Height", cfg.defImageHeight());
    settings->setProperty("Resolution", qRound(cfg.defImageResolution() * 72)); // otherwise we end up with silly floating point numbers
    settings->setProperty("ColorModel", cfg.defColorModel());
    settings->setProperty("ColorDepth", cfg.defaultColorDepth());
    settings->setProperty("ColorProfile", cfg.defColorProfile());
    return settings;
}

QString Settings::lastPreset() const
{
    KisConfig cfg;
    return cfg.readEntry("LastPreset", QString("Basic_tip_default"));
}

