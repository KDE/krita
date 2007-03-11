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

#include <lcms.h>

#include <QByteArray>
#include <QString>

#include <pigment_export.h>

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
    icColorSpaceSignature colorSpaceSignature() const;
    /**
     * @return the class of the color space signature
     */
    icProfileClassSignature deviceClass() const;
    /**
     * @return the name of the profile
     */
    QString productName() const;
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
    cmsHPROFILE profile();

    friend bool operator==( const KoColorProfile &,  const KoColorProfile & );

    /**
     * @return true if the profile is valid, false if it isn't been loaded in memory yet, or
     * if the loaded memory is a bad profile
     */
    bool valid() const;

    /**
     * @return true if you can use this profile can be used to convert color from a different
     * profile to this one
     */
    bool isSuitableForOutput();

    /**
     * @return the filename of the profile (it's an empty string if it was initialized from
     * a memory pointer or from a LCMS structure)
     */
    QString filename() const;

    /**
     * @return an array with the raw data of the profile
     */
    QByteArray rawData() const;

public:
    /**
     * @return the color profile of the screen
     */
    static KoColorProfile *  getScreenProfile(int screen = -1);

private:
    bool init();

    class Private;
    Private * const d;
};

#endif // KOCOLORPROFILE_H

