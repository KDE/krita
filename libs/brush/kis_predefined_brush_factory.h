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


class KisPredefinedBrushFactory : public KisBrushFactory
{
public:
    KisPredefinedBrushFactory(const QString &brushType);

    QString id() const override;
    KisBrushSP createBrush(const QDomElement& brushDefinition, KisResourcesInterfaceSP resourcesInterface) override;

private:
    const QString m_id;
};

#endif /* __KIS_PREDEFINED_BRUSH_FACTORY_H */
