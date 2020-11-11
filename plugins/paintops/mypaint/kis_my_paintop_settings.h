#ifndef KIS_MY_PAINTOP_SETTINGS_H_
#define KIS_MY_PAINTOP_SETTINGS_H_

#include <QScopedPointer>

#include <brushengine/kis_no_size_paintop_settings.h>
#include <kis_types.h>

#include <kis_outline_generation_policy.h>
#include "kis_my_paintop_settings_widget.h"


class KisMyPaintOpSettings : public KisOutlineGenerationPolicy<KisPaintOpSettings>
{
public:
    KisMyPaintOpSettings();
    ~KisMyPaintOpSettings() override;

    void setPaintOpSize(qreal value) override;
    qreal paintOpSize() const override;

    QPainterPath brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom) override;

    QString modelName() const override {
        return "airbrush";
    }

    bool paintIncremental() override;

protected:

    QList<KisUniformPaintOpPropertySP> uniformProperties(KisPaintOpSettingsSP settings) override;

private:
    Q_DISABLE_COPY(KisMyPaintOpSettings)

    struct Private;
    const QScopedPointer<Private> m_d;

};

typedef KisSharedPtr<KisMyPaintOpSettings> KisMyPaintOpSettingsSP;

#endif
