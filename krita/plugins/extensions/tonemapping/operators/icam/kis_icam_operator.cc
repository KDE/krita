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

#include "kis_icam_operator.h"
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceTraits.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpace.h>

#include <kis_paint_device.h>
#include <kis_properties_configuration.h>

#include <kis_tone_mapping_operator_configuration_widget.h>
#include "kis_tone_mapping_operators_registry.h"

#include <kis_array2d.h>

#include "tmo_icam.h"
#include "ui_icam_configuration_widget.h"

class KisIcamOperatorConfigurationWidget : public KisToneMappingOperatorConfigurationWidget
{
public:
    KisIcamOperatorConfigurationWidget(QWidget* wdg) : KisToneMappingOperatorConfigurationWidget(wdg) {
        widget.setupUi(this);
    }
    virtual void setConfiguration(const KisPropertiesConfiguration* config) {
        widget.independence->setChecked(config->getBool("Independence", false));
        widget.variance->setValue(config->getDouble("Variance", -0.10));
        widget.variance2->setValue(config->getDouble("Variance2", -0.30));
        widget.d->setValue(config->getDouble("D", 0.10));
        widget.percentile->setValue(config->getDouble("Percentile", 1000.0));
    }
    virtual KisPropertiesConfiguration* configuration() const {
        KisPropertiesConfiguration* config = new KisPropertiesConfiguration();
        config->setProperty("Independence", widget.independence->isChecked());
        config->setProperty("Variance", widget.variance->value());
        config->setProperty("Variance2", widget.variance2->value());
        config->setProperty("D", widget.d->value());
        config->setProperty("Percentile", widget.percentile->value());
        return config;
    }
private:
    Ui_IcamOperatorConfigurationWidget widget;
};

PUBLISH_OPERATOR(KisIcamOperator)

KisIcamOperator::KisIcamOperator() : KisToneMappingOperator("icam", i18n("Icam"))
{
}

KisToneMappingOperatorConfigurationWidget* KisIcamOperator::createConfigurationWidget(QWidget* wdg) const
{
    return new KisIcamOperatorConfigurationWidget(wdg);
}

const KoColorSpace* KisIcamOperator::colorSpace() const
{
    return KoColorSpaceRegistry::instance()->colorSpace(XYZAColorModelID.id(), Float32BitsColorDepthID.id(), "");
}

void KisIcamOperator::toneMap(KisPaintDeviceSP device, KisPropertiesConfiguration* config) const
{
    Q_ASSERT(*device->colorSpace() == *colorSpace());
    QRect r = device->exactBounds();
    dbgKrita << "Tonemaping with Icam operator on " << r;

    pfs::Array2DImpl Y(r, KoXyzTraits<float>::y_pos, device);

    pfs::Array2DImpl L(r.width(), r.height());
    dbgKrita << "tmo_ashikhmin02";
    icam::tmo_icam(&Y, &L,
                   config->getDouble("Variance", -0.1),
                   config->getDouble("Variance2", -0.3),
                   config->getDouble("D", 0.1),
                   config->getDouble("Prescaling", 1000.0),
                   config->getDouble("Percentile", 0.01),
                   config->getBool("Independence", false));
    dbgKrita << "Apply luminance";
    applyLuminance(device, L.device(), r);
    dbgKrita << "Done";
}
