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
#include <QUndoCommand>

#include "kis_reinhard05_operator.h"
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceTraits.h>
#include <KoColorModelStandardIds.h>

#include <kis_types.h>
#include <kis_paint_device.h>
#include <kis_properties_configuration.h>

#include <kis_tone_mapping_operator_configuration_widget.h>
#include "kis_tone_mapping_operators_registry.h"

#include <kis_array2d.h>

#include "tmo_reinhard05.h"
#include "ui_reinhard05_configuration_widget.h"

class KisReinhard05OperatorConfigurationWidget : public KisToneMappingOperatorConfigurationWidget
{
public:
    KisReinhard05OperatorConfigurationWidget(QWidget* wdg) : KisToneMappingOperatorConfigurationWidget(wdg) {
        widget.setupUi(this);
    }
    virtual void setConfiguration(const KisPropertiesConfiguration* config) {
        widget.brightness->setValue(config->getDouble("Brightness", 0.0));
        widget.chromatic->setValue(config->getDouble("Chromatic", 0.0));
        widget.light->setValue(config->getDouble("Light", 1.0));
    }
    virtual KisPropertiesConfiguration* configuration() const {
        KisPropertiesConfiguration* config = new KisPropertiesConfiguration();
        config->setProperty("Brightness", widget.brightness->value());
        config->setProperty("Chromatic", widget.chromatic->value());
        config->setProperty("Light", widget.light->value());
        return config;
    }
private:
    Ui_Reinhard05OperatorConfigurationWidget widget;
};

PUBLISH_OPERATOR(KisReinhard05Operator)

KisReinhard05Operator::KisReinhard05Operator() : KisToneMappingOperator("reinhard05", i18n("Reinhard 05"))
{
}

KisToneMappingOperatorConfigurationWidget* KisReinhard05Operator::createConfigurationWidget(QWidget* wdg) const
{
    return new KisReinhard05OperatorConfigurationWidget(wdg);
}

const KoColorSpace* KisReinhard05Operator::colorSpace() const
{
    return KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), "");
}

void KisReinhard05Operator::toneMap(KisPaintDeviceSP device, KisPropertiesConfiguration* config) const
{
    dbgKrita << "Create a copy of the paint device as XYZAF32";
    QRect r = device->exactBounds();
    const KoColorSpace* XYZACs = KoColorSpaceRegistry::instance()->colorSpace(XYZAColorModelID.id(), Float32BitsColorDepthID.id(), "");
    Q_ASSERT(XYZACs);
    KisPaintDeviceSP deviceXYZ = new KisPaintDevice(*device);
    QUndoCommand* cmd = deviceXYZ->convertTo(XYZACs);
    delete cmd; // XXX: make undo possible?
    dbgKrita << "Tone map using reinhard05";
    pfs::Array2DImpl Y(r, KoXyzTraits<float>::y_pos, device);
    pfs::Array2DImpl R(r, KoRgbTraits<float>::red_pos, device);
    pfs::Array2DImpl G(r, KoRgbTraits<float>::green_pos, device);
    pfs::Array2DImpl B(r, KoRgbTraits<float>::blue_pos, device);
    tmo_reinhard05(&R, &G, &B, &Y, config->getDouble("Brightness", 0.0), config->getDouble("Chromatic", 0.0), config->getDouble("Light", 1.0));
    dbgKrita << "Done !";
}
