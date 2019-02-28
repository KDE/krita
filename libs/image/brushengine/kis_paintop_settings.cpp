/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2014 Mohit Goyal <mohit.bits2011@gmail.com>
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

#include <brushengine/kis_paintop_settings.h>

#include <QImage>
#include <QColor>
#include <QPointer>

#include <KoPointerEvent.h>
#include <KoColor.h>
#include <KoCompositeOpRegistry.h>
#include <KoViewConverter.h>

#include "kis_paint_layer.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_paint_device.h"
#include "kis_paintop_registry.h"
#include "kis_timing_information.h"
#include <brushengine/kis_paint_information.h>
#include "kis_paintop_config_widget.h"
#include <brushengine/kis_paintop_preset.h>
#include "kis_paintop_settings_update_proxy.h"
#include <time.h>
#include <kis_types.h>
#include <kis_signals_blocker.h>

#include <brushengine/kis_locked_properties_server.h>
#include <brushengine/kis_locked_properties_proxy.h>

#include "KisPaintopSettingsIds.h"


struct Q_DECL_HIDDEN KisPaintOpSettings::Private {
    Private()
        : disableDirtyNotifications(false)
    {}

    QPointer<KisPaintOpConfigWidget> settingsWidget;
    QString modelName;
    KisPaintOpPresetWSP preset;
    QList<KisUniformPaintOpPropertyWSP> uniformProperties;

    bool disableDirtyNotifications;

    class DirtyNotificationsLocker {
    public:
        DirtyNotificationsLocker(KisPaintOpSettings::Private *d)
            : m_d(d),
              m_oldNotificationsState(d->disableDirtyNotifications)
        {
            m_d->disableDirtyNotifications = true;
        }

        ~DirtyNotificationsLocker() {
            m_d->disableDirtyNotifications = m_oldNotificationsState;
        }

    private:
        KisPaintOpSettings::Private *m_d;
        bool m_oldNotificationsState;
        Q_DISABLE_COPY(DirtyNotificationsLocker)
    };

    KisPaintopSettingsUpdateProxy* updateProxyNoCreate() const {
        auto presetSP = preset.toStrongRef();
        return presetSP ? presetSP->updateProxyNoCreate() : 0;
    }

    KisPaintopSettingsUpdateProxy* updateProxyCreate() const {
        auto presetSP = preset.toStrongRef();
        return presetSP ? presetSP->updateProxy() : 0;
    }
};


KisPaintOpSettings::KisPaintOpSettings()
    : d(new Private)
{
    d->preset = 0;
}

KisPaintOpSettings::~KisPaintOpSettings()
{
}

KisPaintOpSettings::KisPaintOpSettings(const KisPaintOpSettings &rhs)
    : KisPropertiesConfiguration(rhs)
    , d(new Private)
{
    d->settingsWidget = 0;
    d->preset = rhs.preset();
    d->modelName = rhs.modelName();
}

void KisPaintOpSettings::setOptionsWidget(KisPaintOpConfigWidget* widget)
{
    d->settingsWidget = widget;
}
void KisPaintOpSettings::setPreset(KisPaintOpPresetWSP preset)
{
    d->preset = preset;
}
KisPaintOpPresetWSP KisPaintOpSettings::preset() const
{
    return d->preset;
}

bool KisPaintOpSettings::mousePressEvent(const KisPaintInformation &paintInformation, Qt::KeyboardModifiers modifiers, KisNodeWSP currentNode)
{
    Q_UNUSED(modifiers);
    Q_UNUSED(currentNode);
    setRandomOffset(paintInformation);
    return true; // ignore the event by default
}

