/*
 *  SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QGlobalStatic>

#include "kis_color_manager.h"
#include "colord/KisColord.h"

#include <kis_debug.h>

Q_GLOBAL_STATIC(KisColorManager, s_instance)

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
    //dbgKrita << "ColorManager started";
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
    return s_instance;
}
