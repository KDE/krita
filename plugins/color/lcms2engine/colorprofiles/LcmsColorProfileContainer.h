/*
 * This file is part of the KDE project
 *  Copyright (c) 2000 Matthias Elter  <elter@kde.org>
 *                2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007 Thomas Zander <zander@kde.org>
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

#ifndef _KO_LCMS_COLORPROFILE_H
#define _KO_LCMS_COLORPROFILE_H

#include "IccColorProfile.h"

#include <lcms2.h>

#include <QByteArray>
#include <QString>

/**
 * This class contains an LCMS color profile. Don't use it outside LcmsColorSpace.
 */
class LcmsColorProfileContainer : public IccColorProfile::Container
{
    friend class IccColorProfile;
protected:
    LcmsColorProfileContainer(IccColorProfile::Data *);
private:
    /**
     * Create a byte array from a lcms profile.
     */
    static QByteArray lcmsProfileToByteArray(const cmsHPROFILE profile);

public:
    /**
     * @param profile lcms memory structure with the profile, it is freed after the call
     *                to this function
     * @return an ICC profile created from an LCMS profile
     */
    static IccColorProfile *createFromLcmsProfile(const cmsHPROFILE profile);
public:

    ~LcmsColorProfileContainer() override;

    /**
     * @return the ICC color space signature
     */
    cmsColorSpaceSignature colorSpaceSignature() const;
    /**
     * @return the class of the color space signature
     */
    cmsProfileClassSignature deviceClass() const;
    /**
     * @return the name of the manufacturer
     */
    QString manufacturer() const override;
    /**
     * @return the embedded copyright
     */
    QString copyright() const override;
    /**
     * @return the structure to use with LCMS functions
     */
    cmsHPROFILE lcmsProfile() const;

    bool valid() const override;
    virtual float version() const;

    bool isSuitableForOutput() const override;

    bool isSuitableForPrinting() const override;

    bool isSuitableForDisplay() const override;

    virtual bool supportsPerceptual() const;
    virtual bool supportsSaturation() const;
    virtual bool supportsAbsolute() const;
    virtual bool supportsRelative() const;

    bool hasColorants() const override;
    virtual bool hasTRC() const;
    bool isLinear() const;
    QVector <double> getColorantsXYZ() const override;
    QVector <double> getColorantsxyY() const override;
    QVector <double> getWhitePointXYZ() const override;
    QVector <double> getWhitePointxyY() const override;
    QVector <double> getEstimatedTRC() const override;
    virtual void LinearizeFloatValue(QVector <double> & Value) const;
    virtual void DelinearizeFloatValue(QVector <double> & Value) const;
    virtual void LinearizeFloatValueFast(QVector <double> & Value) const;
    virtual void DelinearizeFloatValueFast(QVector <double> & Value) const;
    QString name() const override;
    QString info() const override;
    QByteArray getProfileUniqueId() const override;

protected:
    LcmsColorProfileContainer();

private:
    bool init();

    class Private;
    Private *const d;
};

#endif // KOCOLORPROFILE_H

