/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 * Copyright (C) Sven Langkamp <sven.langkamp@gmail.com>, (C) 2009
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include <brushengine/kis_paintop_preset.h>

#include <QFile>
#include <QSize>
#include <QImage>
#include <QImageWriter>
#include <QImageReader>
#include <QDomDocument>
#include <QBuffer>

#include <KisResourceDirtyStateSaver.h>

#include <brushengine/kis_paintop_settings.h>
#include "kis_paintop_registry.h"
#include "kis_painter.h"
#include <brushengine/kis_paint_information.h>
#include "kis_paint_device.h"
#include "kis_image.h"
#include "kis_paintop_settings_update_proxy.h"
#include <brushengine/kis_paintop_config_widget.h>
#include <KisRequiredResourcesOperators.h>
#include <KoLocalStrokeCanvasResources.h>

#include <KoStore.h>

struct Q_DECL_HIDDEN KisPaintOpPreset::Private {
    Private(KisPaintOpPreset *q)
    {
        proxyParent = new ProxyParent(q);
    }

    QPointer<ProxyParent> proxyParent{0};
    KisPaintOpSettingsSP settings {0};
    QPointer<KisPaintopSettingsUpdateProxy> updateProxy {0};
};


KisPaintOpPreset::KisPaintOpPreset()
    : KoResource(QString())
    , d(new Private(this))
{
}

KisPaintOpPreset::KisPaintOpPreset(const QString & fileName)
    : KoResource(fileName)
    , d(new Private(this))
{
}

KisPaintOpPreset::~KisPaintOpPreset()
{
    delete d;
}

KisPaintOpPreset::KisPaintOpPreset(const KisPaintOpPreset &rhs)
    : KoResource(rhs)
    , d(new Private(this))
{
    if (rhs.settings()) {
        setSettings(rhs.settings()); // the settings are cloned inside!
    }
    KIS_SAFE_ASSERT_RECOVER_NOOP(isDirty() == rhs.isDirty());
    // only valid if we could clone the settings
    setValid(rhs.settings());

    setPaintOp(rhs.paintOp());
    setName(rhs.name());
    setImage(rhs.image());
}

KoResourceSP KisPaintOpPreset::clone() const
{
    return KoResourceSP(new KisPaintOpPreset(*this));
}

void KisPaintOpPreset::setPaintOp(const KoID & paintOp)
{
    Q_ASSERT(d->settings);
    d->settings->setProperty("paintop", paintOp.id());
}

KoID KisPaintOpPreset::paintOp() const
{
    Q_ASSERT(d->settings);
    return KoID(d->settings->getString("paintop"));
}

void KisPaintOpPreset::setOptionsWidget(KisPaintOpConfigWidget* widget)
{
    if (d->settings) {
        d->settings->setOptionsWidget(widget);

        if (widget) {
            widget->setConfigurationSafe(d->settings);
        }
    }
}

void KisPaintOpPreset::setSettings(KisPaintOpSettingsSP settings)
{
    Q_ASSERT(settings);
    Q_ASSERT(!settings->getString("paintop", QString()).isEmpty());

    KisResourceDirtyStateSaver dirtyStateSaver(this);
    Q_UNUSED(dirtyStateSaver);

    KisPaintOpConfigWidget *oldOptionsWidget = 0;

    if (d->settings) {
        oldOptionsWidget = d->settings->optionsWidget();
        d->settings->setOptionsWidget(0);
        d->settings->setUpdateProxy(0);
        d->settings = 0;
    }

    if (settings) {
        d->settings = settings->clone();
        d->settings->setUpdateProxy(updateProxy());

        if (oldOptionsWidget) {
            oldOptionsWidget->setConfigurationSafe(d->settings);
            d->settings->setOptionsWidget(oldOptionsWidget);
        }
    }

    setValid(d->settings);

    if (d->updateProxy) {
        d->updateProxy->notifyUniformPropertiesChanged();
        d->updateProxy->notifySettingsChanged();
    }
}

KisPaintOpSettingsSP KisPaintOpPreset::settings() const
{
    Q_ASSERT(d->settings);
    Q_ASSERT(!d->settings->getString("paintop", QString()).isEmpty());

    return d->settings;
}

bool KisPaintOpPreset::load(KisResourcesInterfaceSP resourcesInterface)
{
    setValid(false);

    if (filename().isEmpty()) {
        return false;
    }

    QIODevice *dev = 0;
    QByteArray ba;

    if (filename().startsWith("bundle://")) {
        QString bn = filename().mid(9);
        int pos = bn.lastIndexOf(":");
        QString fn = bn.right(bn.size() - pos - 1);
        bn = bn.left(pos);

        QScopedPointer<KoStore> resourceStore(KoStore::createStore(bn, KoStore::Read, "application/x-krita-resourcebundle", KoStore::Zip));
        if (!resourceStore || resourceStore->bad()) {
            warnKrita << "Could not open store on bundle" << bn;
            return false;
        }

        if (resourceStore->isOpen()) resourceStore->close();

        if (!resourceStore->open(fn)) {
            warnKrita << "Could not open preset" << fn << "in bundle" << bn;
            return false;
        }

        ba = resourceStore->device()->readAll();
        dev = new QBuffer(&ba);

        resourceStore->close();
    }
    else {

        dev = new QFile(filename());
        if (dev->size() == 0)
        {
            delete dev;
            return false;
        }

        if (!dev->open(QIODevice::ReadOnly)) {
            warnKrita << "Can't open file " << filename();
            delete dev;
            return false;
        }
    }

    bool res = loadFromDevice(dev, resourcesInterface);
    delete dev;

    setValid(res);
    return res;

}

