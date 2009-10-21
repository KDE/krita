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

#include "kis_trilateral_operator.h"
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceTraits.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpace.h>

#include <kis_paint_device.h>
#include <kis_properties_configuration.h>

#include <kis_tone_mapping_operator_configuration_widget.h>
#include "kis_tone_mapping_operators_registry.h"

#include <kis_array2d.h>

#include "tmo_trilateral.h"
#include "ui_trilateral_configuration_widget.h"

class KisTrilateralOperatorConfigurationWidget : public KisToneMappingOperatorConfigurationWidget
{
public:
    KisTrilateralOperatorConfigurationWidget(QWidget* wdg) : KisToneMappingOperatorConfigurationWidget(wdg) {
        widget.setupUi(this);
    }
    virtual void setConfiguration(const KisPropertiesConfiguration* config) {
        widget.saturation->setValue(config->getDouble("Saturation", 1.0));
        widget.sigma->setValue(config->getDouble("Sigma", 21.0));
        widget.contrast->setValue(config->getDouble("Contrast", 5.0));
        widget.shift->setValue(config->getDouble("Shift", 0.0));
    }
    virtual KisPropertiesConfiguration* configuration() const {
        KisPropertiesConfiguration* config = new KisPropertiesConfiguration();
        config->setProperty("Saturation", widget.saturation->value());
        config->setProperty("Sigma", widget.sigma->value());
        config->setProperty("Contrast", widget.contrast->value());
        config->setProperty("Shift", widget.shift->value());
        return config;
    }
private:
    Ui_TrilateralOperatorConfigurationWidget widget;
};

PUBLISH_OPERATOR(KisTrilateralOperator)

KisTrilateralOperator::KisTrilateralOperator() : KisToneMappingOperator("trilateral", i18n("Trilateral"))
{
}

KisToneMappingOperatorConfigurationWidget* KisTrilateralOperator::createConfigurationWidget(QWidget* wdg) const
{
    return new KisTrilateralOperatorConfigurationWidget(wdg);
}

const KoColorSpace* KisTrilateralOperator::colorSpace() const
{
    return KoColorSpaceRegistry::instance()->colorSpace(KoColorSpaceRegistry::instance()->colorSpaceId(XYZAColorModelID, Float32BitsColorDepthID), "");
}

void KisTrilateralOperator::toneMap(KisPaintDeviceSP device, KisPropertiesConfiguration* config) const
{
    Q_ASSERT(*device->colorSpace() == *colorSpace());
    QRect r = device->exactBounds();
    dbgKrita << "Tonemaping with Trilateral operator on " << r;

    pfs::Array2DImpl Y(r, KoXyzTraits<float>::y_pos, device);

    pfs::Array2DImpl L(r.width(), r.height());
    dbgKrita << "tmo_ashikhmin02";
    tmo_trilateral(&Y, &L,
                   config->getDouble("Contrast", 5.0),
                   config->getDouble("Sigma", 21.0),
                   config->getDouble("Shift", 0.0),
                   config->getDouble("Saturation", 1.0));
    dbgKrita << "Apply luminance";
    applyLuminance(device, L.device(), r);
    dbgKrita << "Done";
}
