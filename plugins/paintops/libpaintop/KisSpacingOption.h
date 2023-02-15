/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSPACINGOPTION_H
#define KISSPACINGOPTION_H

#include <KisCurveOption.h>

struct KisSpacingOptionData;

class PAINTOP_EXPORT KisSpacingOption : public KisCurveOption
{
public:
    KisSpacingOption(const KisPropertiesConfiguration *setting);

    qreal apply(const KisPaintInformation & info) const;

    bool isotropicSpacing() const;

    /**
     * @return True if and only if the spacing option allows spacing updates between painted dabs.
     */
    bool usingSpacingUpdates() const;

private:
    KisSpacingOption(const KisSpacingOptionData &data);
private:
    bool m_isotropicSpacing;
    bool m_useSpacingUpdates;
};

#endif // KISSPACINGOPTION_H
