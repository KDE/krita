/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-only
 */
#ifndef KISTONEGAMUTMAPFILTER_H
#define KISTONEGAMUTMAPFILTER_H

#include <filter/kis_color_transformation_filter.h>
#include <kis_config_widget.h>

class KisToneGamutMapFilterPlugin : public QObject
{
    Q_OBJECT
public:
    KisToneGamutMapFilterPlugin(QObject *parent, const QVariantList &);
    ~KisToneGamutMapFilterPlugin() override;
};

class KisToneGamutMapFilter: public KisColorTransformationFilter
{
public:
    KisToneGamutMapFilter();

    KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const override;

    static inline KoID id() {
        return KoID("tone_gamut_map", i18n("Tone/Gamut Map"));
    }

    KisFilterConfigurationSP defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const override;
    KisConfigWidget *createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, bool useForMasks) const override;
};

namespace Ui
{
class KisToneGamutMapWidget;
}

class KisToneGamutMapWidget : public KisConfigWidget
{
    Q_OBJECT

public:
    explicit KisToneGamutMapWidget(QWidget *parent, KisPaintDeviceSP dev);
    ~KisToneGamutMapWidget();

    void setConfiguration(const KisPropertiesConfigurationSP config);
    KisPropertiesConfigurationSP configuration() const;
private:
    Ui::KisToneGamutMapWidget *ui;

};

#endif // KISTONEGAMUTMAPFILTER_H
