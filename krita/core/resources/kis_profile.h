/*
 *  kis_profile.h - part of Krayon
 *
 *  Copyright (c) 2000 Matthias Elter  <elter@kde.org>
 *                2004 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_PROFILE_H
#define KIS_PROFILE_H

#include LCMS_HEADER

#include <qvaluevector.h>
#include <qimage.h>
#include <qcstring.h>

#include <ksharedptr.h>
#include <kio/job.h>
#include <koffice_export.h>
#include "kis_resource.h"
#include "kis_types.h"


//XXX: Profiles should be loaded by the color strategies
//     and be available only through the color strategy
//     that matches the profile's color model
class KRITACORE_EXPORT KisProfile : public KisResource, public KShared {
    typedef KisResource super;
    Q_OBJECT

public:
    KisProfile(Q_UINT32 colorType);
    KisProfile(QByteArray rawData, Q_UINT32 colorType);
    KisProfile(const QString& file);
    KisProfile(const cmsHPROFILE profile, QByteArray rawData, Q_UINT32 colorType);

    // Create a profile from a cms profile handle; this profile does not have associated
    // raw data, so we cannot save it as an annotation, unless we implement the code
    // in lcms testbed TestSaveToMem -- XXX.
    KisProfile(const cmsHPROFILE profile, Q_UINT32 colorType);

    virtual ~KisProfile();

    virtual bool load();
    virtual bool save();
    virtual QImage img();

    icColorSpaceSignature colorSpaceSignature() const { return m_colorSpaceSignature; }
    icProfileClassSignature deviceClass() const { return m_deviceClass; }
    QString productName() const { return m_productName; }
    QString productDescription() const { return m_productDescription; }
    QString productInfo() const { return m_productInfo; }
    QString manufacturer() const { return m_manufacturer; }
    cmsHPROFILE profile();
private:
    // XXX: When I'm sure this isn't needed anywhere, remove it.
    DWORD colorType() { return m_lcmsColorType; }
public:
    void setColorType(DWORD colorType) { m_lcmsColorType = colorType; }

    KisAnnotationSP annotation() const;

    friend inline bool operator==( const KisProfile &,  const KisProfile & );

public:

    static KisProfile *  getScreenProfile(int screen = -1);

private:
    bool init();

    cmsHPROFILE m_profile;
    icColorSpaceSignature m_colorSpaceSignature;
    icProfileClassSignature m_deviceClass;
    QString m_productName;
    QString m_productDescription;
    QString m_productInfo;
    QString m_manufacturer;
    DWORD m_lcmsColorType;

    QByteArray m_rawData;
};

inline bool operator==( const KisProfile & p1,  const KisProfile & p2 )
{
    return p1.m_profile == p2.m_profile;
}



#endif // KIS_PROFILE_H

