/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KO_DUMMY_COLOR_PROFILE_H_
#define _KO_DUMMY_COLOR_PROFILE_H_

#include "KoColorProfile.h"

class KoDummyColorProfile : public KoColorProfile
{
public:
    KoDummyColorProfile();
    ~KoDummyColorProfile() override;
    KoColorProfile* clone() const override;
    bool valid() const override;
    float version() const override;
    bool isSuitableForOutput() const override;
    bool isSuitableForPrinting() const override;
    bool isSuitableForDisplay() const override;
    bool supportsPerceptual() const override;
    bool supportsSaturation() const override;
    bool supportsAbsolute() const override;
    bool supportsRelative() const override;
    bool hasColorants() const override;
    bool hasTRC() const override;
    bool isLinear() const override;
    QVector <double> getColorantsXYZ() const override;
    QVector <double> getColorantsxyY() const override;
    QVector <double> getWhitePointXYZ() const override;
    QVector <double> getWhitePointxyY() const override;
    QVector <double> getEstimatedTRC() const override;
    void linearizeFloatValue(QVector <double> & Value) const override;
    void delinearizeFloatValue(QVector <double> & Value) const override;
    void linearizeFloatValueFast(QVector <double> & Value) const override;
    void delinearizeFloatValueFast(QVector <double> & Value) const override;
    bool operator==(const KoColorProfile&) const override;
    QByteArray uniqueId() const override;
};

#endif
