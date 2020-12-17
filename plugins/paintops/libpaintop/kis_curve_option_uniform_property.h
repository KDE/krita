/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_CURVE_OPTION_UNIFORM_PROPERTY_H
#define __KIS_CURVE_OPTION_UNIFORM_PROPERTY_H

#include <QScopedPointer>
#include "kis_slider_based_paintop_property.h"
#include <kritapaintop_export.h>

class KisCurveOption;


class PAINTOP_EXPORT KisCurveOptionUniformProperty : public KisDoubleSliderBasedPaintOpProperty
{
public:
    KisCurveOptionUniformProperty(const QString &name,
                                  KisCurveOption *option,
                                  KisPaintOpSettingsRestrictedSP settings,
                                  QObject *parent);
    ~KisCurveOptionUniformProperty() override;

    void readValueImpl() override;
    void writeValueImpl() override;

    bool isVisible() const override;

private:
    QScopedPointer<KisCurveOption> m_option;
};

#endif /* __KIS_CURVE_OPTION_UNIFORM_PROPERTY_H */
