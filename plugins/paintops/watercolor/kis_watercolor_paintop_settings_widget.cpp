#include "kis_watercolor_paintop_settings_widget.h"

#include "kis_watercolorop_option.h"
#include "kis_watercolor_paintop_settings.h"

#include <kis_compositeop_option.h>

KisWatercolorPaintOpSettingsWidget::KisWatercolorPaintOpSettingsWidget(QWidget *parent)
    : KisPaintOpSettingsWidget(parent)
{

    addPaintOpOption(new KisWatercolorOpOption(), i18n("Watercolor option"));
    addPaintOpOption(new KisCompositeOpOption(true), i18n("Blending Mode"));
}

KisPropertiesConfigurationSP KisWatercolorPaintOpSettingsWidget::configuration() const
{
    KisWatercolorPaintOpSettings* config = new KisWatercolorPaintOpSettings();
    config->setOptionsWidget(const_cast<KisWatercolorPaintOpSettingsWidget*>(this));
    config->setProperty("paintop", "watercolorbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}
