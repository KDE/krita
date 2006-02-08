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

#include <config.h>

#include LCMS_HEADER

#include <qvaluevector.h>
#include <qcstring.h>

#include <kio/job.h>

#include <kis_annotation.h>

//XXX: Profiles should be loaded by the color strategies
//     and be available only through the color strategy
//     that matches the profile's color model
class KisProfile {

public:
    KisProfile(QByteArray rawData);
    KisProfile(const QString& file);
    KisProfile(const cmsHPROFILE profile);

    virtual ~KisProfile();

    virtual bool load();
    virtual bool save();

    inline icColorSpaceSignature colorSpaceSignature() const { return m_colorSpaceSignature; }
    inline icProfileClassSignature deviceClass() const { return m_deviceClass; }
    inline QString productName() const { return m_productName; }
    inline QString productDescription() const { return m_productDescription; }
    inline QString productInfo() const { return m_productInfo; }
    inline QString manufacturer() const { return m_manufacturer; }
    cmsHPROFILE profile();

    KisAnnotationSP annotation() const;

    friend inline bool operator==( const KisProfile &,  const KisProfile & );

    inline bool valid() const { return m_valid; };
    
    inline bool isSuitableForOutput() { return m_suitableForOutput; };

    inline QString filename() const { return m_filename; }

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

    QByteArray m_rawData;

    QString m_filename;
    bool m_valid;
    bool m_suitableForOutput;

};

inline bool operator==( const KisProfile & p1,  const KisProfile & p2 )
{
    return p1.m_profile == p2.m_profile;
}

#endif // KIS_PROFILE_H

