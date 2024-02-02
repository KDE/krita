/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_BRUSH_HUD_PROPERTIES_CONFIG_H
#define __KIS_BRUSH_HUD_PROPERTIES_CONFIG_H

#include <QScopedPointer>
#include "kritaui_export.h"

#include "brushengine/kis_uniform_paintop_property.h"

class QDomDocument;
class QStringList;
class QString;


class KRITAUI_EXPORT KisBrushHudPropertiesConfig
{
public:
    KisBrushHudPropertiesConfig();
    ~KisBrushHudPropertiesConfig();

    void setSelectedProperties(const QString &paintOpId, const QList<QString> &ids);
    QList<QString> selectedProperties(const QString &paintOpId) const;

    void filterProperties(const QString &paintOpId,
                          const QList<KisUniformPaintOpPropertySP> &allProperties,
                          QList<KisUniformPaintOpPropertySP> *chosenProperties,
                          QList<KisUniformPaintOpPropertySP> *skippedProperties) const;

public:
    QDomDocument *testingGetDocument();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_BRUSH_HUD_PROPERTIES_CONFIG_H */
