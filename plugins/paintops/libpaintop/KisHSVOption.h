/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISHSVOPTION_H
#define KISHSVOPTION_H

#include <KisCurveOption.h>

class KoColorTransformation;
struct KisHSVOptionData;


class PAINTOP_EXPORT KisHSVOption : public KisCurveOption
{
public:
    static KisHSVOption* createHueOption(const KisPropertiesConfiguration *setting);
    static KisHSVOption* createSaturationOption(const KisPropertiesConfiguration *setting);
    static KisHSVOption* createValueOption(const KisPropertiesConfiguration *setting);

    void apply(KoColorTransformation* transfo, const KisPaintInformation& info) const;

private:
    KisHSVOption(const KisCurveOptionData &data);

    template<typename Data>
    static Data initializeData(const KisPropertiesConfiguration *setting);

private:
    KoID m_id;
    mutable int m_paramId {-1};
};

#endif // KISHSVOPTION_H
