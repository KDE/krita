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
 * For the design: see krita's filter configuration objects. Maybe a
 * straight copy is good enough?
 *
 * XXX: Should we move this to KoShape itself?
 */
class KoShapeParameters : KoProperties {

public:

    virtual ~KoShapeParameters(){}

};

/**
 * Contains a KoShapeParameters object that describes the settings of a
 * particular variant of a shape object, together with a name, a description
 * and a pixmap for use in the user interface.
 */
struct KoShapeTemplate {

    QString name;
    QString description;
    QString tooltip;
    QPixmap pixmap;
    KoShapeParameters * params;
};

/**
 * An option widget that can create a KoShapeParameters for the creation
 * of KoShapes.
 */
class KoShapeOptionsWidget : public QWidget {

public:

    /// get the parameters as set in the widget's widget
    virtual KoShapeParameters * shapeParameters() = 0;

    /// set the widget's widgets according to the given parameters
    virtual void setShapeParameters( KoShapeParameters * ) = 0;
};

/**
 * The shape factory can create a flake object without needing a tool or a direct
 * new().
 *
 * XXX: Should we wrap the shapes the factory returns in a shared pointer?
 */
class KoShapeFactory {

public:

    KoShapeFactory() { m_optionWidget = 0; }
    virtual ~KoShapeFactory() {}

    virtual KoID id() { return KoID(m_name, m_description); }

    virtual KoShape * createDefaultShape() = 0;
    virtual KoShape * createShape(KoShapeParameters * params) const = 0;
    virtual KoShape * createShapeFromTemplate(KoShapeTemplate * shapeTemplate) const = 0;

    virtual QWidget * createOptionWidget(QWidget * parent) { Q_UNUSED(parent); return 0; }
    virtual QWidget * optionWidget() const { return m_optionWidget; }

    const QList<KoShapeParameters*> getTemplates() const { return m_templates; }
    const QString & name() const { return m_name; }
    const QString & description() const { return m_description; }
    const QString & tooltip() const { return m_tooltip; }
    const QPixmap & pixmap() const { return m_pixmap; }

protected:

    void addTemplate(KoShapeParameters * params) { m_templates.append(params); }
    void setName(const QString & name) { m_name = name; }
    void setDescription(const QString & description) { m_description = description; }
    void setToolTip(const QString & tooltip) { m_tooltip = tooltip; }
    void setPixmap(const QPixmap & pixmap) { m_pixmap = pixmap; }
    void setOptionWidget(QWidget * widget) { m_optionWidget = widget; }
private:

    QList<KoShapeParameters*> m_templates;
    QString m_name;
    QString m_description;
    QString m_tooltip;
    QPixmap m_pixmap;
    QWidget * m_optionWidget;
};

#endif // _KO_SHAPE_FACTORY_
