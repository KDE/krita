/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_derived_resources.h"

#include "kis_signal_auto_connection.h"
#include "kis_canvas_resource_provider.h"
#include "kis_paintop_preset.h"
#include "kis_paintop_settings.h"
#include "kis_paintop_settings_update_proxy.h"

struct KisPresetUpdateMediator::Private
{
    KisSignalAutoConnectionsStore connections;
};

KisPresetUpdateMediator::KisPresetUpdateMediator()
    : KoResourceUpdateMediator(KisCanvasResourceProvider::CurrentPaintOpPreset),
      m_d(new Private)
{
}

KisPresetUpdateMediator::~KisPresetUpdateMediator()
{
}

void KisPresetUpdateMediator::connectResource(QVariant sourceResource)
{
    KisPaintOpPresetSP preset = sourceResource.value<KisPaintOpPresetSP>();
    if (!preset) return;

    m_d->connections.clear();
    m_d->connections.addUniqueConnection(
        preset->updateProxy(),
        SIGNAL(sigSettingsChanged()),
        this,
        SLOT(slotSettingsChanged()));
}

void KisPresetUpdateMediator::slotSettingsChanged()
{
    emit sigResourceChanged(key());
}


/*********************************************************************/
/*          KisCompositeOpResourceConverter                          */
/*********************************************************************/


KisCompositeOpResourceConverter::KisCompositeOpResourceConverter()
    : KoDerivedResourceConverter(KisCanvasResourceProvider::CurrentCompositeOp,
                                 KisCanvasResourceProvider::CurrentPaintOpPreset)
{
}

QVariant KisCompositeOpResourceConverter::fromSource(const QVariant &value)
{
    KisPaintOpPresetSP preset = value.value<KisPaintOpPresetSP>();
    return preset ? preset->settings()->paintOpCompositeOp() : QVariant();
}

QVariant KisCompositeOpResourceConverter::toSource(const QVariant &value, const QVariant &sourceValue)
{
    KisPaintOpPresetSP preset = sourceValue.value<KisPaintOpPresetSP>();
    if (!preset) return sourceValue;

    preset->settings()->setPaintOpCompositeOp(value.toString());
    return QVariant::fromValue(preset);
}

/*********************************************************************/
/*          KisEffectiveCompositeOpResourceConverter                 */
/*********************************************************************/


KisEffectiveCompositeOpResourceConverter::KisEffectiveCompositeOpResourceConverter()
    : KoDerivedResourceConverter(KisCanvasResourceProvider::CurrentEffectiveCompositeOp,
                                 KisCanvasResourceProvider::CurrentPaintOpPreset)
{
}

QVariant KisEffectiveCompositeOpResourceConverter::fromSource(const QVariant &value)
{
    KisPaintOpPresetSP preset = value.value<KisPaintOpPresetSP>();
    return preset ? preset->settings()->effectivePaintOpCompositeOp() : QVariant();
}

QVariant KisEffectiveCompositeOpResourceConverter::toSource(const QVariant &value, const QVariant &sourceValue)
{
    Q_UNUSED(value);

    // WARNING: we don't save that!
    KisPaintOpPresetSP preset = sourceValue.value<KisPaintOpPresetSP>();
    if (!preset) return sourceValue;

    return QVariant::fromValue(preset);
}

/*********************************************************************/
/*          KisOpacityResourceConverter                              */
/*********************************************************************/

KisOpacityResourceConverter::KisOpacityResourceConverter()
    : KoDerivedResourceConverter(KisCanvasResourceProvider::Opacity,
                                 KisCanvasResourceProvider::CurrentPaintOpPreset)
{
}

QVariant KisOpacityResourceConverter::fromSource(const QVariant &value)
{
    KisPaintOpPresetSP preset = value.value<KisPaintOpPresetSP>();
    return preset ? preset->settings()->paintOpOpacity() : QVariant();
}

QVariant KisOpacityResourceConverter::toSource(const QVariant &value, const QVariant &sourceValue)
{
    KisPaintOpPresetSP preset = sourceValue.value<KisPaintOpPresetSP>();
    if (!preset) return sourceValue;

    preset->settings()->setPaintOpOpacity(value.toReal());
    return QVariant::fromValue(preset);
}

/*********************************************************************/
/*          KisFlowResourceConverter                                 */
/*********************************************************************/

KisFlowResourceConverter::KisFlowResourceConverter()
    : KoDerivedResourceConverter(KisCanvasResourceProvider::Flow,
                                 KisCanvasResourceProvider::CurrentPaintOpPreset)
{
}

QVariant KisFlowResourceConverter::fromSource(const QVariant &value)
{
    KisPaintOpPresetSP preset = value.value<KisPaintOpPresetSP>();
    return preset ? preset->settings()->paintOpFlow() : QVariant();
}

QVariant KisFlowResourceConverter::toSource(const QVariant &value, const QVariant &sourceValue)
{
    KisPaintOpPresetSP preset = sourceValue.value<KisPaintOpPresetSP>();
    if (!preset) return sourceValue;

    preset->settings()->setPaintOpFlow(value.toReal());
    return QVariant::fromValue(preset);
}

