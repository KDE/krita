/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISOPACITYOPTION_H
#define KISOPACITYOPTION_H

#include <KisStandardOptions.h>

class KisPainter;

class PAINTOP_EXPORT KisOpacityOption : public KisStandardOption<KisOpacityOptionData>
{
public:
    using BaseClass = KisStandardOption<KisOpacityOptionData>;

    KisOpacityOption(const KisPropertiesConfiguration *setting, KisNodeSP currentNode);

    using BaseClass::apply;

    void apply(KisPainter* painter, const KisPaintInformation& info) const;

private:
    bool m_indirectPaintingActive = false;
};

#endif // KISOPACITYOPTION_H
