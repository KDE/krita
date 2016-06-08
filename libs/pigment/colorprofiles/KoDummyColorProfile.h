/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KO_DUMMY_COLOR_PROFILE_H_
#define _KO_DUMMY_COLOR_PROFILE_H_

#include "KoColorProfile.h"

class KoDummyColorProfile : public KoColorProfile
{
public:
    KoDummyColorProfile();
    virtual ~KoDummyColorProfile();
    virtual KoColorProfile* clone() const;
    virtual bool valid() const;
    virtual float version() const;
    virtual bool isSuitableForOutput() const;
    virtual bool isSuitableForPrinting() const;
    virtual bool isSuitableForDisplay() const;
    virtual bool supportsPerceptual() const;
    virtual bool supportsSaturation() const;
    virtual bool supportsAbsolute() const;
    virtual bool supportsRelative() const;
    virtual bool hasColorants() const;
    virtual bool hasTRC() const;
    virtual QVector <double> getColorantsXYZ() const;
    virtual QVector <double> getColorantsxyY() const;
    virtual QVector <double> getWhitePointXYZ() const;
    virtual QVector <double> getWhitePointxyY() const;
    virtual QVector <double> getEstimatedTRC() const;
    virtual void linearizeFloatValue(QVector <double> & Value) const;
    virtual void delinearizeFloatValue(QVector <double> & Value) const;
    virtual void linearizeFloatValueFast(QVector <double> & Value) const;
    virtual void delinearizeFloatValueFast(QVector <double> & Value) const;
    virtual bool operator==(const KoColorProfile&) const;
};

#endif
