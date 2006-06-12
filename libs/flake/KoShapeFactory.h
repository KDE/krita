/*
 * KoShapeFactory.h -- Part of KOffice
 *
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KO_SHAPE_FACTORY_
#define _KO_SHAPE_FACTORY_

#include <QString>
#include <QWidget>
#include <QPixmap>
#include <QList>

#include <KoProperties.h>
#include <KoID.h>
#include "KoShape.h"

/**
 * Contains a KoProperties object that describes the settings of a
 * particular variant of a shape object, together with a name, a description
 * and a pixmap for use in the user interface.
 */
struct KoShapeTemplate {

    QString name;
    QString description;
    QString tooltip;
    QPixmap pixmap;
    KoProperties * params;
};

/**
 * The shape factory can create a KoShape without needing a tool or a direct new().
 */
class KoShapeFactory {
public:

    /// Factory for shapes
    KoShapeFactory() { m_optionWidget = 0; }
    virtual ~KoShapeFactory() {}

    virtual KoID id() { return m_id; }

    virtual KoShape * createDefaultShape() = 0;
    virtual KoShape * createShape(KoProperties * params) const = 0;
    virtual KoShape * createShapeFromTemplate(KoShapeTemplate * shapeTemplate) const = 0;

    virtual QWidget * optionWidget() const { return m_optionWidget; }

    const QList<KoProperties*> templates() const { return m_templates; }
    const QString & toolTip() const { return m_tooltip; }
    const QPixmap & icon() const { return m_icon; }

protected:

    void addTemplate(KoProperties * params) { m_templates.append(params); }
    void setId(KoID id) { m_id = id; }
    void setToolTip(const QString & tooltip) { m_tooltip = tooltip; }
    void setIcon(const QPixmap & icon) { m_icon = icon; }
    void setOptionWidget(QWidget * widget) { m_optionWidget = widget; }

private:

    QList<KoProperties*> m_templates;
    QString m_tooltip;
    QPixmap m_icon;
    QWidget * m_optionWidget;
    KoID m_id;
};

#endif // _KO_SHAPE_FACTORY_
