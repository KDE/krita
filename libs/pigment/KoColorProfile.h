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

#include <lcms.h>

#include <q3valuevector.h>

#include <kio/job.h>

#include <koffice_export.h>

/**
 * This class contains an ICC color profile.
 */
class PIGMENT_EXPORT KoColorProfile {

public:
    /**
     * Initialize the profile using the raw data. Use it after reading the profile data
     * inside an image file.
     * @param rawData profile data
     */
    explicit KoColorProfile(const QByteArray& rawData);
    /**
     * Initialize a profile from the disk. You need to call the function \ref load to actually
     * load the profile data in memory.
     * @param file filename of the profile
     */
    explicit KoColorProfile(const QString& file);
    /**
     * Initialize the profile from the LCMS color profile
     */
    explicit KoColorProfile(const cmsHPROFILE profile);

    virtual ~KoColorProfile();

    /**
     * Load the profile in memory.
     * @return true if the profile has been successfully loaded
     */
    virtual bool load();
    /**
     * Override this function to save the profile.
     * @return always return false
     */
    virtual bool save();

    /**
     * @return the ICC color space signature
     */
    inline icColorSpaceSignature colorSpaceSignature() const { return m_colorSpaceSignature; }
    /**
     * @return the class of the color space signature
     */
    inline icProfileClassSignature deviceClass() const { return m_deviceClass; }
    /**
     * @return the name of the profile
     */
    inline QString productName() const { return m_productName; }
    /**
     * @return the description of the profile
     */
    inline QString productDescription() const { return m_productDescription; }
    /**
     * @return some information about the profile
     */
    inline QString productInfo() const { return m_productInfo; }
    /**
     * @return the name of the manufacturer
     */
    inline QString manufacturer() const { return m_manufacturer; }
    
    /**
     * @return the structure to use with LCMS functions
     */
    cmsHPROFILE profile();

    friend inline bool operator==( const KoColorProfile &,  const KoColorProfile & );

    /**
     * @return true if the profile is valid, false if it isn't been loaded in memory yet, or
     * if the loaded memory is a bad profile
     */
    inline bool valid() const { return m_valid; };

    /**
     * @return true if you can use this profile can be used to convert color from a different
     * profile to this one
     */
    inline bool isSuitableForOutput() { return m_suitableForOutput; };

    /**
     * @return the filename of the profile (it's an empty string if it was initialized from
     * a memory pointer or from a LCMS structure)
     */
    inline QString filename() const { return m_filename; }

    /**
     * @return an array with the raw data of the profile
     */
    inline QByteArray rawData() const { return m_rawData; }

public:
    /**
     * @return the color profile of the screen
     */
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

