/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCURVEOPTIONDATAUNIFORMPROPERTY_H
#define KISCURVEOPTIONDATAUNIFORMPROPERTY_H

#include <QScopedPointer>
#include "kis_slider_based_paintop_property.h"
#include <kritapaintop_export.h>

struct KisCurveOptionData;
class KisPaintOpPresetUpdateProxy;

class PAINTOP_EXPORT KisCurveOptionDataUniformProperty : public KisDoubleSliderBasedPaintOpProperty
{
public:
    KisCurveOptionDataUniformProperty(const KisCurveOptionData &data, KisPaintOpSettingsRestrictedSP settings, QObject *parent);
    KisCurveOptionDataUniformProperty(const KisCurveOptionData &data, const QString &propertyId, KisPaintOpSettingsRestrictedSP settings, QObject *parent);
    ~KisCurveOptionDataUniformProperty() override;

    void readValueImpl() override;
    void writeValueImpl() override;

    bool isVisible() const override;

private:
    KisCurveOptionDataUniformProperty(const KisCurveOptionData &data, const KoID &propertyId, KisPaintOpSettingsRestrictedSP settings, QObject *parent);
private:
    QScopedPointer<KisCurveOptionData> m_data;
};

#endif // KISCURVEOPTIONDATAUNIFORMPROPERTY_H
