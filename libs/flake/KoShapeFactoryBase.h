/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Boudewijn Rempt (boud@valdyas.org)
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2008 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSHAPEFACTORYBASE_H
#define KOSHAPEFACTORYBASE_H

#include <QObject>
#include <QString>
#include <QList>

#include "kritaflake_export.h"

#include <KoXmlReader.h>

class KoShape;
class KoProperties;
class KoShapeConfigWidgetBase;
class KoShapeLoadingContext;
class KoDocumentResourceManager;
class QStringList;

#define SHAPETEMPLATE_MIMETYPE "application/x-flake-shapetemplate"
#define SHAPEID_MIMETYPE "application/x-flake-shapeId"

/**
 * Contains a KoProperties object that describes the settings of a
 * particular variant of a shape object, together with a name, a description
 * and an icon for use in the user interface.
 */
struct KRITAFLAKE_EXPORT KoShapeTemplate {
    KoShapeTemplate() {
        properties = 0;
    }
    QString id;         ///< The id of the shape
    QString templateId;         ///< The id of this particular template - only has to be unique with the shape
    QString name;       ///< The name to be shown for this template
    QString family;       ///< The family of the shape (possible values are: "funny","arrow")
    QString toolTip;    ///< The tooltip text for the template
    QString iconName;       ///< Icon name
    /**
     * The properties which, when passed to the KoShapeFactoryBase::createShape() method
     * result in the shape this template represents.
     */
    const KoProperties *properties;
};

/**
 * A factory for KoShape objects.
 * The baseclass for all shape plugins. Each plugin that ships a KoShape should also
 * ship a factory. That factory will extend this class and set variable data like
 * a toolTip and icon in the constructor of that extending class.
 *
 * An example usage would be:
@code
class MyShapeFactory : public KoShapeFactoryBase {
public:
    MyShapeFactory()
        : KoShapeFactoryBase("MyShape", i18n("My Shape")) {
        setToolTip(i18n("A nice shape"));
    }
    ~MyShapeFactory() {}
    // more methods here
};
@endcode

 * After you created the factory you should create a plugin that can announce the factory to the
 * KoShapeRegistry.  See the KoPluginLoader as well.
 */
class KRITAFLAKE_EXPORT KoShapeFactoryBase : public QObject
{
    Q_OBJECT
public:

    /**
     * Create the new factory
     * @param id a string that will be used internally for referencing the shape
     * @param name the user visible name of the shape this factory creates.
     */
    KoShapeFactoryBase(const QString &id, const QString &name, const QString &deferredPluginName = QString());
    ~KoShapeFactoryBase() override;

    /**
     * Create a list of option panels to show on creating a new shape.
     * The shape type this factory creates may have general or specific setting panels
     * that will be shown after inserting a new shape.
     * The first item in the list will be shown as the first tab in the list of panels,
     * behind all app specific panels.
     * This is a separate list as set by setOptionPanels() and fetched by panelFactories()
     */
    virtual QList<KoShapeConfigWidgetBase*> createShapeOptionPanels() {
        return QList<KoShapeConfigWidgetBase*>();
    }

    /**
     * return the id for the shape this factory creates.
     * @return the id for the shape this factory creates.
     */
    QString id() const;
    /**
     * Return all the templates this factory knows about.
     * Each template shows a different way to create a shape this factory is specialized in.
     */
    QList<KoShapeTemplate> templates() const;
    /**
     * return a translated tooltip Text for a selector of shapes
     * @return a translated tooltip Text
     */
    QString toolTip() const;
    /**
     * return the basename of the icon for a selector of shapes
     * @return the basename of the icon for a selector of shapes
     */
    QString iconName() const;
    /**
     * return the user visible (and translated) name to be seen by the user.
     * @return the user visible (and translated) name to be seen by the user.
     */
    QString name() const;
    /**
     * return the non-visible name of the family the default shape belongs to.
     * @return the family name.
     */
    QString family() const;
    /// lower prio means the shape is more generic and will be checked later
    int loadingPriority() const;

    /**
     * The list of namespaces to the supported elements the factory supports.
     */
    QList<QPair<QString, QStringList> > odfElements() const;

    /// returns true if this shapeFactory is able to load the ODF type
    /// started at argument element. ('draw:line' / 'draw:frame' / etc)
    virtual bool supports(const QDomElement &element, KoShapeLoadingContext &context) const = 0;

    /**
     * The hidden boolean requests if the shape should be hidden in the
     * shape selector or shown with all its templates.
     * The default is false
     * @see setHidden()
     */
    bool hidden() const;

