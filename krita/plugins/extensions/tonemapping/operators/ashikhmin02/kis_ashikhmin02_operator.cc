/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_ashikhmin02_operator.h"
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceTraits.h>

#include <kis_paint_device.h>
#include <kis_properties_configuration.h>

#include <kis_tone_mapping_operator_configuration_widget.h>
#include "kis_tone_mapping_operators_registry.h"

#include <kis_array2d.h>

#include "tmo_ashikhmin02.h"
#include "ui_ashikhmin02_configuration_widget.h"

class KisAshikhmin02OperatorConfigurationWidget : public KisToneMappingOperatorConfigurationWidget
{
public:
    KisAshikhmin02OperatorConfigurationWidget(QWidget* wdg) : KisToneMappingOperatorConfigurationWidget(wdg) {
        widget.setupUi(this);
    }
    virtual void setConfiguration(const KisPropertiesConfiguration* config) {
        widget.simple->setChecked(config->getBool("Simple", false));
        widget.localContrast->setValue(config->getDouble("LocalContrastThreshold", 0.5));
        if (config->getInt("Equation", 2) == 2) {
            widget.equation->setCurrentIndex(0);
        } else {
            widget.equation->setCurrentIndex(1);
        }
    }
    virtual KisPropertiesConfiguration* configuration() const {
        KisPropertiesConfiguration* config = new KisPropertiesConfiguration();
        config->setProperty("Simple", widget.simple->isChecked());
        config->setProperty("LocalContrastThreshold", widget.localContrast->value());
        if (widget.equation->currentIndex() == 0)
            config->setProperty("Equation", 2);
        else
            config->setProperty("Equation", 4);
        return config;
    }
private:
    Ui_Ashikhmin02OperatorConfigurationWidget widget;
};

PUBLISH_OPERATOR(KisAshikhmin02Operator);

KisAshikhmin02Operator::KisAshikhmin02Operator() : KisToneMappingOperator("ashikhminO2", i18n("Ashikhmin 02"))
{
}

KisToneMappingOperatorConfigurationWidget* KisAshikhmin02Operator::createConfigurationWidget(QWidget* wdg) const
{
    return new KisAshikhmin02OperatorConfigurationWidget(wdg);
}

KoColorSpace* KisAshikhmin02Operator::colorSpace() const
{
    return KoColorSpaceRegistry::instance()->colorSpace(KoColorSpaceRegistry::instance()->colorSpaceId(XYZAColorModelID, Float32BitsColorDepthID), "");
}

void KisAshikhmin02Operator::toneMap(KisPaintDeviceSP device, KisPropertiesConfiguration* config) const
{
    QRect r = device->exactBounds();
    dbgKrita << "Tonemaping with Ashikhmin02 operator on " << r;
    bool simple = config->getBool("Simple", false);
    double lC = config->getDouble("LocalContrastThreshold", 0.5);
    int eqn = config->getInt("Equation", 2);
    if (eqn != 2 or eqn != 4) eqn = 2;

    // Compute luminance
    dbgKrita << "Compute luminance";
    double avLum = 0.0;
    double maxLum = 0.0f;
    double minLum = 0.0f;
    {
        KisRectIterator itR = device->createRectIterator(r.x(), r.y(), r.width(), r.height());
        while (not itR.isDone()) {
            KoXyzTraits<float>::Pixel* data = reinterpret_cast< KoXyzTraits<float>::Pixel* >(itR.rawData());
            avLum += log(data->Y + 1e-4);
            maxLum = (data->Y > maxLum) ? data->Y : maxLum ;
            minLum = (data->Y < minLum) ? data->Y : minLum ;
            ++itR;
        }
    }
    avLum = exp(avLum / (r.width() * r.height()));

    pfs::Array2DImpl Y(r, KoXyzTraits<float>::y_pos, device);

    pfs::Array2DImpl L(r.width(), r.height());
    dbgKrita << "tmo_ashikhmin02";
    tmo_ashikhmin02(&Y, &L, maxLum, minLum, avLum, simple, lC, eqn);

    dbgKrita << "Apply luminance";
    applyLuminance(device, L.device(), r);
    dbgKrita << "Finished";
}
