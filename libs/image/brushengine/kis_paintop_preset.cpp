/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 * Copyright (C) Sven Langkamp <sven.langkamp@gmail.com>, (C) 2009
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include <brushengine/kis_paintop_preset.h>

#include <QFile>
#include <QSize>
#include <QImage>
#include <QImageWriter>
#include <QImageReader>
#include <QDomDocument>
#include <QBuffer>

#include <brushengine/kis_paintop_settings.h>
#include "kis_paintop_registry.h"
#include "kis_painter.h"
#include <brushengine/kis_paint_information.h>
#include "kis_paint_device.h"
#include "kis_image.h"
#include "kis_paintop_settings_update_proxy.h"
#include <brushengine/kis_paintop_config_widget.h>

#include <KoStore.h>

struct Q_DECL_HIDDEN KisPaintOpPreset::Private {
    Private()
        : settings(0),
          dirtyPreset(false)
    {
    }

    KisPaintOpSettingsSP settings;
    bool dirtyPreset;
    QScopedPointer<KisPaintopSettingsUpdateProxy> updateProxy;
};


KisPaintOpPreset::KisPaintOpPreset()
    : KoResource(QString())
    , m_d(new Private)
{
}

KisPaintOpPreset::KisPaintOpPreset(const QString & fileName)
    : KoResource(fileName)
    , m_d(new Private)
{
}

KisPaintOpPreset::~KisPaintOpPreset()
{
    delete m_d;
}

KisPaintOpPresetSP KisPaintOpPreset::clone() const
{
    KisPaintOpPresetSP preset(new KisPaintOpPreset());

    if (settings()) {
        preset->setSettings(settings()); // the settings are cloned inside!
    }
    preset->setDirty(isDirty());
    // only valid if we could clone the settings
    preset->setValid(settings());

    preset->setPaintOp(paintOp());
    preset->setName(name());
    preset->setImage(image());
    preset->settings()->setPreset(KisPaintOpPresetWSP(preset));

    Q_ASSERT(preset->valid());

    return preset;
}
void KisPaintOpPreset::setDirty(bool value)
{
    m_d->dirtyPreset = value;
}

bool KisPaintOpPreset::isDirty() const
{
    return m_d->dirtyPreset;
}

void KisPaintOpPreset::setPaintOp(const KoID & paintOp)
{
    Q_ASSERT(m_d->settings);
    m_d->settings->setProperty("paintop", paintOp.id());
}

KoID KisPaintOpPreset::paintOp() const
{
    Q_ASSERT(m_d->settings);
    return KoID(m_d->settings->getString("paintop"));
}

void KisPaintOpPreset::setOptionsWidget(KisPaintOpConfigWidget* widget)
{
    if (m_d->settings) {
        m_d->settings->setOptionsWidget(widget);

        if (widget) {
            widget->setConfigurationSafe(m_d->settings);
        }
    }
}

void KisPaintOpPreset::setSettings(KisPaintOpSettingsSP settings)
{
    Q_ASSERT(settings);
    Q_ASSERT(!settings->getString("paintop", QString()).isEmpty());

    DirtyStateSaver dirtyStateSaver(this);

    KisPaintOpConfigWidget *oldOptionsWidget = 0;

    if (m_d->settings) {
        oldOptionsWidget = m_d->settings->optionsWidget();
        m_d->settings->setOptionsWidget(0);
        m_d->settings->setPreset(0);
        m_d->settings = 0;
    }

    if (settings) {
        m_d->settings = settings->clone();
        m_d->settings->setPreset(KisPaintOpPresetWSP(this));

        if (oldOptionsWidget) {
            m_d->settings->setOptionsWidget(oldOptionsWidget);
            oldOptionsWidget->setConfigurationSafe(m_d->settings);
        }
    }

    setValid(m_d->settings);

    if (m_d->updateProxy) {
        m_d->updateProxy->notifyUniformPropertiesChanged();
        m_d->updateProxy->notifySettingsChanged();
    }
}

KisPaintOpSettingsSP KisPaintOpPreset::settings() const
{
    Q_ASSERT(m_d->settings);
    Q_ASSERT(!m_d->settings->getString("paintop", QString()).isEmpty());

    return m_d->settings;
}

bool KisPaintOpPreset::load()
{
    dbgImage << "Load preset " << filename();
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

    bool res = loadFromDevice(dev);
    delete dev;

    setValid(res);
    setDirty(false);
    return res;

}

bool KisPaintOpPreset::loadFromDevice(QIODevice *dev)
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

    fromXML(doc.documentElement());

    if (!m_d->settings) {
        return false;
    }

    setValid(true);
    setImage(img);

    return true;
}

bool KisPaintOpPreset::save()
{

    if (filename().isEmpty())
        return false;

    QString paintopid = m_d->settings->getString("paintop", QString());

    if (paintopid.isEmpty())
        return false;

    QFile f(filename());
    f.open(QFile::WriteOnly);

    return saveToDevice(&f);
}

void KisPaintOpPreset::toXML(QDomDocument& doc, QDomElement& elt) const
{
    QString paintopid = m_d->settings->getString("paintop", QString());

    elt.setAttribute("paintopid", paintopid);
    elt.setAttribute("name", name());

    // sanitize the settings
    bool hasTexture = m_d->settings->getBool("Texture/Pattern/Enabled");
    if (!hasTexture) {
        Q_FOREACH (const QString & key, m_d->settings->getProperties().keys()) {
            if (key.startsWith("Texture") && key != "Texture/Pattern/Enabled") {
                m_d->settings->removeProperty(key);
            }
        }
    }

    m_d->settings->toXML(doc, elt);
}

void KisPaintOpPreset::fromXML(const QDomElement& presetElt)
{
    setName(presetElt.attribute("name"));
    QString paintopid = presetElt.attribute("paintopid");

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

    KisPaintOpSettingsSP settings = KisPaintOpRegistry::instance()->settings(id);
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

    m_d->dirtyPreset = false;

    KoResource::saveToDevice(dev);

    return writer.write(img);

}

KisPaintopSettingsUpdateProxy* KisPaintOpPreset::updateProxy() const
{
    if (!m_d->updateProxy) {
        m_d->updateProxy.reset(new KisPaintopSettingsUpdateProxy());
    }
    return m_d->updateProxy.data();
}

KisPaintopSettingsUpdateProxy* KisPaintOpPreset::updateProxyNoCreate() const
{
    return m_d->updateProxy.data();
}

QList<KisUniformPaintOpPropertySP> KisPaintOpPreset::uniformProperties()
{
    return m_d->settings->uniformProperties(m_d->settings);
}

bool KisPaintOpPreset::hasMaskingPreset() const
{
    return m_d->settings && m_d->settings->hasMaskingSettings();
}

KisPaintOpPresetSP KisPaintOpPreset::createMaskingPreset() const
{
    KisPaintOpPresetSP result;

    if (m_d->settings && m_d->settings->hasMaskingSettings()) {
        result = new KisPaintOpPreset();
        result->setSettings(m_d->settings->createMaskingSettings());
        if (!result->valid()) {
            result.clear();
        }
    }

    return result;
}

KisPaintOpPreset::UpdatedPostponer::UpdatedPostponer(KisPaintOpPreset *preset)
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
