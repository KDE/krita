/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
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

#include "KritaNamespace.h"

#include "ImageBuilder.h"
#include "MouseTracker.h"
#include "VirtualKeyboardController.h"
#include "DocumentManager.h"
#include "ProgressProxy.h"
#include <QDir>

class KritaNamespace::Private
{
public:
    QObject *imageBuilder;
    QObject *mouseTracker;
    QObject* window;
};

KritaNamespace::KritaNamespace(QObject* parent)
    : QObject(parent), d(new Private)
{
    d->imageBuilder = new ImageBuilder(this);
    d->mouseTracker = new MouseTracker(this);
    d->window = 0;
}

KritaNamespace::~KritaNamespace()
{
    delete d;
}

QObject* KritaNamespace::imageBuilder() const
{
    return d->imageBuilder;
}

QObject* KritaNamespace::mouseTracker() const
{
    return d->mouseTracker;
}

QObject* KritaNamespace::window() const
{
    return d->window;
}

void KritaNamespace::setWindow(QObject* window)
{
    d->window = window;
    emit windowChanged();
}

QObject* KritaNamespace::virtualKeyboardController() const
{
    return VirtualKeyboardController::instance();
}

QObject* KritaNamespace::progressProxy() const
{
    return DocumentManager::instance()->progressProxy();
}

bool KritaNamespace::fileExists(const QString& filename) const
{
    return QDir().exists(filename);
}