    /**
     * This method is called whenever there is a new document resource
     * manager that is created. The factory may reimplement this in
     * order to get existing resources or put factory specific resources in.
     * In case the factory creates new resources it is advised to parent
     * them to the manager (which is a QObject) for memory management
     * purposes.
     *
     * FIXME: this method is only used by Tables. We should refactor so
     * it is no longer necessary.
     *
     * NOTE: this actually is also used somehow to create the imagecollection
     *        for the picture shape?
     *
     * NOTE: we store the documentmanagers in a list, and remove them
     * from the list on delete.
     *
     * @param manager the new manager
     */
    virtual void newDocumentResourceManager(KoDocumentResourceManager *manager) const;

    /**
     * This method should be implemented by factories to create a shape that the user
     * gets when doing a base insert. For example from a script.  The created shape
     * should have its values set to good defaults that the user can then adjust further if
     * needed.  Including the KoShape:setShapeId(), with the Id from this factory
     * The default shape position is not relevant, it will be moved by the caller.
     * @param documentResources the resources manager that has all the document wide
     *      resources which can be used to create the object.
     * @return a new shape
     * @see createShape() newDocumentResourceManager()
     */
    virtual KoShape *createDefaultShape(KoDocumentResourceManager *documentResources = 0) const;

    /**
     * This method should be implemented by factories to create a shape based on a set of
     * properties that are specifically made for this shape-type.
     * This method should also set this factories shapeId on the shape using KoShape::setShapeId()
     * The default implementation just ignores 'params' and calls createDefaultShape()
     * @return a new shape
     * @param params the parameters to use when creating the shape
     * @param documentResources the resources manager that has all the document wide
     *      resources which can be used to create the object.
     * @see createDefaultShape() newDocumentResourceManager() addTemplate()
     * @see KoShapeTemplate::properties
     */
    virtual KoShape *createShape(const KoProperties *params, KoDocumentResourceManager *documentResources = 0) const;

    /**
     * This method provides the default implementation for creating a shape
     * from a specified xml element of an odf document.
     * Most derived factories do not need to reimplement this method, however if a factory
     * has some special requirements or does something special it is still possible.
     * One example is creating different shapes depending on the content of the passed
     * xml element.
     */
    virtual KoShape *createShapeFromXML(const QDomElement &element, KoShapeLoadingContext &context);

protected:

    /**
     * Add a template with the properties of a specific type of shape this factory can generate
     * using the createShape() method. The factory will take ownership of the properties object
     * to which the member @p properties of @p params points to and destroy it only in its own destructor.
     * @param params the new template this factory knows to produce
     */
    void addTemplate(const KoShapeTemplate &params);

    /**
     * Set the tooltip to be used for a selector of shapes
     * @param tooltip the tooltip
     */
    void setToolTip(const QString &tooltip);

    /**
     * Set an icon to be used in a selector of shapes
     * @param iconName the basename (without extension) of the icon
     */
    void setIconName(const char *iconName);

    /**
     * Set the family name of the default shape
     * @param family the family name of the default shape this factory creates.
     *   for example "funny", "arrows", "geometrics". Use "" for default
     */
    void setFamily(const QString &family);

    /**
     * Set the loading priority for this icon; higher priority means
     * the shape is more specific which means it will be earlier in
     * the queue to try loading a particular odf element.
     */
    void setLoadingPriority(int priority);

    /**
     * Set the namespace and element tags used for quick checking whether this shapefactory
     * is able to create a shape from xml identified by this element
     * name.
     *
     * @param nameSpace the ODF name space (like
     * urn:oasis:names:tc:opendocument:xmlns:text:1.0,
     * take it from KoXmlNS.h)
     * @param elementNames the name of the element itself, like "path"
     *
     */
    void setXmlElementNames(const QString &nameSpace, const QStringList &elementNames);

    /**
     * Set the namespaces and according element tags used for quick checking whether this shapefactory
     * is able to create a shape from xml identified by this element
     * name.
     *
     * @param elementNamesList containing a list of namespace (like
     * urn:oasis:names:tc:opendocument:xmlns:text:1.0,
     * take it from KoXmlNS.h) to a list of elementName of the element itself, like "path"
     */
    void setXmlElements(const QList<QPair<QString, QStringList> > &elementNamesList);

    /**
     * The hidden boolean requests if the shape should be hidden in the
     * shape selector or shown with all its templates.
     * The default is false
     * @see hidden()
     */
    void setHidden(bool hidden);

private:

    void getDeferredPlugin();

private Q_SLOTS:

    /// called whenever a document KoDocumentResourceManager is deleted
    void pruneDocumentResourceManager(QObject *);

private:

    class Private;
    Private * const d;
};

#endif
