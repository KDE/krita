 /*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#include <stdint.h>
#include <limits.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <fixx11h.h>
#include <QX11Info>
#endif

#include <QFile>
#include <QSharedPointer>

#include "DebugPigment.h"
#include "LcmsColorProfileContainer.h"

#include "lcms2.h"

struct IccColorProfile::Data::Private {
    QByteArray rawData;
};

IccColorProfile::Data::Data() : d(new Private)
{
}
IccColorProfile::Data::Data(const QByteArray &rawData) : d(new Private)
{
    d->rawData = rawData;
}


IccColorProfile::Data::~Data()
{
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
        QScopedPointer<IccColorProfile::Data> data;
        QScopedPointer<LcmsColorProfileContainer> lcmsProfile;
        QVector<KoChannelInfo::DoubleRange> uiMinMaxes;
    };
    QSharedPointer<Shared> shared;
};

IccColorProfile::IccColorProfile(const QString &fileName)
    : KoColorProfile(fileName), d(new Private)
{
    // QSharedPointer lacks a reset in Qt 4.x
    d->shared = QSharedPointer<Private::Shared>( new Private::Shared() );
    d->shared->data.reset( new Data() );
}

IccColorProfile::IccColorProfile(const QByteArray& rawData)
    : KoColorProfile(""), d(new Private)
{
    d->shared = QSharedPointer<Private::Shared>( new Private::Shared() );
    d->shared->data.reset( new Data() );

    setRawData(rawData);
    init();
}

IccColorProfile::IccColorProfile(const IccColorProfile& rhs)
    : KoColorProfile(rhs)
    , d(new Private(*rhs.d))
{
    Q_ASSERT(d->shared);
}

IccColorProfile::~IccColorProfile()
{
    Q_ASSERT(d->shared);
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
    if (init()) {
        return true;
    }
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
        d->shared->lcmsProfile.reset( new LcmsColorProfileContainer(d->shared->data.data()) );
    }
    if (d->shared->lcmsProfile->init()) {
        setName(d->shared->lcmsProfile->name());
        setInfo(d->shared->lcmsProfile->info());
        CalculateFloatUIMinMax();
        return true;
    } else {
        return false;
    }
}

LcmsColorProfileContainer* IccColorProfile::asLcms() const
{
    Q_ASSERT(d->shared->lcmsProfile);
    return d->shared->lcmsProfile.data();
}

bool IccColorProfile::operator==(const KoColorProfile& rhs) const
{
    const IccColorProfile* rhsIcc = dynamic_cast<const IccColorProfile*>(&rhs);
    if (rhsIcc) {
        return d->shared == rhsIcc->d->shared;
    }
    return false;
}

const QVector<KoChannelInfo::DoubleRange> & IccColorProfile::GetFloatUIMinMax(void) const
{
    Q_ASSERT(!d->shared->uiMinMaxes.isEmpty());
    return d->shared->uiMinMaxes;
}

void IccColorProfile::CalculateFloatUIMinMax(void)
{
    QVector<KoChannelInfo::DoubleRange> &ret = d->shared->uiMinMaxes;

    cmsHPROFILE cprofile = d->shared->lcmsProfile->lcmsProfile();
    Q_ASSERT(cprofile);

    cmsColorSpaceSignature color_space_sig = cmsGetColorSpace(cprofile);
    unsigned int num_channels = cmsChannelsOf(color_space_sig);
    unsigned int color_space_mask = _cmsLCMScolorSpace(color_space_sig);

    Q_ASSERT(num_channels>=1 && num_channels <=4);  // num_channels==1 is for grayscale, we need to handle it
    Q_ASSERT(color_space_mask);

    // to try to find the max range of float/doubles for this profile,
    // pass in min/max int and make the profile convert that
    // this is far from perfect, we need a better way, if possible to get the "bounds" of a profile

    uint16_t in_min_pixel[4] = {0,0,0,0};
    uint16_t in_max_pixel[4] = {0xFFFF,0xFFFF,0xFFFF,0xFFFF};
    double out_min_pixel[4] = {0,0,0,0};
    double out_max_pixel[4] = {0,0,0,0};

    cmsHTRANSFORM trans = cmsCreateTransform(
        cprofile,
        (COLORSPACE_SH(color_space_mask)|CHANNELS_SH(num_channels)|BYTES_SH(2)),
        cprofile,
        (COLORSPACE_SH(color_space_mask)|FLOAT_SH(1)|CHANNELS_SH(num_channels)|BYTES_SH(0)), //NOTE THAT 'BYTES' FIELD IS SET TO ZERO ON DLB because 8 bytes overflows the bitfield
        INTENT_PERCEPTUAL, 0);      // does the intent matter in this case?

    if (!trans) {
        // log something?
        // assume [0..1] for all channels I guess
        ret.resize(num_channels);
        for (unsigned int i=0; i<num_channels; ++i) {
            ret[i].minVal = 0;
            ret[i].maxVal = 1;
        }
        return;
    }
    Q_ASSERT(trans);
    cmsDoTransform(trans, in_min_pixel, out_min_pixel, 1);
    cmsDoTransform(trans, in_max_pixel, out_max_pixel, 1);
    cmsDeleteTransform(trans);

    ret.resize(num_channels);
    for (unsigned int i=0; i<num_channels; ++i) {
        ret[i].minVal = out_min_pixel[i];
        ret[i].maxVal = out_max_pixel[i];
        Q_ASSERT(ret[i].minVal < ret[i].maxVal);
    }

    //for (unsigned int i=0; i<num_channels; ++i)
    //qDebug() << "*********** min/max " << i << ret[i].minVal << ret[i].maxVal;
}

