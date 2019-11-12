/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 * Copyright (C) Sven Langkamp <sven.langkamp@gmail.com>, (C) 2008
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

#include "kis_brush_option.h"

#include <QDomDocument>
#include <QDomElement>

#include "kis_properties_configuration.h"
#include <KisPaintopSettingsIds.h>

void KisBrushOptionProperties::writeOptionSettingImpl(KisPropertiesConfiguration *setting) const
{
    if (!m_brush)
        return;

    QDomDocument d;
    QDomElement e = d.createElement("Brush");
    m_brush->toXML(d, e);
    d.appendChild(e);
    setting->setProperty("brush_definition", d.toString());

    QString brushFileName  = !m_brush->filename().isEmpty() ?
                            m_brush->filename() : QString();

    setting->setProperty(KisPaintOpUtils::RequiredBrushFileTag, brushFileName);

    {
        QStringList requiredFiles =
            setting->getStringList(KisPaintOpUtils::RequiredBrushFilesListTag);



        requiredFiles << brushFileName;
        setting->setProperty(KisPaintOpUtils::RequiredBrushFilesListTag, requiredFiles);
    }

}

QDomElement getBrushXMLElement(const KisPropertiesConfiguration *setting)
{
    QDomElement element;

    QString brushDefinition = setting->getString("brush_definition");
    if (!brushDefinition.isEmpty()) {
        QDomDocument d;
        d.setContent(brushDefinition, false);
        element = d.firstChildElement("Brush");
    }

    return element;
}

void KisBrushOptionProperties::readOptionSettingImpl(const KisPropertiesConfiguration *setting)
{
    QDomElement element = getBrushXMLElement(setting);

    if (!element.isNull()) {
        m_brush = KisBrush::fromXML(element);
    }
}

#ifdef HAVE_THREADED_TEXT_RENDERING_WORKAROUND

#include "kis_text_brush_factory.h"

bool KisBrushOptionProperties::isTextBrush(const KisPropertiesConfiguration *setting)
{
    static QString textBrushId = KisTextBrushFactory().id();

    QDomElement element = getBrushXMLElement(setting);
    QString brushType = element.attribute("type");

    return brushType == textBrushId;
}

#endif /* HAVE_THREADED_TEXT_RENDERING_WORKAROUND */

KisBrushSP KisBrushOptionProperties::brush() const
{
    return m_brush;
}

void KisBrushOptionProperties::setBrush(KisBrushSP brush)
{
    m_brush = brush;
}