bool KisPaintOpPreset::loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface)
{
    QImageReader reader(dev, "PNG");

    QString version = reader.text("version");
    QString preset = reader.text("preset");

    dbgImage << version;

    if (version != "2.2") {
        return false;
    }

    QImage img;
    if (!reader.read(&img)) {
        dbgImage << "Fail to decode PNG";
        return false;
    }

    //Workaround for broken presets
    //Presets was saved with nested cdata section
    preset.replace("<curve><![CDATA[", "<curve>");
    preset.replace("]]></curve>", "</curve>");

    QDomDocument doc;
    if (!doc.setContent(preset)) {
        return false;
    }

    fromXML(doc.documentElement(), resourcesInterface);

    if (!d->settings) {
        return false;
    }
    setValid(true);
    setImage(img);

    return true;
}

bool KisPaintOpPreset::save()
{
    const QString paintopid = d->settings->getString("paintop", QString());
    if (paintopid.isEmpty())
        return false;

    return KoResource::save();
}

void KisPaintOpPreset::toXML(QDomDocument& doc, QDomElement& elt) const
{
    QString paintopid = d->settings->getString("paintop", QString());

    elt.setAttribute("paintopid", paintopid);
    elt.setAttribute("name", name());

    // sanitize the settings
    bool hasTexture = d->settings->getBool("Texture/Pattern/Enabled");
    if (!hasTexture) {
        Q_FOREACH (const QString & key, d->settings->getProperties().keys()) {
            if (key.startsWith("Texture") && key != "Texture/Pattern/Enabled") {
                d->settings->removeProperty(key);
            }
        }
    }

    d->settings->toXML(doc, elt);
}

void KisPaintOpPreset::fromXML(const QDomElement& presetElt, KisResourcesInterfaceSP resourcesInterface)
{
    setName(presetElt.attribute("name"));
    QString paintopid = presetElt.attribute("paintopid");

    if (!metadata().contains("paintopid")) {
        addMetaData("paintopid", paintopid);
    }

    if (paintopid.isEmpty()) {
        dbgImage << "No paintopid attribute";
        setValid(false);
        return;
    }

    if (KisPaintOpRegistry::instance()->get(paintopid) == 0) {
        dbgImage << "No paintop " << paintopid;
        setValid(false);
        return;
    }

    KoID id(paintopid, QString());

    KisPaintOpSettingsSP settings = KisPaintOpRegistry::instance()->createSettings(id, resourcesInterface);
    if (!settings) {
        setValid(false);
        warnKrita << "Could not load settings for preset" << paintopid;
        return;
    }

    settings->fromXML(presetElt);
    // sanitize the settings
    bool hasTexture = settings->getBool("Texture/Pattern/Enabled");
    if (!hasTexture) {
        Q_FOREACH (const QString & key, settings->getProperties().keys()) {
            if (key.startsWith("Texture") && key != "Texture/Pattern/Enabled") {
                settings->removeProperty(key);
            }
        }
    }
    setSettings(settings);

}

bool KisPaintOpPreset::saveToDevice(QIODevice *dev) const
{
    QImageWriter writer(dev, "PNG");

    QDomDocument doc;
    QDomElement root = doc.createElement("Preset");
    toXML(doc, root);
    doc.appendChild(root);

    writer.setText("version", "2.2");
    writer.setText("preset", doc.toString());

    QImage img;

    if (image().isNull()) {
        img = QImage(1, 1, QImage::Format_RGB32);
    } else {
        img = image();
    }

    KoResource::saveToDevice(dev);

    return writer.write(img);

}

QPointer<KisPaintopSettingsUpdateProxy> KisPaintOpPreset::updateProxy() const
{
    if (!d->updateProxy) {
        d->updateProxy = new KisPaintopSettingsUpdateProxy(d->proxyParent);
    }
    return d->updateProxy;
}

QPointer<KisPaintopSettingsUpdateProxy> KisPaintOpPreset::updateProxyNoCreate() const
{
    return d->updateProxy;
}

QList<KisUniformPaintOpPropertySP> KisPaintOpPreset::uniformProperties()
{
    return d->settings->uniformProperties(d->settings);
}

bool KisPaintOpPreset::hasMaskingPreset() const
{
    return d->settings && d->settings->hasMaskingSettings();
}

