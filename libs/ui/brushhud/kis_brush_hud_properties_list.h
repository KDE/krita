/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_BRUSH_HUD_PROPERTIES_LIST_H
#define __KIS_BRUSH_HUD_PROPERTIES_LIST_H

#include <QScopedPointer>
#include <QListWidget>
#include "kis_uniform_paintop_property.h"


class KisBrushHudPropertiesList : public QListWidget
{
public:
    KisBrushHudPropertiesList(QWidget *parent);
    ~KisBrushHudPropertiesList() override;

    void addProperties(const QList<KisUniformPaintOpPropertySP> &properties);
    QList<QString> selectedPropertiesIds() const;

    Qt::DropActions supportedDropActions() const override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_BRUSH_HUD_PROPERTIES_LIST_H */
