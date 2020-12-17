/*
 *  SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_color_manager.h"

Q_GLOBAL_STATIC(KisColorManager, s_instance)

class KisColorManager::Private {
public:
    Private(QObject *)
    {}
};

KisColorManager::KisColorManager()
    : QObject()
    , d(new Private(this))
{
}

KisColorManager::~KisColorManager()
{
    delete d;
}

QString KisColorManager::deviceName(const QString &)
{
    return QString();
}

QStringList KisColorManager::devices(DeviceType ) const
{
    return QStringList();
}

QByteArray KisColorManager::displayProfile(const QString &, int ) const
{
    return QByteArray();
}

KisColorManager *KisColorManager::instance()
{
    return s_instance;
}
