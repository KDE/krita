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
#include <QDomDocument>

#include <KoColorSpaceRegistry.h>
#include <KoInputDevice.h>

#include "kis_types.h"
#include "kis_paintop_settings.h"
#include "kis_paintop_registry.h"
#include "kis_painter.h"
#include "kis_paint_information.h"
#include "kis_paint_device.h"
#include "kis_image.h"

struct KisPaintOpPreset::Private {
    KisPaintOpSettingsSP settings;
    QImage image;
};


KisPaintOpPreset::KisPaintOpPreset()
        : KoResource(QString())
        , m_d(new Private)
{
    m_d->settings = 0;
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
    preset->setPaintOp(paintOp());
    preset->setName(name());
    preset->setValid(valid());

    return preset;
}

void KisPaintOpPreset::setPaintOp(const KoID & paintOp)
{
    m_d->settings->setProperty("paintop", paintOp.id());
}

KoID KisPaintOpPreset::paintOp() const
{
    return KoID(m_d->settings->getString("paintop"), name());
}

void KisPaintOpPreset::setSettings(KisPaintOpSettingsSP settings)
{
    Q_ASSERT(!settings->getString("paintop", "").isEmpty());
    m_d->settings = settings->clone();
    setValid(true);
}

KisPaintOpSettingsSP KisPaintOpPreset::settings() const
{
    Q_ASSERT(!m_d->settings->getString("paintop", "").isEmpty());

    return m_d->settings;
}

bool KisPaintOpPreset::load()
{
    if (filename().isEmpty())
        return false;

    QFile f(filename());

    QDomDocument doc;
    if (!f.open(QIODevice::ReadOnly))
        return false;
    if (!doc.setContent(&f)) {
        f.close();
        return false;
    }
    f.close();

    QDomElement element = doc.documentElement();
    QString paintopid = element.attribute("paintopid");

    if (paintopid.isEmpty())
        return false;

    KoID id(paintopid, "");

    KisPaintOpSettingsSP settings = KisPaintOpRegistry::instance()->settings(id, 0);
    if (!settings)
        return false;
    settings->fromXML(element);
    setSettings(settings);

    updateImage();
    return true;
}

bool KisPaintOpPreset::save()
{
    if (filename().isEmpty())
        return false;

    QFile f(filename());
    f.open(QIODevice::WriteOnly);

    QDomDocument doc;
    QDomElement root = doc.createElement("preset");

    QString paintopid = m_d->settings->getString("paintop", "");
    if (paintopid.isEmpty())
        return false;

    root.setAttribute("paintopid", paintopid);
    doc.appendChild(root);

    m_d->settings->toXML(doc, root);

    QTextStream textStream(&f);
    doc.save(textStream, 4);
    f.close();
    return true;
}

QImage KisPaintOpPreset::image() const
{
    return m_d->image;
}

void KisPaintOpPreset::updateImage()
{
    m_d->image = m_d->settings->sampleStroke(QSize(100, 20));
}

QImage KisPaintOpPreset::generatePreviewImage(int width, int height)
{
    return m_d->settings->sampleStroke(QSize(width, height));
}