void KisPaintOpSettings::setRandomOffset(const KisPaintInformation &paintInformation)
{
    if (getBool("Texture/Pattern/Enabled")) {
        if (getBool("Texture/Pattern/isRandomOffsetX")) {
            setProperty("Texture/Pattern/OffsetX",
                        paintInformation.randomSource()->generate(0, KisPropertiesConfiguration::getInt("Texture/Pattern/MaximumOffsetX")));
        }
        if (getBool("Texture/Pattern/isRandomOffsetY")) {
            setProperty("Texture/Pattern/OffsetY",
                        paintInformation.randomSource()->generate(0, KisPropertiesConfiguration::getInt("Texture/Pattern/MaximumOffsetY")));

        }
    }

}

bool KisPaintOpSettings::hasMaskingSettings() const
{
    return getBool(KisPaintOpUtils::MaskingBrushEnabledTag, false);
}

KisPaintOpSettingsSP KisPaintOpSettings::createMaskingSettings() const
{
    if (!hasMaskingSettings()) return KisPaintOpSettingsSP();

    const KoID pixelBrushId(KisPaintOpUtils::MaskingBrushPaintOpId, QString());

    KisPaintOpSettingsSP maskingSettings = KisPaintOpRegistry::instance()->settings(pixelBrushId);
    this->getPrefixedProperties(KisPaintOpUtils::MaskingBrushPresetPrefix, maskingSettings);

    const bool useMasterSize = this->getBool(KisPaintOpUtils::MaskingBrushUseMasterSizeTag, true);
    if (useMasterSize) {
        const qreal masterSizeCoeff = getDouble(KisPaintOpUtils::MaskingBrushMasterSizeCoeffTag, 1.0);
        maskingSettings->setPaintOpSize(masterSizeCoeff * paintOpSize());
    }

    return maskingSettings;
}

QString KisPaintOpSettings::maskingBrushCompositeOp() const
{
    return getString(KisPaintOpUtils::MaskingBrushCompositeOpTag, COMPOSITE_MULT);
}

KisPaintOpSettingsSP KisPaintOpSettings::clone() const
{
    QString paintopID = getString("paintop");
    if (paintopID.isEmpty())
        return 0;

    KisPaintOpSettingsSP settings = KisPaintOpRegistry::instance()->settings(KoID(paintopID));
    QMapIterator<QString, QVariant> i(getProperties());
    while (i.hasNext()) {
        i.next();
        settings->setProperty(i.key(), QVariant(i.value()));
    }
    settings->setPreset(this->preset());
    return settings;
}

void KisPaintOpSettings::resetSettings(const QStringList &preserveProperties)
{
    QStringList allKeys = preserveProperties;
    allKeys << "paintop";

    QHash<QString, QVariant> preserved;
    Q_FOREACH (const QString &key, allKeys) {
        if (hasProperty(key)) {
            preserved[key] = getProperty(key);
        }
    }

    clearProperties();

    for (auto it = preserved.constBegin(); it != preserved.constEnd(); ++it) {
        setProperty(it.key(), it.value());
    }
}

void KisPaintOpSettings::activate()
{
}

void KisPaintOpSettings::setPaintOpOpacity(qreal value)
{
    KisLockedPropertiesProxySP proxy(
                KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(this));

    proxy->setProperty("OpacityValue", value);
}

void KisPaintOpSettings::setPaintOpFlow(qreal value)
{
    KisLockedPropertiesProxySP proxy(
                KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(this));

    proxy->setProperty("FlowValue", value);
}

void KisPaintOpSettings::setPaintOpCompositeOp(const QString &value)
{
    KisLockedPropertiesProxySP proxy(
                KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(this));

    proxy->setProperty("CompositeOp", value);
}

qreal KisPaintOpSettings::paintOpOpacity()
{
    KisLockedPropertiesProxySP proxy = KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(this);

    return proxy->getDouble("OpacityValue", 1.0);
}

qreal KisPaintOpSettings::paintOpFlow()
{
    KisLockedPropertiesProxySP proxy(
                KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(this));

    return proxy->getDouble("FlowValue", 1.0);
}

