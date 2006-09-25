/*
 *  KoColorProfile.h - part of Krayon
 *
 *  Copyright (c) 2000 Matthias Elter  <elter@kde.org>
 *                2004 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOCOLORPROFILE_H
#define KOCOLORPROFILE_H

#include <config.h>

#include <lcms.h>

#include <q3valuevector.h>

#include <kio/job.h>

#include <koffice_export.h>

class PIGMENT_EXPORT KoColorProfile {

public:
    KoColorProfile(QByteArray rawData);
    KoColorProfile(const QString& file);
    KoColorProfile(const cmsHPROFILE profile);

    virtual ~KoColorProfile();

    virtual bool load();
    virtual bool save();

    inline icColorSpaceSignature colorSpaceSignature() const { return m_colorSpaceSignature; }
    inline icProfileClassSignature deviceClass() const { return m_deviceClass; }
    inline QString productName() const { return m_productName; }
    inline QString productDescription() const { return m_productDescription; }
    inline QString productInfo() const { return m_productInfo; }
    inline QString manufacturer() const { return m_manufacturer; }
    cmsHPROFILE profile();

    friend inline bool operator==( const KoColorProfile &,  const KoColorProfile & );

    inline bool valid() const { return m_valid; };

    inline bool isSuitableForOutput() { return m_suitableForOutput; };

    inline QString filename() const { return m_filename; }

    inline QByteArray rawData() const { return m_rawData; }

public:

    static KoColorProfile *  getScreenProfile(int screen = -1);

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

inline bool operator==( const KoColorProfile & p1,  const KoColorProfile & p2 )
{
    return p1.m_profile == p2.m_profile;
}

#endif // KOCOLORPROFILE_H

