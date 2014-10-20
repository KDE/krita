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
#include "kis_paintop_preset.h"

#include <QFile>
#include <QSize>
#include <QImage>
#include <QImageWriter>
#include <QImageReader>
#include <QDomDocument>
#include <QBuffer>
#include <QCryptographicHash>

#include <KoInputDevice.h>

#include "kis_types.h"
#include "kis_paintop_settings.h"
#include "kis_paintop_registry.h"
#include "kis_painter.h"
#include "kis_paint_information.h"
#include "kis_paint_device.h"
#include "kis_image.h"



struct KisPaintOpPreset::Private {
    Private()
        : settings(0) {
    }

    KisPaintOpSettingsSP settings;
    bool dirtyPreset;

};


KisPaintOpPreset::KisPaintOpPreset()
    : KoResource(QString())
    , m_d(new Private)
{
    m_d->dirtyPreset = false;
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

KisPaintOpPreset* KisPaintOpPreset::clone() const
{
    KisPaintOpPreset * preset = new KisPaintOpPreset();

    if (settings()) {
        preset->setSettings(settings()->clone());
    }
    preset->setPresetDirty(isPresetDirty());
    // only valid if we could clone the settings
    preset->setValid(settings());

    preset->setPaintOp(paintOp());
    preset->setName(name());
    preset->settings()->setPreset(KisPaintOpPresetWSP(preset));


    return preset;
}
void KisPaintOpPreset::setPresetDirty(bool value)
{
    m_d->dirtyPreset = value;    
}
bool KisPaintOpPreset::isPresetDirty() const
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
    return KoID(m_d->settings->getString("paintop"), name());
}

void KisPaintOpPreset::setSettings(KisPaintOpSettingsSP settings)
{
    Q_ASSERT(settings);
    Q_ASSERT(!settings->getString("paintop", "").isEmpty());

    bool saveDirtyPreset = isPresetDirty();
    if (settings) {
        m_d->settings = settings->clone();
        m_d->settings->setPreset(KisPaintOpPresetWSP(this));
    } else {
        m_d->settings = 0;
        m_d->settings->setPreset(0);
    }
    setPresetDirty(saveDirtyPreset);

    setValid(m_d->settings);
}

KisPaintOpSettingsSP KisPaintOpPreset::settings() const
{
    Q_ASSERT(m_d->settings);
    Q_ASSERT(!m_d->settings->getString("paintop", "").isEmpty());

    return m_d->settings;
}

bool KisPaintOpPreset::load()
{
    dbgImage << "Load preset " << filename();
    setValid(false);

    if (filename().isEmpty()) {
        return false;
    }

    QFile file(filename());

    if (file.size() == 0) return false;
    if (!file.open(QIODevice::ReadOnly)) {
        warnKrita << "Can't open file " << filename();
        return false;
    }

    bool res = loadFromDevice(&file);


    this->setPresetDirty(false);
    return res;

}

bool KisPaintOpPreset::loadFromDevice(QIODevice *dev)
{

    QImageReader reader(dev, "PNG");

    QString version = reader.text("version");
    QString preset = reader.text("preset");

    dbgImage << version << preset;

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

    QString paintopid = m_d->settings->getString("paintop", "");

    if (paintopid.isEmpty())
        return false;

    QFile f(filename());
    f.open(QFile::WriteOnly);

    return saveToDevice(&f);
}

void KisPaintOpPreset::toXML(QDomDocument& doc, QDomElement& elt) const
{
    QString paintopid = m_d->settings->getString("paintop", "");

    elt.setAttribute("paintopid", paintopid);
    elt.setAttribute("name", name());

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

    KoID id(paintopid, "");

    KisPaintOpSettingsSP settings = KisPaintOpRegistry::instance()->settings(id);
    if (!settings) {
        setValid(false);
        qWarning() << "Could not load settings for preset" << paintopid;
        return;
    }

    settings->fromXML(presetElt);

    setSettings(settings);

}

QByteArray KisPaintOpPreset::generateMD5() const
{
    QByteArray ba;
    QBuffer buf(&ba);
    buf.open(QBuffer::WriteOnly);
    saveToDevice(&buf);
    buf.close();

    if (!ba.isEmpty()) {
        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData(ba);
        return md5.result();
    }

    return ba;
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

    return writer.write(img);

}

