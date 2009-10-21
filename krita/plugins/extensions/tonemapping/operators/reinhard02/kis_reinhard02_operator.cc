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

#include "kis_reinhard02_operator.h"
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceTraits.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpace.h>

#include <kis_paint_device.h>
#include <kis_properties_configuration.h>

#include <kis_tone_mapping_operator_configuration_widget.h>
#include "kis_tone_mapping_operators_registry.h"

#include <kis_array2d.h>

#include "tmo_reinhard02.h"
#include "ui_reinhard02_configuration_widget.h"

class KisReinhard02OperatorConfigurationWidget : public KisToneMappingOperatorConfigurationWidget
{
public:
    KisReinhard02OperatorConfigurationWidget(QWidget* wdg) : KisToneMappingOperatorConfigurationWidget(wdg) {
        widget.setupUi(this);
    }
    virtual void setConfiguration(const KisPropertiesConfiguration* config) {
        widget.scales->setChecked(config->getBool("Scales", false));
        widget.key->setValue(config->getDouble("Key", 0.18));
        widget.phi->setValue(config->getDouble("Phi", 1.0));
        widget.range->setValue(config->getDouble("Range", 8.0));
        widget.lower->setValue(config->getDouble("Lower", 1.0));
        widget.upper->setValue(config->getDouble("Upper", 43.0));
        widget.timeCoherent->setChecked(config->getBool("TimeCoherent", false));
    }
    virtual KisPropertiesConfiguration* configuration() const {
        KisPropertiesConfiguration* config = new KisPropertiesConfiguration();
        config->setProperty("Scales", widget.scales->isChecked());
        config->setProperty("Key", widget.key->value());
        config->setProperty("Phi", widget.phi->value());
        config->setProperty("Range", widget.range->value());
        config->setProperty("Lower", widget.lower->value());
        config->setProperty("Upper", widget.upper->value());
        config->setProperty("TimeCoherent", widget.timeCoherent->isChecked());
        return config;
    }
private:
    Ui_Reinhard02OperatorConfigurationWidget widget;
};

PUBLISH_OPERATOR(KisReinhard02Operator)

KisReinhard02Operator::KisReinhard02Operator() : KisToneMappingOperator("reinhard02", i18n("Reinhard 02"))
{
}

KisToneMappingOperatorConfigurationWidget* KisReinhard02Operator::createConfigurationWidget(QWidget* wdg) const
{
    return new KisReinhard02OperatorConfigurationWidget(wdg);
}

const KoColorSpace* KisReinhard02Operator::colorSpace() const
{
    return KoColorSpaceRegistry::instance()->colorSpace(KoColorSpaceRegistry::instance()->colorSpaceId(XYZAColorModelID, Float32BitsColorDepthID), "");
}

void KisReinhard02Operator::toneMap(KisPaintDeviceSP device, KisPropertiesConfiguration* config) const
{
    Q_ASSERT(*device->colorSpace() == *colorSpace());
    QRect r = device->exactBounds();
    dbgKrita << "Tonemaping with Reinhard02 operator on " << r;

    pfs::Array2DImpl Y(r, KoXyzTraits<float>::y_pos, device);

    pfs::Array2DImpl L(r.width(), r.height());
    dbgKrita << "tmo_ashikhmin02";
    tmo_reinhard02(&Y, &L, config->getBool("Scales", false),
                   config->getDouble("Key", 0.18),
                   config->getDouble("Phi", 1.00),
                   config->getDouble("Range", 8.0),
                   config->getDouble("Lower", 1.0),
                   config->getDouble("Upper", 43.0),
                   config->getBool("TimeCoherent", false));
    dbgKrita << "Apply luminance";
    applyLuminance(device, L.device(), r);
    dbgKrita << "Done";
}
