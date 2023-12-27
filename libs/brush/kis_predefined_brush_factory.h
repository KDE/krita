/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_PREDEFINED_BRUSH_FACTORY_H
#define __KIS_PREDEFINED_BRUSH_FACTORY_H

#include <QString>
#include <QDomElement>

#include "kis_brush_factory.h"
#include "kis_brush.h"

#include "kritabrush_export.h"

class BRUSH_EXPORT KisPredefinedBrushFactory : public KisBrushFactory
{
public:
    KisPredefinedBrushFactory(const QString &brushType);

    QString id() const override;
    KoResourceLoadResult createBrush(const KisBrushModel::BrushData &brushData, KisResourcesInterfaceSP resourcesInterface) override;
    KoResourceLoadResult createBrush(const QDomElement& brushDefinition, KisResourcesInterfaceSP resourcesInterface) override;
    std::optional<KisBrushModel::BrushData> createBrushModel(const QDomElement& element, KisResourcesInterfaceSP resourcesInterface) override;
    static void loadFromBrushResource(KisBrushModel::CommonData &commonData, KisBrushModel::PredefinedBrushData &predefinedBrushData, KisBrushSP brushResource);
    void toXML(QDomDocument &doc, QDomElement &element, const KisBrushModel::BrushData &model) override;

private:
    std::variant<KisBrushModel::BrushData, KoResourceSignature> createBrushModelImpl(const QDomElement& element, KisResourcesInterfaceSP resourcesInterface);

private:
    const QString m_id;
};

#endif /* __KIS_PREDEFINED_BRUSH_FACTORY_H */
