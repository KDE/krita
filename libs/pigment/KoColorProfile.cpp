/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "KoColorProfile.h"
#include "DebugPigment.h"

struct Q_DECL_HIDDEN KoColorProfile::Private {
    QString name;
    QString info;
    QString fileName;
    QString manufacturer;
    QString copyright;
};

KoColorProfile::KoColorProfile(const QString &fileName) : d(new Private)
{
//     dbgPigment <<" Profile filename =" << fileName;
    d->fileName = fileName;
}

KoColorProfile::KoColorProfile(const KoColorProfile& profile)
    : d(new Private(*profile.d))
{
}

KoColorProfile::~KoColorProfile()
{
    delete d;
}

bool KoColorProfile::load()
{
    return false;
}

bool KoColorProfile::save(const QString & filename)
{
    Q_UNUSED(filename);
    return false;
}


QString KoColorProfile::name() const
{
    return d->name;
}

QString KoColorProfile::info() const
{
    return d->info;
}
QString KoColorProfile::manufacturer() const
{
    return d->manufacturer;
}
QString KoColorProfile::copyright() const
{
    return d->copyright;
}
QString KoColorProfile::fileName() const
{
    return d->fileName;
}

void KoColorProfile::setFileName(const QString &f)
{
    d->fileName = f;
}

void KoColorProfile::setName(const QString &name)
{
    d->name = name;
}
void KoColorProfile::setInfo(const QString &info)
{
    d->info = info;
}
void KoColorProfile::setManufacturer(const QString &manufacturer)
{
    d->manufacturer = manufacturer;
}
void KoColorProfile::setCopyright(const QString &copyright)
{
    d->copyright = copyright;
}
