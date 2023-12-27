/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_brush_registry.h"

#include <QString>

#include <QGlobalStatic>
#include <klocalizedstring.h>

#include <KoPluginLoader.h>

#include <kis_debug.h>

#include "KoResourceServer.h"
#include "kis_auto_brush_factory.h"
#include "kis_text_brush_factory.h"
#include "kis_predefined_brush_factory.h"

Q_GLOBAL_STATIC(KisBrushRegistry, s_instance)


KisBrushRegistry::KisBrushRegistry()
{
}

KisBrushRegistry::~KisBrushRegistry()
{
    Q_FOREACH (const QString & id, keys()) {
        delete get(id);
    }
    dbgRegistry << "deleting KisBrushRegistry";
}

KisBrushRegistry* KisBrushRegistry::instance()
{
    if (!s_instance.exists()) {
        s_instance->add(new KisAutoBrushFactory());
        s_instance->add(new KisPredefinedBrushFactory("gbr_brush"));
        s_instance->add(new KisPredefinedBrushFactory("abr_brush"));
        s_instance->add(new KisTextBrushFactory());
        s_instance->add(new KisPredefinedBrushFactory("png_brush"));
        s_instance->add(new KisPredefinedBrushFactory("svg_brush"));
    }
    return s_instance;
}


KoResourceLoadResult KisBrushRegistry::createBrush(const QDomElement& element, KisResourcesInterfaceSP resourcesInterface)
{
    QString brushType = element.attribute("type");

    if (brushType.isEmpty()) {
        return KoResourceSignature(ResourceType::Brushes, "", "unknown", "unknown");
    }

    KisBrushFactory *factory = get(brushType);
    if (!factory) {
        return KoResourceSignature(ResourceType::Brushes, "", "unknown", "unknown");
    }

    return factory->createBrush(element, resourcesInterface);
}

KoResourceLoadResult KisBrushRegistry::createBrush(const KisBrushModel::BrushData &data, KisResourcesInterfaceSP resourcesInterface)
{
    QDomDocument doc;
    QDomElement element = doc.createElement("brush_definition");
    toXML(doc, element, data);
    return createBrush(element, resourcesInterface);
}

std::optional<KisBrushModel::BrushData> KisBrushRegistry::createBrushModel(const QDomElement& element, KisResourcesInterfaceSP resourcesInterface)
{
    QString brushType = element.attribute("type");

    if (brushType.isEmpty()) {
        return std::nullopt;
    }

    KisBrushFactory *factory = get(brushType);

    if (!factory) {
        return std::nullopt;
    }

    return factory->createBrushModel(element, resourcesInterface);
}

void KisBrushRegistry::toXML(QDomDocument &doc, QDomElement &element, const KisBrushModel::BrushData &model)
{
    QString brushType;

    if (model.type == KisBrushModel::Auto) {
        brushType = "auto_brush";
    } else if (model.type == KisBrushModel::Text) {
        brushType = "kis_text_brush";
    } else {
        brushType = model.predefinedBrush.subtype;
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN(!brushType.isEmpty());

    KisBrushFactory *factory = get(brushType);
    KIS_SAFE_ASSERT_RECOVER_RETURN(factory);

    factory->toXML(doc, element, model);
}
