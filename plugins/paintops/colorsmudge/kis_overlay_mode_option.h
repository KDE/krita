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
#include <QVBoxLayout>
class KisOverlayModeOptionWidget: public KisOverlayModeOption
{
public:
    KisOverlayModeOptionWidget() {
        QWidget *widget = new QWidget();

        m_label = new QLabel(
            i18n("Paints on the current layer\n"
                 "but uses all layers that are currently visible for smudge input\n"
                 "NOTE: This mode is only able to work correctly with a fully opaque background"),
            widget);

        m_label->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);


        m_disabledWarningLabel = new QLabel(
            i18n("Disabled: overlay mode is not supported in Lightness mode of the brush"),
            widget);

        m_disabledWarningLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);


        QVBoxLayout *layout = new QVBoxLayout(widget);
        layout->addWidget(m_disabledWarningLabel);
        layout->addWidget(m_label, 1);

        m_disabledWarningLabel->setVisible(false);

        setConfigurationPage(widget);
    }

    void setEnabled(bool value) {
        m_disabledWarningLabel->setVisible(!value);
        m_label->setEnabled(value);
    }

    QLabel *m_label;
    QLabel *m_disabledWarningLabel;
};

#endif // _KIS_OVERLAYMODE_OPTION_H_
