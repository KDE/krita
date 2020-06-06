#ifndef KIS_MYPAINTOP_SETTINGS_WIDGET_H_
#define KIS_MYPAINTOP_SETTINGS_WIDGET_H_

#include <kis_paintop_settings_widget.h>

class KisMyPaintOpSettingsWidget : public KisPaintOpSettingsWidget
{
    Q_OBJECT

public:
    KisMyPaintOpSettingsWidget(QWidget* parent = 0);
    ~KisMyPaintOpSettingsWidget() override;

    KisPropertiesConfigurationSP configuration() const override;

};

#endif
