/*
 * This file is part of the KDE project
 * Copyright (C) 2014 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "Theme.h"

#include <QStringList>
#include <QUrl>
#include <kis_debug.h>
#include <QFile>
#include <QDir>
#include <QColor>
#include <QFont>
#include <QFontDatabase>
#include <QApplication>
#include <QWidget>
#include <QQmlComponent>
#include <QStandardPaths>

#include "QmlGlobalEngine.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class Theme::Private
{
public:
    Private()
        : inheritedTheme(0)
        , iconPath("icons/")
        , imagePath("images/")
        , fontPath("fonts/")
        , fontsAdded(false)
        , lineCountLandscape(40)
        , lineCountPortrait(70)
    { }

    void rebuildFontCache();

    QString id;
    QString name;
    QString inherits;
    Theme* inheritedTheme;

    QVariantMap colors;
    QVariantMap sizes;
    QVariantMap fonts;

    QString basePath;
    QString iconPath;
    QString imagePath;
    QString fontPath;

    QHash<QString, QColor> colorCache;
    QHash<QString, QFont> fontMap;

    bool fontsAdded;
    QList<int> addedFonts;
    int lineCountLandscape;
    int lineCountPortrait;
};

Theme::Theme(QObject* parent)
    : QObject(parent), d(new Private)
{
    qApp->installEventFilter(this);
}

Theme::~Theme()
{
    QFontDatabase db;
    Q_FOREACH(int id, d->addedFonts) {
        db.removeApplicationFont(id);
    }

    delete d;
}

QString Theme::id() const
{
    return d->id;
}

void Theme::setId(const QString& newValue)
{
    if(newValue != d->id) {
        d->id = newValue;
        const QString themeQmlPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString("kritasketch/themes/%1/theme.qml").arg(d->id));
        d->basePath = QFileInfo(themeQmlPath).dir().absolutePath();
        emit idChanged();
    }
}

QString Theme::name() const
{
    return d->name;
}

void Theme::setName(const QString& newValue)
{
    if(newValue != d->name) {
        d->name = newValue;
        emit nameChanged();
    }
}

QString Theme::inherits() const
{
    return d->inherits;
}

void Theme::setInherits(const QString& newValue)
{
    if(newValue != d->inherits) {
        if(d->inheritedTheme) {
            delete d->inheritedTheme;
            d->inheritedTheme = 0;
        }
        d->inherits = newValue;

        if(!d->inherits.isEmpty()) {
            d->inheritedTheme = Theme::load(d->inherits, this);
            connect(d->inheritedTheme, SIGNAL(fontCacheRebuilt()), SIGNAL(fontCacheRebuilt()));
        }

        emit inheritsChanged();
    }
}

QVariantMap Theme::colors() const
{
    return d->colors;
}

void Theme::setColors(const QVariantMap& newValue)
{
    if(newValue != d->colors) {
        d->colors = newValue;
        emit colorsChanged();
    }
}

QColor Theme::color(const QString& name)
{
    if(d->colorCache.contains(name))
        return d->colorCache.value(name);

    QStringList parts = name.split('/');
    QColor result;

    if(!parts.isEmpty())
    {
        QVariantMap map = d->colors;
        QString current = parts.takeFirst();

        while(map.contains(current))
        {
            QVariant value = map.value(current);
            if(value.type() == QVariant::Map)
            {
                if(parts.isEmpty())
                    break;

                map = value.toMap();
                current = parts.takeFirst();
            }
            else
            {
                result = value.value<QColor>();
                map = QVariantMap();
            }
        }
    }

    if(!result.isValid() && d->inheritedTheme) {
        result = d->inheritedTheme->color(name);
    }

    if(!result.isValid()) {
        warnKrita << "Unable to find color" << name;
    } else {
        d->colorCache.insert(name, result);
    }

    return result;
}

QVariantMap Theme::sizes() const
{
    return d->sizes;
}

void Theme::setSizes(const QVariantMap& newValue)
{
    if(newValue != d->sizes) {
        d->sizes = newValue;
        emit sizesChanged();
    }
}

float Theme::size(const QString& name)
{
    Q_UNUSED(name);
    return 0.f;
}

QVariantMap Theme::fonts() const
{
    return d->fonts;
}

void Theme::setFonts(const QVariantMap& newValue)
{
    if(newValue != d->fonts)
    {
        d->fonts = newValue;

        d->fontMap.clear();

        emit fontsChanged();
    }
}

QFont Theme::font(const QString& name)
{
    if(!d->fontsAdded) {
        QDir fontDir(d->basePath + '/' + d->fontPath);
        QStringList entries = fontDir.entryList(QDir::Files);
        QFontDatabase db;
        Q_FOREACH(QString entry, entries) {
            d->addedFonts.append(db.addApplicationFont(fontDir.absoluteFilePath(entry)));
        }
        d->fontsAdded = true;
    }

    if(d->fontMap.isEmpty()) {
        d->rebuildFontCache();
    }

    if(d->fontMap.contains(name))
        return d->fontMap.value(name);

    if(d->inheritedTheme)
        return d->inheritedTheme->font(name);

    warnKrita << "Unable to find font" << name;
    return QFont();
}

QString Theme::fontPath() const
{
    return d->fontPath;
}

void Theme::setFontPath(const QString& newValue)
{
    if(newValue != d->fontPath) {
        if(!d->addedFonts.isEmpty()) {
            QFontDatabase db;
            Q_FOREACH(int id, d->addedFonts) {
                db.removeApplicationFont(id);
            }
            d->addedFonts.clear();
        }

        d->fontPath = newValue;
        d->fontsAdded = false;

        emit fontPathChanged();
    }
}


QString Theme::iconPath() const
{
    return d->iconPath;
}

void Theme::setIconPath(const QString& newValue)
{
    if(newValue != d->iconPath) {
        d->iconPath = newValue;
        emit iconPathChanged();
    }
}

QUrl Theme::icon(const QString& name)
{
    QString url = QString("%1/%2/%3.svg").arg(d->basePath, d->iconPath, name);
    if(!QFile::exists(url)) {
        if(d->inheritedTheme) {
            return d->inheritedTheme->icon(name);
        } else {
            warnKrita << "Unable to find icon" << url;
        }
    }

    return QUrl::fromLocalFile(url);
}

QString Theme::imagePath() const
{
    return d->imagePath;
}

void Theme::setImagePath(const QString& newValue)
{
    if(newValue != d->imagePath) {
        d->imagePath = newValue;
        emit imagePathChanged();
    }
}

QUrl Theme::image(const QString& name)
{
    QString url = QString("%1/%2/%3").arg(d->basePath, d->imagePath, name);
    if(!QFile::exists(url)) {
        if(d->inheritedTheme) {
            return d->inheritedTheme->image(name);
        } else {
            warnKrita << "Unable to find image" << url;
        }
    }

    return QUrl::fromLocalFile(url);
}

Theme* Theme::load(const QString& id, QObject* parent)
{
    QString qml;

    //Ugly hacky stuff for making things work on Windows
#ifdef Q_OS_WIN
    QDir appdir(qApp->applicationDirPath());

    // Corrects for mismatched case errors in path (qtdeclarative fails to load)
    wchar_t buffer[1024];
    QString absolute = appdir.absolutePath();
    DWORD rv = ::GetShortPathName((wchar_t*)absolute.utf16(), buffer, 1024);
    rv = ::GetLongPathName(buffer, buffer, 1024);
    QString correctedPath((QChar *)buffer);
    appdir.setPath(correctedPath);

    // for now, the app in bin/ and we still use the env.bat script
    appdir.cdUp();
    qml = QString("%1/share/apps/kritasketch/themes/%2/theme.qml").arg(appdir.canonicalPath(), id);
#else
    qml = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString("kritasketch/themes/%1/theme.qml").arg(id));
#endif

    QQmlComponent themeComponent(QmlGlobalEngine::instance()->engine(), parent);
    themeComponent.loadUrl(QUrl::fromLocalFile(qml), QQmlComponent::PreferSynchronous);

    if(themeComponent.isError()) {
        warnKrita << themeComponent.errorString();
        return 0;
    }

    Theme* theme = qobject_cast<Theme*>(themeComponent.create());
    if(!theme) {
        warnKrita << "Failed to create theme instance!";
        return 0;
    }

    return theme;
}

bool Theme::eventFilter(QObject* target, QEvent* event)
{
    if(target == qApp->activeWindow() && target->inherits("QMainWindow") && event->type() == QEvent::Resize) {
        d->rebuildFontCache();
        emit fontCacheRebuilt();
    }

    return QObject::eventFilter(target, event);
}

void Theme::Private::rebuildFontCache()
{
    fontMap.clear();
    QFontDatabase db;
    for(QVariantMap::iterator itr = fonts.begin(); itr != fonts.end(); ++itr)
    {
        QVariantMap map = itr->toMap();
        if(map.isEmpty())
            continue;

        QFont font = db.font(map.value("family").toString(), map.value("style", "Regular").toString(), 10);

        if(font.isCopyOf(qApp->font()))
            warnKrita << "Could not find font" << map.value("family") << "with style" << map.value("style", "Regular");

        float lineCount = qApp->activeWindow()->height() > qApp->activeWindow()->width() ? lineCountPortrait : lineCountLandscape;
        float lineHeight = qApp->activeWindow()->height() / lineCount;
        font.setPixelSize(lineHeight * map.value("size", 1).toFloat());

        fontMap.insert(itr.key(), font);
    }
}

