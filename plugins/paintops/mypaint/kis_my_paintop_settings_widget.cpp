#include "kis_my_paintop_settings_widget.h"

#include "kis_my_paintop_settings.h"

#include <kis_color_option.h>
#include <kis_paintop_settings_widget.h>
#include <kis_paint_action_type_option.h>

#include <kis_pressure_rotation_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_rate_option.h>
#include <kis_curve_option_widget.h>
#include <kis_brush_option_widget.h>
#include <kis_compositeop_option.h>
#include <kis_my_paintop_option.h>

KisMyPaintOpSettingsWidget:: KisMyPaintOpSettingsWidget(QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
{
    addPaintOpOption(new KisMyPaintOpOption(), i18n("Basic"));
}

KisMyPaintOpSettingsWidget::~ KisMyPaintOpSettingsWidget()
{
}

KisPropertiesConfigurationSP  KisMyPaintOpSettingsWidget::configuration() const
{
    KisMyPaintOpSettings* config = new KisMyPaintOpSettings();
    config->setOptionsWidget(const_cast<KisMyPaintOpSettingsWidget*>(this));
    config->setProperty("paintop", "mypaintbrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}
