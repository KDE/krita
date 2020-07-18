#ifndef KIS_MYPAINTOP_SETTINGS_WIDGET_H_
#define KIS_MYPAINTOP_SETTINGS_WIDGET_H_

#include <kis_paintop_settings_widget.h>
#include <kis_my_paintop_option.h>
#include <kis_mypaint_curve_option_widget.h>

class KisMyPaintOpSettingsWidget : public KisPaintOpSettingsWidget
{
    Q_OBJECT

public:
    KisMyPaintOpSettingsWidget(QWidget* parent = 0);
    ~KisMyPaintOpSettingsWidget() override;

    KisPropertiesConfigurationSP configuration() const override;
    void refreshBaseOption();

protected:
    void showEvent(QShowEvent *event) override;

private:
    KisMyPaintOpOption *m_baseOption;
    KisMyPaintCurveOptionWidget *m_radiusWidget;
    KisMyPaintCurveOptionWidget *m_hardnessWidget;
    KisMyPaintCurveOptionWidget *m_opacityWidget;

public Q_SLOTS:
    void updateBaseOptionRadius(qreal);
    void updateBaseOptionHardness(qreal);
    void updateBaseOptionOpacity(qreal);

    void updateRadiusOptionOpacity(qreal);
    void updateHardnessOptionOpacity(qreal);
    void updateOpacityOptionOpacity(qreal);
};

#endif
