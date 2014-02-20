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

#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtGui/QColor>
#include <QtGui/QFont>

#include <kglobal.h>
#include <kstandarddirs.h>

class Theme::Private
{
public:
    Private() : iconPath("icons/") { }

    QString id;
    QString name;
    QString inherits;
    Theme* inheritedTheme;

    QVariantMap colors;
    QVariantMap sizes;
    QVariantMap fonts;

    QString iconPath;
};

Theme::Theme(QObject* parent)
    : QObject(parent), d(new Private)
{
}

Theme::~Theme()
{
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
        d->inherits = newValue;
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
    return 0.f;
}

QVariantMap Theme::fonts() const
{
    return d->fonts;
}

void Theme::setFonts(const QVariantMap& newValue)
{
    if(newValue != d->fonts) {
        d->fonts = newValue;
        emit fontsChanged();
    }
}

QFont Theme::font(const QString& name)
{
    return QFont();
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
    return QUrl::fromLocalFile(KGlobal::dirs()->findResource("data", QString("kritasketch/themes/%1/%2/%3.svg").arg(d->id, d->iconPath, name)));
}

#include "Theme.moc"
