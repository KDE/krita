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

#include "IccColorProfile.h"

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

#include "LcmsColorProfileContainer.h"

struct IccColorProfile::Data::Private {
    QByteArray rawData;
};

IccColorProfile::Data::Data() : d(new Private)
{
}
IccColorProfile::Data::Data(QByteArray rawData) : d(new Private)
{
    d->rawData = rawData;
}


IccColorProfile::Data::~Data()
{
    delete d;
}

QByteArray IccColorProfile::Data::rawData()
{
    return d->rawData;
}

void IccColorProfile::Data::setRawData(const QByteArray & rawData)
{
    d->rawData = rawData;
}

IccColorProfile::Container::Container()
{
}

IccColorProfile::Container::~Container()
{
}


struct IccColorProfile::Private {
    struct Shared {
        Shared() : count(0), data(0), lcmsProfile(0), chromacities(0) {}
        ~Shared() {
            delete data; delete lcmsProfile; delete chromacities;
        }
        int count;
        IccColorProfile::Data* data;
        LcmsColorProfileContainer* lcmsProfile;
        KoRGBChromaticities* chromacities;
    };
    Shared* shared;
};

IccColorProfile::IccColorProfile(const KoRGBChromaticities& chromacities, qreal gamma, QString name) : KoColorProfile(""), d(new Private)
{
    d->shared = new Private::Shared();
    d->shared->count ++;
    d->shared->chromacities = new KoRGBChromaticities(chromacities);
    d->shared->data = new Data();
    d->shared->lcmsProfile = 0;
    d->shared->data->setRawData(LcmsColorProfileContainer::createFromChromacities(chromacities, gamma, name));
    init();
}

IccColorProfile::IccColorProfile(QString fileName) : KoColorProfile(fileName), d(new Private)
{
    d->shared = new Private::Shared();
    d->shared->count ++;
    d->shared->data = new Data();
    d->shared->lcmsProfile = 0;
    d->shared->chromacities = 0;
}

IccColorProfile::IccColorProfile(const QByteArray& rawData) : KoColorProfile(""), d(new Private)
{
    d->shared = new Private::Shared();
    d->shared->count ++;
    d->shared->data = new Data();
    d->shared->lcmsProfile = 0;
    d->shared->chromacities = 0;
    setRawData(rawData);
    init();
}

IccColorProfile::IccColorProfile(const IccColorProfile& rhs) : KoColorProfile(rhs), d(new Private(*rhs.d))
{
    Q_ASSERT(d->shared);
    d->shared->count++;
}

IccColorProfile::~IccColorProfile()
{
    Q_ASSERT(d->shared);
    d->shared->count--;
    if (d->shared->count <= 0) {
        Q_ASSERT(d->shared->count == 0);
        delete d->shared;
    }
    delete d;
}

KoColorProfile* IccColorProfile::clone() const
{
    return new IccColorProfile(*this);
}


QByteArray IccColorProfile::rawData() const
{
    return d->shared->data->rawData();
}

void IccColorProfile::setRawData(const QByteArray& rawData)
{
    d->shared->data->setRawData(rawData);
}

bool IccColorProfile::valid() const
{
    if (d->shared->lcmsProfile)
        return d->shared->lcmsProfile->valid();
    return false;
}

bool IccColorProfile::isSuitableForOutput() const
{
    if (d->shared->lcmsProfile)
        return d->shared->lcmsProfile->isSuitableForOutput();
    return false;
}

bool IccColorProfile::isSuitableForPrinting() const
{
    if (d->shared->lcmsProfile)
        return d->shared->lcmsProfile->isSuitableForPrinting();
    return false;
}

bool IccColorProfile::isSuitableForDisplay() const
{
    if (d->shared->lcmsProfile)
        return d->shared->lcmsProfile->isSuitableForDisplay();
    return false;
}


bool IccColorProfile::load()
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

bool IccColorProfile::save()
{
    return false;
}

bool IccColorProfile::init()
{
    if (!d->shared->lcmsProfile) {
        d->shared->lcmsProfile = new LcmsColorProfileContainer(d->shared->data);
    }
    if (d->shared->lcmsProfile->init()) {
        setName(d->shared->lcmsProfile->name());
        setInfo(d->shared->lcmsProfile->info());
        return true;
    } else {
        return false;
    }
}

LcmsColorProfileContainer* IccColorProfile::asLcms() const
{
    Q_ASSERT(d->shared->lcmsProfile);
    return d->shared->lcmsProfile;
}

bool IccColorProfile::operator==(const KoColorProfile& rhs) const
{
    const IccColorProfile* rhsIcc = dynamic_cast<const IccColorProfile*>(&rhs);
    if (rhsIcc) {
        return d->shared == rhsIcc->d->shared;
    }
    return false;
}

