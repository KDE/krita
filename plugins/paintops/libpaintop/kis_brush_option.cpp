/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2008 Sven Langkamp <sven.langkamp@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_brush_option.h"

#include <QDomDocument>
#include <QDomElement>

#include "kis_properties_configuration.h"
#include <KisPaintopSettingsIds.h>
#include <kis_brush.h>
#include <KoEphemeralResource.h>

#include <KoCanvasResourcesInterface.h>
#include <KoCanvasResourcesIds.h>


void KisBrushOptionProperties::writeOptionSettingImpl(KisPropertiesConfiguration *setting) const
{
    if (!m_brush) return;

    QDomDocument d;
    QDomElement e = d.createElement("Brush");
    m_brush->toXML(d, e);
    d.appendChild(e);
    setting->setProperty("brush_definition", d.toString());

    QString brushFileName = !m_brush->filename().isEmpty() ? m_brush->filename() : QString();
    setting->setProperty(KisPaintOpUtils::RequiredBrushFileTag, brushFileName);

    {
        QStringList requiredFiles =
            setting->getStringList(KisPaintOpUtils::RequiredBrushFilesListTag);
        requiredFiles << brushFileName;
        setting->setProperty(KisPaintOpUtils::RequiredBrushFilesListTag, requiredFiles);
    }

    QString md5sum = m_brush->md5Sum();
    setting->setProperty(KisPaintOpUtils::RequiredBrushMD5Tag, md5sum);

    {
        QStringList requiredMD5s =
            setting->getStringList(KisPaintOpUtils::RequiredBrushMD5ListTag);
        requiredMD5s << md5sum;
        setting->setProperty(KisPaintOpUtils::RequiredBrushMD5ListTag, requiredMD5s);
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

void KisBrushOptionProperties::readOptionSettingResourceImpl(const KisPropertiesConfiguration *setting, KisResourcesInterfaceSP resourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    QDomElement element = getBrushXMLElement(setting);

    if (!element.isNull()) {
        m_brush = KisBrush::fromXML(element, resourcesInterface);

        if (m_brush && m_brush->applyingGradient() && canvasResourcesInterface) {
            KoAbstractGradientSP gradient = canvasResourcesInterface->resource(KoCanvasResource::CurrentGradient).value<KoAbstractGradientSP>()->cloneAndBakeVariableColors(canvasResourcesInterface);
            m_brush->setGradient(gradient);
        }
    }
}

QList<KoResourceSP> KisBrushOptionProperties::prepareLinkedResourcesImpl(const KisPropertiesConfiguration *settings, KisResourcesInterfaceSP resourcesInterface) const
{
    QList<KoResourceSP> resources;

    QDomElement element = getBrushXMLElement(settings);
    if (element.isNull()) return resources;

    KisBrushSP brush = KisBrush::fromXML(element, resourcesInterface);
    // TODO: implement proper property for KoResource about ephemerality
    if (brush && !dynamic_cast<KoEphemeralResource<KisBrush>*>(brush.data())) {
        resources << brush;
    }

    return resources;
}

QList<KoResourceSP> KisBrushOptionProperties::prepareEmbeddedResourcesImpl(const KisPropertiesConfiguration *settings, KisResourcesInterfaceSP resourcesInterface) const
{
    Q_UNUSED(settings);
    Q_UNUSED(resourcesInterface);
    return {};
}

enumBrushApplication KisBrushOptionProperties::brushApplication(const KisPropertiesConfiguration *settings, KisResourcesInterfaceSP resourcesInterface)
{
    QList<KoResourceSP> resources;

    QDomElement element = getBrushXMLElement(settings);
    if (element.isNull()) return ALPHAMASK;

    KisBrushSP brush = KisBrush::fromXML(element, resourcesInterface);

    return brush->brushApplication();
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
