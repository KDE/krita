/*
 * This file is part of the KDE project
 *  Copyright (c) 2000 Matthias Elter  <elter@kde.org>
 *                2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KoIccColorProfile.h"

#include <lcms.h>

#include <QByteArray>
#include <QString>

/**
 * This class contains an LCMS color profile. Use it with care outside KoLcmsColorSpace.
 */
class PIGMENT_EXPORT KoLcmsColorProfile : public KoIccColorProfile {

public:
    /**
     * Initialize the profile using the raw data. Use it after reading the profile data
     * inside an image file.
     * @param rawData profile data
     */
    explicit KoLcmsColorProfile(const QByteArray& rawData);
    /**
     * Initialize a profile from the disk. You need to call the function \ref load to actually
     * load the profile data in memory.
     * @param file filename of the profile
     */
    explicit KoLcmsColorProfile(const QString& file);
    /**
     * Initialize the profile from the LCMS color profile
     */
    explicit KoLcmsColorProfile(const cmsHPROFILE profile);

    virtual ~KoLcmsColorProfile();

    virtual bool load();
    
    virtual bool save();

    /**
     * @return the ICC color space signature
     */
    icColorSpaceSignature colorSpaceSignature() const;
    /**
     * @return the class of the color space signature
     */
    icProfileClassSignature deviceClass() const;
    /**
     * @return the description of the profile
     */
    QString productDescription() const;
    /**
     * @return some information about the profile
     */
    QString productInfo() const;
    /**
     * @return the name of the manufacturer
     */
    QString manufacturer() const;

    /**
     * @return the structure to use with LCMS functions
     */
    cmsHPROFILE lcmsProfile();

    friend bool operator==( const KoLcmsColorProfile &,  const KoLcmsColorProfile & );

    virtual bool valid() const;

    virtual bool isSuitableForOutput() const;

    virtual bool isSuitableForPrinting() const;

    virtual bool isSuitableForDisplay() const;

protected:
    KoLcmsColorProfile();
    void setProfile(const cmsHPROFILE profile);

private:
    bool init();

    class Private;
    Private * const d;
};

#endif // KOCOLORPROFILE_H