QString KisPaintOpSettings::paintOpCompositeOp()
{
    KisLockedPropertiesProxySP proxy(
                KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(this));

    return proxy->getString("CompositeOp", COMPOSITE_OVER);
}

void KisPaintOpSettings::setEraserMode(bool value)
{
    KisLockedPropertiesProxySP proxy(
                KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(this));

    proxy->setProperty("EraserMode", value);
}

bool KisPaintOpSettings::eraserMode()
{
    KisLockedPropertiesProxySP proxy(
                KisLockedPropertiesServer::instance()->createLockedPropertiesProxy(this));

    return proxy->getBool("EraserMode", false);
}

QString KisPaintOpSettings::effectivePaintOpCompositeOp()
{
    return !eraserMode() ? paintOpCompositeOp() : COMPOSITE_ERASE;
}

qreal KisPaintOpSettings::savedEraserSize() const
{
    return getDouble("SavedEraserSize", 0.0);
}

void KisPaintOpSettings::setSavedEraserSize(qreal value)
{
    setProperty("SavedEraserSize", value);
    setPropertyNotSaved("SavedEraserSize");
}

qreal KisPaintOpSettings::savedBrushSize() const
{
    return getDouble("SavedBrushSize", 0.0);
}

void KisPaintOpSettings::setSavedBrushSize(qreal value)
{
    setProperty("SavedBrushSize", value);
    setPropertyNotSaved("SavedBrushSize");
}

qreal KisPaintOpSettings::savedEraserOpacity() const
{
    return getDouble("SavedEraserOpacity", 0.0);
}

void KisPaintOpSettings::setSavedEraserOpacity(qreal value)
{
    setProperty("SavedEraserOpacity", value);
    setPropertyNotSaved("SavedEraserOpacity");
}

qreal KisPaintOpSettings::savedBrushOpacity() const
{
    return getDouble("SavedBrushOpacity", 0.0);
}

void KisPaintOpSettings::setSavedBrushOpacity(qreal value)
{
    setProperty("SavedBrushOpacity", value);
    setPropertyNotSaved("SavedBrushOpacity");
}

QString KisPaintOpSettings::modelName() const
{
    return d->modelName;
}

void KisPaintOpSettings::setModelName(const QString & modelName)
{
    d->modelName = modelName;
}

KisPaintOpConfigWidget* KisPaintOpSettings::optionsWidget() const
{
    if (d->settingsWidget.isNull())
        return 0;

    return d->settingsWidget.data();
}

bool KisPaintOpSettings::isValid() const
{
    return true;
}

bool KisPaintOpSettings::isLoadable()
{
    return isValid();
}

QString KisPaintOpSettings::indirectPaintingCompositeOp() const
{
    return COMPOSITE_ALPHA_DARKEN;
}

bool KisPaintOpSettings::isAirbrushing() const
{
    return getBool(AIRBRUSH_ENABLED, false);
}

qreal KisPaintOpSettings::airbrushInterval() const
{
    qreal rate = getDouble(AIRBRUSH_RATE, 1.0);
    if (rate == 0.0) {
        return LONG_TIME;
    }
    else {
        return 1000.0 / rate;
    }
}

bool KisPaintOpSettings::useSpacingUpdates() const
{
    return getBool(SPACING_USE_UPDATES, false);
}

bool KisPaintOpSettings::needsAsynchronousUpdates() const
{
    return false;
}

QPainterPath KisPaintOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode)
{
    QPainterPath path;
    if (mode.isVisible) {
        path = ellipseOutline(10, 10, 1.0, 0);

        if (mode.showTiltDecoration) {
            path.addPath(makeTiltIndicator(info, QPointF(0.0, 0.0), 0.0, 2.0));
        }

        path.translate(info.pos());
    }

    return path;
}

QPainterPath KisPaintOpSettings::ellipseOutline(qreal width, qreal height, qreal scale, qreal rotation)
{
    QPainterPath path;
    QRectF ellipse(0, 0, width * scale, height * scale);
    ellipse.translate(-ellipse.center());
    path.addEllipse(ellipse);

    QTransform m;
    m.reset();
    m.rotate(rotation);
    path = m.map(path);
    return path;
}

