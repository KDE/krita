/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoIccColorProfile.h"

#include <limits.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <fixx11h.h>
#include <QX11Info>
#endif

#include <QFile>
#include "DebugPigment.h"
#include "KoChromaticities.h"

#include "KoLcmsColorProfileContainer.h"

struct KoIccColorProfile::Data::Private {
    QByteArray rawData;
};

KoIccColorProfile::Data::Data() : d(new Private)
{
}
KoIccColorProfile::Data::Data(QByteArray rawData) : d(new Private)
{
    d->rawData = rawData;
}


KoIccColorProfile::Data::~Data()
{
    delete d;
}

QByteArray KoIccColorProfile::Data::rawData()
{
    return d->rawData;
}

void KoIccColorProfile::Data::setRawData(const QByteArray & rawData)
{
    d->rawData = rawData;
}

KoIccColorProfile::Container::Container()
{
}

KoIccColorProfile::Container::~Container()
{
}


struct KoIccColorProfile::Private {
    struct Shared {
        Shared() : count(0), data(0), lcmsProfile(0), chromacities(0) {}
        ~Shared() {
            delete data; delete lcmsProfile; delete chromacities;
        }
        int count;
        KoIccColorProfile::Data* data;
        KoLcmsColorProfileContainer* lcmsProfile;
        KoRGBChromaticities* chromacities;
    };
    Shared* shared;
};

KoIccColorProfile::KoIccColorProfile(const KoRGBChromaticities& chromacities, qreal gamma, QString name) : KoColorProfile(""), d(new Private)
{
    d->shared = new Private::Shared();
    d->shared->count ++;
    d->shared->chromacities = new KoRGBChromaticities(chromacities);
    d->shared->data = new Data();
    d->shared->lcmsProfile = 0;
    d->shared->data->setRawData(KoLcmsColorProfileContainer::createFromChromacities(chromacities, gamma, name));
    init();
}

KoIccColorProfile::KoIccColorProfile(QString fileName) : KoColorProfile(fileName), d(new Private)
{
    d->shared = new Private::Shared();
    d->shared->count ++;
    d->shared->data = new Data();
    d->shared->lcmsProfile = 0;
    d->shared->chromacities = 0;
}

KoIccColorProfile::KoIccColorProfile(const QByteArray& rawData) : KoColorProfile(""), d(new Private)
{
    d->shared = new Private::Shared();
    d->shared->count ++;
    d->shared->data = new Data();
    d->shared->lcmsProfile = 0;
    d->shared->chromacities = 0;
    setRawData(rawData);
    init();
}

KoIccColorProfile::KoIccColorProfile(const KoIccColorProfile& rhs) : KoColorProfile(rhs), d(new Private(*rhs.d))
{
    Q_ASSERT(d->shared);
    d->shared->count++;
}

KoIccColorProfile::~KoIccColorProfile()
{
    Q_ASSERT(d->shared);
    d->shared->count--;
    if (d->shared->count <= 0) {
        Q_ASSERT(d->shared->count == 0);
        delete d->shared;
    }
    delete d;
}

KoColorProfile* KoIccColorProfile::clone() const
{
    return new KoIccColorProfile(*this);
}


QByteArray KoIccColorProfile::rawData() const
{
    return d->shared->data->rawData();
}

void KoIccColorProfile::setRawData(const QByteArray& rawData)
{
    d->shared->data->setRawData(rawData);
}

bool KoIccColorProfile::valid() const
{
    if (d->shared->lcmsProfile)
        return d->shared->lcmsProfile->valid();
    return false;
}

bool KoIccColorProfile::isSuitableForOutput() const
{
    if (d->shared->lcmsProfile)
        return d->shared->lcmsProfile->isSuitableForOutput();
    return false;
}

bool KoIccColorProfile::isSuitableForPrinting() const
{
    if (d->shared->lcmsProfile)
        return d->shared->lcmsProfile->isSuitableForPrinting();
    return false;
}

bool KoIccColorProfile::isSuitableForDisplay() const
{
    if (d->shared->lcmsProfile)
        return d->shared->lcmsProfile->isSuitableForDisplay();
    return false;
}


bool KoIccColorProfile::load()
{
    QFile file(fileName());
    file.open(QIODevice::ReadOnly);
    QByteArray rawData = file.readAll();
    setRawData(rawData);
    file.close();
    if (init())
        return true;

    warnPigment << "Failed to load profile from " << fileName();
    return false;
}

bool KoIccColorProfile::save()
{
    return false;
}

bool KoIccColorProfile::init()
{
    if (!d->shared->lcmsProfile) {
        d->shared->lcmsProfile = new KoLcmsColorProfileContainer(d->shared->data);
    }
    if (d->shared->lcmsProfile->init()) {
        setName(d->shared->lcmsProfile->name());
        setInfo(d->shared->lcmsProfile->info());
        return true;
    } else {
        return false;
    }
}

KoLcmsColorProfileContainer* KoIccColorProfile::asLcms() const
{
    Q_ASSERT(d->shared->lcmsProfile);
    return d->shared->lcmsProfile;
}

bool KoIccColorProfile::operator==(const KoColorProfile& rhs) const
{
    const KoIccColorProfile* rhsIcc = dynamic_cast<const KoIccColorProfile*>(&rhs);
    if (rhsIcc) {
        return d->shared == rhsIcc->d->shared;
    }
    return false;
}

