/*
 *  Copyright (c) 2015 Boudewijn Rempt <boud@valdyas.org>
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

#include <kglobal.h>

#include "kis_color_manager.h"
#include "colord/KisColord.h"

#include <QDebug>

class KisColorManager::Private {
public:
    Private(QObject *parent)
        : colord(new KisColord(parent))
    {}

    KisColord *colord;
};

KisColorManager::KisColorManager()
    : QObject()
    , d(new Private(this))
{
    //qDebug() << "ColorManager started";
    connect(d->colord, SIGNAL(changed(QString)), this, SIGNAL(changed(QString)));
}

KisColorManager::~KisColorManager()
{
    delete d;
}

QString KisColorManager::deviceName(const QString &id)
{
    return d->colord->deviceName(id);
}

QStringList KisColorManager::devices(DeviceType type) const
{

    switch (type) {
    case screen:
        return d->colord->devices("display");
    case printer:
        return d->colord->devices("printer");
    case camera:
        return d->colord->devices("camera");
    case scanner:
        return d->colord->devices("scanner");
    };

    return QStringList();
}

QByteArray KisColorManager::displayProfile(const QString &device, int profile) const
{
    return d->colord->deviceProfile(device, profile);
}

KisColorManager *KisColorManager::instance()
{
    K_GLOBAL_STATIC(KisColorManager, s_instance);
    return s_instance;
}
