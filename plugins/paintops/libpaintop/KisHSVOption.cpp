/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisHSVOption.h"


#include <kis_properties_configuration.h>
#include <kis_paint_information.h>
#include <KisStandardOptionData.h>


KisHSVOption::KisHSVOption(const KisCurveOptionData &data)
    : KisCurveOption(data),
      m_id(data.id)
{
}

KisHSVOption *KisHSVOption::createHueOption(const KisPropertiesConfiguration *setting)
{
    return new KisHSVOption(initializeData<KisHueOptionData>(setting));
}

KisHSVOption *KisHSVOption::createSaturationOption(const KisPropertiesConfiguration *setting)
{
    return new KisHSVOption(initializeData<KisSaturationOptionData>(setting));
}

KisHSVOption *KisHSVOption::createValueOption(const KisPropertiesConfiguration *setting)
{
    return new KisHSVOption(initializeData<KisValueOptionData>(setting));
}

void KisHSVOption::apply(KoColorTransformation* transfo, const KisPaintInformation& info) const
{
    if (!isChecked()) return;

    if (m_paramId == -1) {
        m_paramId = transfo->parameterId(m_id.id());
    }

    qreal v = 0;
    if (m_id.id() == "h") {
        const qreal scalingPartCoeff = 1.0;
        v = computeRotationLikeValue(info, 0, false, scalingPartCoeff, info.isHoveringMode());
    } else {
        v = computeSizeLikeValue(info);
        qreal halfValue = strengthValue() * 0.5;
        v = (v * strengthValue()) + (0.5 - halfValue);
        v = (v * 2) - 1;
    }
    transfo->setParameter(m_paramId, v);
    transfo->setParameter(3, 0); //sets the type to HSV.
    transfo->setParameter(4, false); //sets the colorize to false.

    /**
     * Theoretically, we might want to make this a user-selectable
     * option, but I doubt anyone would complain. See details in the
     * bugreport:
     *
     * https://bugs.kde.org/show_bug.cgi?id=462193
     */
    transfo->setParameter(8, false); //sets the compatibility to false.
}

template <typename Data>
Data KisHSVOption::initializeData(const KisPropertiesConfiguration *setting)
{
    Data data;
    data.read(setting);
    return data;
}
