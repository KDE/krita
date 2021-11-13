/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2009 Sven Langkamp <sven.langkamp@gmail.com>
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

#include <KisDirtyStateSaver.h>

#include <brushengine/kis_paintop_settings.h>
#include "kis_paintop_registry.h"
#include "kis_painter.h"
#include <brushengine/kis_paint_information.h>
#include "kis_paint_device.h"
#include "kis_image.h"
#include "KisPaintOpPresetUpdateProxy.h"
#include <brushengine/kis_paintop_config_widget.h>
#include <KisRequiredResourcesOperators.h>
#include <KoLocalStrokeCanvasResources.h>
#include <KisResourceModel.h>
#include "KisPaintopSettingsIds.h"
#include <KisResourceTypes.h>
#include <KisResourceModelProvider.h>
#include <krita_container_utils.h>

#include <KoStore.h>

struct Q_DECL_HIDDEN KisPaintOpPreset::Private {

    struct UpdateListener : public KisPaintOpSettings::UpdateListener {
        UpdateListener(KisPaintOpPreset *parentPreset)
            : m_parentPreset(parentPreset)
        {
        }

        void setDirty(bool value) override {
            m_parentPreset->setDirty(value);
        }

        bool isDirty() const override {
            return m_parentPreset->isDirty();
        }

        void notifySettingsChanged() override {
            KisPaintOpPresetUpdateProxy* proxy = m_parentPreset->updateProxyNoCreate();
            if (proxy) {
                proxy->notifySettingsChanged();
            }
        }

    private:
        KisPaintOpPreset *m_parentPreset;
    };

public:
    Private(KisPaintOpPreset *q)
        : settingsUpdateListener(new UpdateListener(q)),
          version("5.0")
    {
    }

