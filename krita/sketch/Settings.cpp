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

#include <kglobal.h>
#include <kstandarddirs.h>

#include "Theme.h"

class Settings::Private
{
public:
    Private() : temporaryFile(false), focusItem(0) { }

    QString currentFile;
    bool temporaryFile;
    QDeclarativeItem *focusItem;
    Theme* theme;
};

Settings::Settings( QDeclarativeEngine* engine, QObject* parent )
    : QObject( parent ), d( new Private )
{
    QDeclarativeComponent* themeComponent = new QDeclarativeComponent(engine, this);
    themeComponent->loadUrl(KGlobal::dirs()->findResource("data", "kritasketch/themes/default/theme.qml"));
    if(themeComponent->isError())
    {
        qDebug() << themeComponent->errorString();
    }
    d->theme = qobject_cast<Theme*>(themeComponent->create());
    if(!d->theme)
        qDebug() << "Failed to create theme instance!";
    delete themeComponent;
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

QDeclarativeItem* Settings::focusItem()
{
    return d->focusItem;
}

void Settings::setFocusItem(QDeclarativeItem* item)
{
    if (item != d->focusItem) {
        d->focusItem = item;
        emit focusItemChanged();
    }
}

QObject* Settings::theme() const
{
    return d->theme;
}

#include "Settings.moc"
