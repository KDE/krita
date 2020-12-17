/*
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KIS_OVERLAYMODE_OPTION_H_
#define _KIS_OVERLAYMODE_OPTION_H_

#include <QLabel>

#include <kis_paintop_option.h>

#include <brushengine/kis_paintop_lod_limitations.h>


class KisOverlayModeOption : public KisPaintOpOption
{
public:
    KisOverlayModeOption():
        KisPaintOpOption(KisPaintOpOption::GENERAL, false)
    {
        setObjectName("KisOverlayModeOption");
    }

    bool isCheckable() const override {
        return true;
    }

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override {
        setting->setProperty("MergedPaint", isChecked());
    }

    void readOptionSetting(const KisPropertiesConfigurationSP setting) override {
        bool enabled = setting->getBool("MergedPaint");
        setChecked(enabled);
    }

    void lodLimitations(KisPaintopLodLimitations *l) const override {
        l->blockers << KoID("colorsmudge-overlay", i18nc("PaintOp instant preview limitation", "Overlay Option"));
    }

};

class KisOverlayModeOptionWidget: public KisOverlayModeOption
{
public:
    KisOverlayModeOptionWidget() {
        QLabel* label = new QLabel(
            i18n("Paints on the current layer\n\
            but uses all layers that are currently visible for smudge input\n\
            NOTE: This mode is only able to work correctly with a fully opaque background")
        );

        label->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        setConfigurationPage(label);
    }
};

#endif // _KIS_OVERLAYMODE_OPTION_H_