    KisPaintOpSettingsSP settings {0};
    QScopedPointer<KisPaintOpPresetUpdateProxy> updateProxy;
    KisPaintOpSettings::UpdateListenerSP settingsUpdateListener;
    QString version;
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
    setName(name().replace("_", " "));
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

QString KisPaintOpPreset::name() const
{
    return KoResource::name().replace("_", " ");
}

void KisPaintOpPreset::setSettings(KisPaintOpSettingsSP settings)
{
    Q_ASSERT(settings);
    Q_ASSERT(!settings->getString("paintop", QString()).isEmpty());

    KisDirtyStateSaver<KisPaintOpPreset*> dirtyStateSaver(this);

    if (d->settings) {
        d->settings->setUpdateListener(KisPaintOpSettings::UpdateListenerWSP());
        d->settings = 0;
    }

    if (settings) {
        d->settings = settings->clone();
        d->settings->setUpdateListener(d->settingsUpdateListener);
    }

    if (d->updateProxy) {
        d->updateProxy->notifyUniformPropertiesChanged();
        d->updateProxy->notifySettingsChanged();
    }
    setValid(true);
}

KisPaintOpSettingsSP KisPaintOpPreset::settings() const
{
    Q_ASSERT(d->settings);
    Q_ASSERT(!d->settings->getString("paintop", QString()).isEmpty());

    return d->settings;
}

bool KisPaintOpPreset::loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface)
{
    QImageReader reader(dev, "PNG");

    d->version = reader.text("version");
    QString preset = reader.text("preset");
    int resourceCount = reader.text("embedded_resources").toInt();

    if (!(d->version == "2.2" || "5.0")) {
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

    if (d->version == "5.0" && resourceCount > 0) {
        // Load the embedded resources
        QDomNode n = doc.firstChild();
        while (!n.isNull()) {
            QDomElement e = n.toElement();
            if (!e.isNull()) {
                if (e.tagName() == "resources") {
                    QDomNode n2 = n.firstChild();
                    while (!n2.isNull()) {
                        n2 = n2.nextSibling();
                        QDomElement e2 = n2.toElement();
                        QString resourceType = e2.attribute("type");
                        QString md5sum = e2.attribute("md5sum");
                        QString name = e2.attribute("name");
                        QString filename = e2.attribute("filename");

                        KoResourceSP existingResource = resourcesInterface->source(resourceType).bestMatch(md5sum, filename, name);

                        if (!existingResource) {
                            QByteArray ba = QByteArray::fromBase64(e2.text().toLatin1());

                            QBuffer buf(&ba);
                            buf.open(QBuffer::ReadOnly);

                            /// HACK ALERT: Calling importResource()
                            /// here is technically undefined
                            /// behavior, because this code is
                            /// called from inside the storage's
                            /// loadVersionedResource(). Basically
                            /// we change underlying storage's
                            /// storage while it is reading from
                            /// it.

                            KisResourceModel model(resourceType);
                            model.importResource(filename, &buf, false, "memory");
                        }
                    }
                    break;
                }
            }
            n = n.nextSibling();
        }
    }

    fromXML(doc.documentElement(), resourcesInterface);

    if (!d->settings) {
        return false;
    }

    setValid(d->settings->isValid());

    setImage(img);

    return true;
}

void KisPaintOpPreset::toXML(QDomDocument& doc, QDomElement& elt) const
{
    QString paintopid = d->settings->getString("paintop", QString());

    elt.setAttribute("paintopid", paintopid);
    elt.setAttribute("name", name());


    QList<KoResourceLoadResult> linkedResources = this->linkedResources(resourcesInterface());

    elt.setAttribute("embedded_resources", linkedResources.count());

    if (!linkedResources.isEmpty()) {
        QDomElement resourcesElement = doc.createElement("resources");
        elt.appendChild(resourcesElement);
        Q_FOREACH(KoResourceLoadResult linkedResource, linkedResources) {
            // we have requested linked resources, how can it be an embedded one?
            KIS_SAFE_ASSERT_RECOVER(linkedResource.type() != KoResourceLoadResult::EmbeddedResource) { continue; }

            KoResourceSP resource = linkedResource.resource();

            if (!resource) {
                qWarning() << "WARNING: KisPaintOpPreset::toXML couldn't fetch a linked resource" << linkedResource.signature();
                continue;
            }

            KIS_SAFE_ASSERT_RECOVER_NOOP(resource->isSerializable() && "embedding non-serializable resources is not yet implemented");

            QByteArray ba;
            QBuffer buf(&ba);
            buf.open(QBuffer::WriteOnly);
            KisResourceModel model(resource->resourceType().first);
            bool r = model.exportResource(resource, &buf);
            buf.close();
            if (r) {
                QDomText text = doc.createCDATASection(QString::fromLatin1(ba.toBase64()));
                QDomElement e = doc.createElement("resource");
                e.setAttribute("type", resource->resourceType().first);
                e.setAttribute("md5sum", resource->md5Sum());
                e.setAttribute("name", resource->name());
                e.setAttribute("filename", resource->filename());
                e.appendChild(text);
                resourcesElement.appendChild(e);

            }
        }
    }

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

    /**
     * HACK ALERT: We update the version of the resource format on
     * the first save operation, even though there is no guarantee
     * that it was "save" operation, but not "export" operation.
     *
     * The only point it affects now is whether we need to check
     * for the presence of the linkedResources() in
     * updateLinkedResourcesMetaData(). The new version of the
     * preset format ("5.0") has all the linked resources embedded
     * outside KisPaintOpSettings, which are automatically
     * loaded on the the resource activation. We we shouldn't
     * add them into metaData()["dependent_resources_filenames"].
     */
    d->version = "5.0";

    writer.setText("version", d->version);
    writer.setText("preset", doc.toString());

    QImage img;

    if (image().isNull()) {
        img = QImage(1, 1, QImage::Format_RGB32);
    } else {
        img = image();
    }

    return writer.write(img);

}

void KisPaintOpPreset::updateLinkedResourcesMetaData(KisResourcesInterfaceSP resourcesInterface)
{
    /**
     * The new preset format embeds all the linked resources outside
     * KisPaintOpSettings and loads them on activation, therefore we
     * shouldn't add them into "dependent_resources_filenames".
     */

    if (d->version == "2.2") {
        QList<KoResourceLoadResult> dependentResources = this->linkedResources(resourcesInterface);

        QStringList resourceFileNames;

        Q_FOREACH (KoResourceLoadResult resource, dependentResources) {
            const QString filename = resource.signature().filename;

            if (!filename.isEmpty()) {
                resourceFileNames.append(filename);
            }
        }

        KritaUtils::makeContainerUnique(resourceFileNames);

        if (!resourceFileNames.isEmpty()) {
            addMetaData("dependent_resources_filenames", resourceFileNames);
        }
    }
}

QPointer<KisPaintOpPresetUpdateProxy> KisPaintOpPreset::updateProxy() const
{
    if (!d->updateProxy) {
        d->updateProxy.reset(new KisPaintOpPresetUpdateProxy());
    }
    return d->updateProxy.data();
}

QPointer<KisPaintOpPresetUpdateProxy> KisPaintOpPreset::updateProxyNoCreate() const
{
    return d->updateProxy.data();
}

QList<KisUniformPaintOpPropertySP> KisPaintOpPreset::uniformProperties()
{
    /// we pass a shared pointer to settings explicitly,
    /// because the settings will not be able to wrap
    /// itself into a shared pointer
    return d->settings->uniformProperties(d->settings, updateProxy());
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
            KisRequiredResourcesOperators::cloneWithResourcesSnapshot<KisPaintOpPresetSP>(this, globalResourcesInterface);

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

QList<KoResourceLoadResult> KisPaintOpPreset::linkedResources(KisResourcesInterfaceSP globalResourcesInterface) const
{
    QList<KoResourceLoadResult> resources;

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(d->settings, resources);

    KisPaintOpFactory* f = KisPaintOpRegistry::instance()->value(paintOp().id());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(f, resources);
    resources << f->prepareLinkedResources(d->settings, globalResourcesInterface);

    if (hasMaskingPreset()) {
        KisPaintOpPresetSP maskingPreset = createMaskingPreset();
        Q_ASSERT(maskingPreset);

        KisPaintOpFactory* f = KisPaintOpRegistry::instance()->value(maskingPreset->paintOp().id());
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(f, resources);
        resources << f->prepareLinkedResources(maskingPreset->settings(), globalResourcesInterface);

    }

    return resources;
}

QList<KoResourceLoadResult> KisPaintOpPreset::embeddedResources(KisResourcesInterfaceSP globalResourcesInterface) const
{
    QList<KoResourceLoadResult> resources;

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(d->settings, resources);

    KisPaintOpFactory* f = KisPaintOpRegistry::instance()->value(paintOp().id());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(f, resources);
    resources << f->prepareEmbeddedResources(d->settings, globalResourcesInterface);

    if (hasMaskingPreset()) {
        KisPaintOpPresetSP maskingPreset = createMaskingPreset();
        Q_ASSERT(maskingPreset);
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

void KisPaintOpPreset::setResourceCacheInterface(KoResourceCacheInterfaceSP cacheInterface)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(d->settings);
    d->settings->setResourceCacheInterface(cacheInterface);
}

KoResourceCacheInterfaceSP KisPaintOpPreset::resourceCacheInterface() const
{
    return d->settings ? d->settings->resourceCacheInterface() : KoResourceCacheInterfaceSP();
}

void KisPaintOpPreset::regenerateResourceCache(KoResourceCacheInterfaceSP cacheInterface)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(d->settings);

    d->settings->regenerateResourceCache(cacheInterface);
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
