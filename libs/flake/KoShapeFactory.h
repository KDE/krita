/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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
#include <QList>

#include <KoID.h>

#include <koffice_export.h>

class KoShape;
class KoProperties;
class KoShapeConfigFactory;
class KoShapeConfigWidgetBase;


/**
 * Contains a KoProperties object that describes the settings of a
 * particular variant of a shape object, together with a name, a description
 * and an icon for use in the user interface.
 */
struct FLAKE_EXPORT KoShapeTemplate {
    QString id;         ///< The id of the shape
    QString name;       ///< The name to be shown for this template
    QString toolTip;    ///< The tooltip text for the template
    QString icon;       ///< Icon name
    /**
     * The properties which, when passed to the KoShapeFactory::createShape() method
     * result in the shape this template represents.
     */
    KoProperties *properties;
};

/**
 * A factory for KoShape objects.
 * The baseclass for all shape plugins. Each plugin that ships a KoShape should also
 * ship a factory. That factory will extend this class and set variable data like
 * a toolTip and icon in the constructor of that extending class.
 *
 * An example usage would be:<pre>
class MyShapeFactory : public KoShapeFactory {
public:
    MyShapeFactory(QObject *parent, const QStringList&)
        : KoShapeFactory(parent, "MyShape", i18n("My Shape")) {
        setToolTip(i18n("A nice shape"));
    }
    ~MyShapeFactory() {}
    // more methods here
};
K_EXPORT_COMPONENT_FACTORY(myLibrary,
     KGenericFactory<MyShapeFactory>( "MyShape" ) )
</pre>

 */
class FLAKE_EXPORT KoShapeFactory : public QObject {
    Q_OBJECT
public:

    /**
     * Create the new factory
     * @param parent the parent QWidget for memory management usage.
     * @param id a string that will be used internally for referencing the shape, for
     *   example for use by the KoShape::sigActivateTemporary.
     * @param name the user visible name of the tool this factory creates.
     */
    KoShapeFactory(QObject *parent, const QString &id, const QString &name);
    virtual ~KoShapeFactory() {}

    /**
     * This method should be implemented by factories to create a shape that the user
     * gets when doing a base insert. For example from a script.  The created shape
     * should have its values set to good defaults that the user can then adjust further if
     * needed.  Including the KoShape:setShapeId(), with the Id from this factory
     * The default shape position is not relevant, it will be moved by the caller.
     * @return a new shape
     */
    virtual KoShape * createDefaultShape() = 0;
    /**
     * This method should be implemented by factories to create a shape based on a set of
     * properties that are specifically made for this shape-type.
     * This method should also set this factories shapeId on the shape using KoShape::setShapeId()
     * @return a new shape
     * @see KoShapeTemplate::properties
     */
    virtual KoShape * createShape(const KoProperties * params) const = 0;
    /**
     * Create a list of option panels to show on creating a new shape.
     * The shape type this factory creates may have general or specific setting panels
     * that will be shown after inserting a new shape.
     * The first item in the list will be shown as the first tab in the list of panels,
     * behind all app specific panels.
     */
    virtual QList<KoShapeConfigWidgetBase*> createShapeOptionPanels() {
        return QList<KoShapeConfigWidgetBase*>();
    }

    /**
     * Set panel factories to show config options after creating a new shape.
     * The application that lets the user create shapes is able to set option
     * widgets that will be shown after the user inserted a new shape of the
     * type that this factory presents.
     * Example:
     *  @code
     *  // Init shape Factories with our frame based configuration panels.
     *  QList<KoShapeConfigFactory *> panels;
     *  panels.append(new AppConfigFactory()); // insert some factory
     *  foreach(KoID id, KoShapeRegistry::instance()->listKeys())
     *      KoShapeRegistry::instance()->get(id)->setOptionPanels(panels);
     *  @endcode
     */
    void setOptionPanels(QList<KoShapeConfigFactory*> &panelFactories);

    /**
     * Return the app-specific panels.
     * @see setOptionPanels
     */
    const QList<KoShapeConfigFactory*> &panelFactories();

    /**
     * Create a KoID for the shape this factory creates.
     */
    const KoID id() const;
    /**
     * return the id for the shape this factory creates.
     * @return the id for the shape this factory creates.
     */
    const QString & shapeId() const;
    /**
     * Return all the templates this factory knows about.
     * Each template shows a different way to create a shape this factory is specialized in.
     */
    const QList<KoShapeTemplate> templates() const { return m_templates; }
    /**
     * return a translated tooltip Text for a selector of shapes
     * @return a translated tooltip Text
     */
    const QString & toolTip() const;
    /**
     * return the basename of i the icon for this tool for a selector of shapes
     * @return the basename of the icon for this selector of shapes
     */
    const QString & icon() const;
    /**
     * return the user visible (and translated) name to be seen by the user.
     * @return the user visible (and translated) name to be seen by the user.
     */
    const QString & name() const;

protected:

    /**
     * Add a template with the properties of a speficic type of shape this factory can generate
     * using the createShape() method.
     * @param params the new template this factory knows to produce
     */
    void addTemplate(KoShapeTemplate params);
    /**
     * Set the tooltip to be used for a selector of shapes
     * @param tooltip the tooltip
     */
    void setToolTip(const QString & tooltip);
    /**
     * Set an icon to be used in a selector of shapes
     * @param iconName the basename (without extention) of the icon
     * @see KIconLoader
     */
    void setIcon(const QString & iconName);

private:

    QList<KoShapeTemplate> m_templates;
    QList<KoShapeConfigFactory*> m_configPanels;
    QString m_tooltip, m_iconName;
    const QString m_id, m_name;
};

#endif // _KO_SHAPE_FACTORY_
