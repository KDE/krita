/*
 *  Copyright (c) 2011 Silvio Heinrich <plassy@web.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef _KIS_OVERLAYMODE_OPTION_H_
#define _KIS_OVERLAYMODE_OPTION_H_

#include <QLabel>
#include <QLayout>
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

        QWidget* mainWidget = new QWidget();

        QLabel* label = new QLabel(
                i18n("Paints on the current layer but uses all layers that are currently visible for smudge input.")
            );
        label->setWordWrap(true);

        QLabel* label2 = new QLabel(
                i18n("NOTE: This mode is only able to work correctly with a fully opaque background")
            );
         label2->setWordWrap(true);

         QVBoxLayout* verticalLayout = new QVBoxLayout();
         verticalLayout->addWidget(label);
         verticalLayout->addWidget(label2);

         verticalLayout->addStretch(1); // pushes things to the top

         mainWidget->setLayout(verticalLayout);
         setConfigurationPage(mainWidget);
    }
};

#endif // _KIS_OVERLAYMODE_OPTION_H_