/*********************************************************************/
/*          KisSizeResourceConverter                                 */
/*********************************************************************/

KisSizeResourceConverter::KisSizeResourceConverter()
    : KoDerivedResourceConverter(KisCanvasResourceProvider::Size,
                                 KisCanvasResourceProvider::CurrentPaintOpPreset)
{
}

QVariant KisSizeResourceConverter::fromSource(const QVariant &value)
{
    KisPaintOpPresetSP preset = value.value<KisPaintOpPresetSP>();
    return preset ? preset->settings()->paintOpSize() : QVariant();
}

QVariant KisSizeResourceConverter::toSource(const QVariant &value, const QVariant &sourceValue)
{
    KisPaintOpPresetSP preset = sourceValue.value<KisPaintOpPresetSP>();
    if (!preset) return sourceValue;

    preset->settings()->setPaintOpSize(value.toReal());
    return QVariant::fromValue(preset);
}

/*********************************************************************/
/*          KisLodAvailabilityResourceConverter                      */
/*********************************************************************/

KisLodAvailabilityResourceConverter::KisLodAvailabilityResourceConverter()
    : KoDerivedResourceConverter(KisCanvasResourceProvider::LodAvailability,
                                 KisCanvasResourceProvider::CurrentPaintOpPreset)
{
}

QVariant KisLodAvailabilityResourceConverter::fromSource(const QVariant &value)
{
    KisPaintOpPresetSP preset = value.value<KisPaintOpPresetSP>();
    return preset ? KisPaintOpSettings::isLodUserAllowed(preset->settings()) : QVariant();
}

QVariant KisLodAvailabilityResourceConverter::toSource(const QVariant &value, const QVariant &sourceValue)
{
    KisPaintOpPresetSP preset = sourceValue.value<KisPaintOpPresetSP>();
    if (!preset) return sourceValue;

    KisPaintOpSettings::setLodUserAllowed(preset->settings().data(), value.toBool());
    return QVariant::fromValue(preset);
}

/*********************************************************************/
/*          KisLodSizeThresholdResourceConverter                     */
/*********************************************************************/

KisLodSizeThresholdResourceConverter::KisLodSizeThresholdResourceConverter()
    : KoDerivedResourceConverter(KisCanvasResourceProvider::LodSizeThreshold,
                                 KisCanvasResourceProvider::CurrentPaintOpPreset)
{
}

QVariant KisLodSizeThresholdResourceConverter::fromSource(const QVariant &value)
{
    KisPaintOpPresetSP preset = value.value<KisPaintOpPresetSP>();
    return preset ? preset->settings()->lodSizeThreshold() : QVariant();
}

QVariant KisLodSizeThresholdResourceConverter::toSource(const QVariant &value, const QVariant &sourceValue)
{
    KisPaintOpPresetSP preset = sourceValue.value<KisPaintOpPresetSP>();
    if (!preset) return sourceValue;

    preset->settings()->setLodSizeThreshold(value.toDouble());
    return QVariant::fromValue(preset);
}

/*********************************************************************/
/*          KisLodSizeThresholdSupportedResourceConverter            */
/*********************************************************************/

KisLodSizeThresholdSupportedResourceConverter::KisLodSizeThresholdSupportedResourceConverter()
    : KoDerivedResourceConverter(KisCanvasResourceProvider::LodSizeThresholdSupported,
                                 KisCanvasResourceProvider::CurrentPaintOpPreset)
{
}

QVariant KisLodSizeThresholdSupportedResourceConverter::fromSource(const QVariant &value)
{
    KisPaintOpPresetSP preset = value.value<KisPaintOpPresetSP>();
    return preset ? preset->settings()->lodSizeThresholdSupported() : QVariant();
}

QVariant KisLodSizeThresholdSupportedResourceConverter::toSource(const QVariant &value, const QVariant &sourceValue)
{
    // this property of the preset is immutable

    Q_UNUSED(value);
    return sourceValue;
}


/*********************************************************************/
/*          KisEraserModeResourceConverter                           */
/*********************************************************************/

KisEraserModeResourceConverter::KisEraserModeResourceConverter()
    : KoDerivedResourceConverter(KisCanvasResourceProvider::EraserMode,
                                 KisCanvasResourceProvider::CurrentPaintOpPreset)
{
}

QVariant KisEraserModeResourceConverter::fromSource(const QVariant &value)
{
    KisPaintOpPresetSP preset = value.value<KisPaintOpPresetSP>();
    return preset ? preset->settings()->eraserMode() : QVariant();
}

QVariant KisEraserModeResourceConverter::toSource(const QVariant &value, const QVariant &sourceValue)
{
    KisPaintOpPresetSP preset = sourceValue.value<KisPaintOpPresetSP>();
    if (!preset) return sourceValue;

    preset->settings()->setEraserMode(value.toBool());
    return QVariant::fromValue(preset);
}
