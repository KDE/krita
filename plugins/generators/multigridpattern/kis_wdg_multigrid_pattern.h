/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2020 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_WDG_MULTIGRID_PATTERN_H
#define KIS_WDG_MULTIGRID_PATTERN_H

#include <kis_config_widget.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoStopGradient.h>

class Ui_WdgMultigridPatternOptions;

class KisWdgMultigridPattern : public KisConfigWidget
{
    Q_OBJECT
public:
    KisWdgMultigridPattern(QWidget* parent = 0, const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8());
    ~KisWdgMultigridPattern() override;
public:
    inline const Ui_WdgMultigridPatternOptions* widget() const {
        return m_widget;
    }
    void setConfiguration(const KisPropertiesConfigurationSP) override;
    KisPropertiesConfigurationSP configuration() const override;
private:
    Ui_WdgMultigridPatternOptions* m_widget;
    const KoColorSpace *m_cs;
    KoStopGradientSP m_gradient;
};

#endif

