/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