QPainterPath KisPaintOpSettings::makeTiltIndicator(KisPaintInformation const& info,
                                                   QPointF const& start, qreal maxLength, qreal angle)
{
    if (maxLength == 0.0) maxLength = 50.0;
    maxLength = qMax(maxLength, 50.0);
    qreal const length = maxLength * (1 - info.tiltElevation(info, 60.0, 60.0, true));
    qreal const baseAngle = 360.0 - fmod(KisPaintInformation::tiltDirection(info, true) * 360.0 + 270.0, 360.0);

    QLineF guideLine = QLineF::fromPolar(length, baseAngle + angle);
    guideLine.translate(start);
    QPainterPath ret;
    ret.moveTo(guideLine.p1());
    ret.lineTo(guideLine.p2());
    guideLine.setAngle(baseAngle - angle);
    ret.lineTo(guideLine.p2());
    ret.lineTo(guideLine.p1());
    return ret;
}

void KisPaintOpSettings::setCanvasRotation(qreal angle)
{
    Private::DirtyNotificationsLocker locker(d.data());

    setProperty("runtimeCanvasRotation", angle);
    setPropertyNotSaved("runtimeCanvasRotation");
}

void KisPaintOpSettings::setCanvasMirroring(bool xAxisMirrored, bool yAxisMirrored)
{
    Private::DirtyNotificationsLocker locker(d.data());

    setProperty("runtimeCanvasMirroredX", xAxisMirrored);
    setPropertyNotSaved("runtimeCanvasMirroredX");

    setProperty("runtimeCanvasMirroredY", yAxisMirrored);
    setPropertyNotSaved("runtimeCanvasMirroredY");
}

void KisPaintOpSettings::setProperty(const QString & name, const QVariant & value)
{
    if (value != KisPropertiesConfiguration::getProperty(name) &&
            !d->disableDirtyNotifications) {
        KisPaintOpPresetSP presetSP = preset().toStrongRef();
        if (presetSP) {
            presetSP->setDirty(true);
        }
    }

    KisPropertiesConfiguration::setProperty(name, value);
    onPropertyChanged();
}


void KisPaintOpSettings::onPropertyChanged()
{
    KisPaintopSettingsUpdateProxy *proxy = d->updateProxyNoCreate();

    if (proxy) {
        proxy->notifySettingsChanged();
    }
}

bool KisPaintOpSettings::isLodUserAllowed(const KisPropertiesConfigurationSP config)
{
    return config->getBool("lodUserAllowed", true);
}

void KisPaintOpSettings::setLodUserAllowed(KisPropertiesConfigurationSP config, bool value)
{
    config->setProperty("lodUserAllowed", value);
}

bool KisPaintOpSettings::lodSizeThresholdSupported() const
{
    return true;
}

qreal KisPaintOpSettings::lodSizeThreshold() const
{
    return getDouble("lodSizeThreshold", 100.0);
}

void KisPaintOpSettings::setLodSizeThreshold(qreal value)
{
    setProperty("lodSizeThreshold", value);
}

#include "kis_standard_uniform_properties_factory.h"

QList<KisUniformPaintOpPropertySP> KisPaintOpSettings::uniformProperties(KisPaintOpSettingsSP settings)
{
    QList<KisUniformPaintOpPropertySP> props =
            listWeakToStrong(d->uniformProperties);


    if (props.isEmpty()) {
        using namespace KisStandardUniformPropertiesFactory;

        props.append(createProperty(opacity, settings, d->updateProxyCreate()));
        props.append(createProperty(size, settings, d->updateProxyCreate()));
        props.append(createProperty(flow, settings, d->updateProxyCreate()));

        d->uniformProperties = listStrongToWeak(props);
    }

    return props;
}