KisPaintOpPresetSP KisPaintOpPreset::createMaskingPreset() const
{
    KisPaintOpPresetSP result;

    if (d->settings && d->settings->hasMaskingSettings()) {
        result.reset(new KisPaintOpPreset());
        result->setSettings(d->settings->createMaskingSettings());
        if (!result->valid()) {
            result.clear();
        }
    }

    return result;
}

KisResourcesInterfaceSP KisPaintOpPreset::resourcesInterface() const
{
    return d->settings ? d->settings->resourcesInterface() : nullptr;
}

void KisPaintOpPreset::setResourcesInterface(KisResourcesInterfaceSP resourcesInterface)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(d->settings);
    d->settings->setResourcesInterface(resourcesInterface);
}

KoCanvasResourcesInterfaceSP KisPaintOpPreset::canvasResourcesInterface() const
{
    return d->settings ? d->settings->canvasResourcesInterface() : nullptr;
}

void KisPaintOpPreset::setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(d->settings);
    d->settings->setCanvasResourcesInterface(canvasResourcesInterface);
}

namespace KisRequiredResourcesOperators
{
template <>
struct ResourceTraits<KisPaintOpPreset>
{
    template <typename T>
    using SharedPointerType = QSharedPointer<T>;

    template <typename D, typename S>
    static inline SharedPointerType<D> dynamicCastSP(SharedPointerType<S> src) {
        return src.template dynamicCast<D>();
    }
};
}

void KisPaintOpPreset::createLocalResourcesSnapshot(KisResourcesInterfaceSP globalResourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    KisRequiredResourcesOperators::createLocalResourcesSnapshot(this, globalResourcesInterface);

    const QList<int> canvasResources = this->requiredCanvasResources();
    if (!canvasResources.isEmpty()) {
        KoLocalStrokeCanvasResourcesSP storage(new KoLocalStrokeCanvasResources());
        Q_FOREACH (int key, canvasResources) {
            storage->storeResource(key, canvasResourcesInterface->resource(key));
        }
        setCanvasResourcesInterface(storage);
    }
}

bool KisPaintOpPreset::hasLocalResourcesSnapshot() const
{
    return KisRequiredResourcesOperators::hasLocalResourcesSnapshot(this);
}

KisPaintOpPresetSP KisPaintOpPreset::cloneWithResourcesSnapshot(KisResourcesInterfaceSP globalResourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface) const
{
    KisPaintOpPresetSP result =
        KisRequiredResourcesOperators::cloneWithResourcesSnapshot(this, globalResourcesInterface);

    const QList<int> canvasResources = result->requiredCanvasResources();
    if (!canvasResources.isEmpty()) {
        KoLocalStrokeCanvasResourcesSP storage(new KoLocalStrokeCanvasResources());
        Q_FOREACH (int key, canvasResources) {
            storage->storeResource(key, canvasResourcesInterface->resource(key));
        }
        result->setCanvasResourcesInterface(storage);
    }

    return result;
}

QList<KoResourceSP> KisPaintOpPreset::linkedResources(KisResourcesInterfaceSP globalResourcesInterface) const
{
    QList<KoResourceSP> resources;

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(d->settings, resources);

    KisPaintOpFactory* f = KisPaintOpRegistry::instance()->value(paintOp().id());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(f, resources);
    resources << f->prepareLinkedResources(d->settings, globalResourcesInterface);

    if (hasMaskingPreset()) {
        KisPaintOpPresetSP maskingPreset = createMaskingPreset();

        KisPaintOpFactory* f = KisPaintOpRegistry::instance()->value(maskingPreset->paintOp().id());
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(f, resources);
        resources << f->prepareLinkedResources(maskingPreset->settings(), globalResourcesInterface);
    }

    return resources;
}

QList<KoResourceSP> KisPaintOpPreset::embeddedResources(KisResourcesInterfaceSP globalResourcesInterface) const
{
    QList<KoResourceSP> resources;

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(d->settings, resources);

    KisPaintOpFactory* f = KisPaintOpRegistry::instance()->value(paintOp().id());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(f, resources);
    resources << f->prepareEmbeddedResources(d->settings, globalResourcesInterface);

    if (hasMaskingPreset()) {
        KisPaintOpPresetSP maskingPreset = createMaskingPreset();

        KisPaintOpFactory* f = KisPaintOpRegistry::instance()->value(maskingPreset->paintOp().id());
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(f, resources);
        resources << f->prepareEmbeddedResources(maskingPreset->settings(), globalResourcesInterface);
    }

    return resources;
}

QList<int> KisPaintOpPreset::requiredCanvasResources() const
{
    return d->settings ? d->settings->requiredCanvasResources() : QList<int>();
}

KisPaintOpPreset::UpdatedPostponer::UpdatedPostponer(KisPaintOpPresetSP preset)
    : m_updateProxy(preset->updateProxyNoCreate())
{
    if (m_updateProxy) {
        m_updateProxy->postponeSettingsChanges();
    }
}

KisPaintOpPreset::UpdatedPostponer::~UpdatedPostponer()
{
    if (m_updateProxy) {
        m_updateProxy->unpostponeSettingsChanges();
    }
}
