/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (C) 2006-2007, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006,2008-2010 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2010 Inge Wallin <inge@lysator.liu.se>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

// Own
#include "KoShapeRegistry.h"

#include "KoSvgTextShape.h"
#include "KoPathShapeFactory.h"
#include "KoConnectionShapeFactory.h"
#include "KoShapeLoadingContext.h"
#include "KoShapeSavingContext.h"
#include "KoShapeGroup.h"
#include "KoShapeLayer.h"
#include "SvgShapeFactory.h"

#include <KoPluginLoader.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoOdfLoadingContext.h>
#include <KoStyleStack.h>

#include <QString>
#include <QHash>
#include <QMultiMap>
#include <QPainter>
#include <QGlobalStatic>

#include <FlakeDebug.h>

Q_GLOBAL_STATIC(KoShapeRegistry, s_instance)

class Q_DECL_HIDDEN KoShapeRegistry::Private
{
public:
    void insertFactory(KoShapeFactoryBase *factory);
    void init(KoShapeRegistry *q);

    KoShape *createShapeInternal(const KoXmlElement &fullElement, KoShapeLoadingContext &context, const KoXmlElement &element) const;

    // Map namespace,tagname to priority:factory
    QHash<QPair<QString, QString>, QMultiMap<int, KoShapeFactoryBase*> > factoryMap;
};

KoShapeRegistry::KoShapeRegistry()
        : d(new Private())
{
}

KoShapeRegistry::~KoShapeRegistry()
{
    qDeleteAll(doubleEntries());
    qDeleteAll(values());
    delete d;
}

void KoShapeRegistry::Private::init(KoShapeRegistry *q)
{
    KoPluginLoader::PluginsConfig config;
    config.whiteList = "FlakePlugins";
    config.blacklist = "FlakePluginsDisabled";
    config.group = "calligra";
    KoPluginLoader::instance()->load(QString::fromLatin1("Calligra/Flake"),
                                     QString::fromLatin1("[X-Flake-PluginVersion] == 28"),
                                     config);
    config.whiteList = "ShapePlugins";
    config.blacklist = "ShapePluginsDisabled";
    KoPluginLoader::instance()->load(QString::fromLatin1("Calligra/Shape"),
                                     QString::fromLatin1("[X-Flake-PluginVersion] == 28"),
                                     config);

    // Also add our hard-coded basic shapes
    q->add(new KoSvgTextShapeFactory());
    q->add(new KoPathShapeFactory(QStringList()));
    q->add(new KoConnectionShapeFactory());
    // As long as there is no shape dealing with embedded svg images
    // we add the svg shape factory here by default
    q->add(new SvgShapeFactory);

    // Now all shape factories are registered with us, determine their
    // associated odf tagname & priority and prepare ourselves for
    // loading ODF.

    QList<KoShapeFactoryBase*> factories = q->values();
    for (int i = 0; i < factories.size(); ++i) {
        insertFactory(factories[i]);
    }
}

KoShapeRegistry* KoShapeRegistry::instance()
{
    if (!s_instance.exists()) {
        s_instance->d->init(s_instance);
    }
    return s_instance;
}

void KoShapeRegistry::addFactory(KoShapeFactoryBase * factory)
{
    add(factory);
    d->insertFactory(factory);
}

void KoShapeRegistry::Private::insertFactory(KoShapeFactoryBase *factory)
{
    const QList<QPair<QString, QStringList> > odfElements(factory->odfElements());

    if (odfElements.isEmpty()) {
        debugFlake << "Shape factory" << factory->id() << " does not have OdfNamespace defined, ignoring";
    }
    else {
        int priority = factory->loadingPriority();
        for (QList<QPair<QString, QStringList> >::const_iterator it(odfElements.begin()); it != odfElements.end(); ++it) {
            foreach (const QString &elementName, (*it).second) {
                QPair<QString, QString> p((*it).first, elementName);

                QMultiMap<int, KoShapeFactoryBase*> & priorityMap = factoryMap[p];

                priorityMap.insert(priority, factory);

                debugFlake << "Inserting factory" << factory->id() << " for"
                    << p << " with priority "
                    << priority << " into factoryMap making "
                    << priorityMap.size() << " entries. ";
            }
        }
    }
}

#include <KoXmlWriter.h>
#include <QBuffer>
#include <KoStore.h>
#include <boost/optional.hpp>

namespace {

struct ObjectEntry {
    ObjectEntry()
    {
    }

    ObjectEntry(const ObjectEntry &rhs)
        : objectXmlContents(rhs.objectXmlContents),
          objectName(rhs.objectName),
          isDir(rhs.isDir)
    {
    }

    ~ObjectEntry()
    {
    }

    QByteArray objectXmlContents; // the XML tree in the object
    QString objectName;       // object name in the frame without "./"
    // This is extracted from objectXmlContents.
    bool isDir = false;
};

// A FileEntry is used to store information about embedded files
// inside (i.e. referred to by) an object.
struct FileEntry {
    FileEntry() {}
    FileEntry(const FileEntry &rhs)
        : path(rhs.path),
          mimeType(rhs.mimeType),
          isDir(rhs.isDir),
          contents(rhs.contents)
    {
    }

    QString path;           // Normalized filename, i.e. without "./".
    QString mimeType;
    bool  isDir;
    QByteArray contents;
};

QByteArray loadFile(const QString &fileName, KoShapeLoadingContext &context)
{
    // Can't load a file which is a directory, return an invalid QByteArray
    if (fileName.endsWith('/'))
        return QByteArray();

    KoStore *store = context.odfLoadingContext().store();
    QByteArray fileContent;

    if (!store->open(fileName)) {
        store->close();
        return QByteArray();
    }

    int fileSize = store->size();
    fileContent = store->read(fileSize);
    store->close();

    //debugFlake << "File content: " << fileContent;
    return fileContent;
}


boost::optional<FileEntry> storeFile(const QString &fileName, KoShapeLoadingContext &context)
{
    debugFlake << "Saving file: " << fileName;

    boost::optional<FileEntry> result;

    QByteArray fileContent = loadFile(fileName, context);
    if (!fileContent.isNull()) {

        // Actually store the file in the list.
        FileEntry entry;
        entry.path = fileName;
        if (entry.path.startsWith(QLatin1String("./"))) {
            entry.path.remove(0, 2);
        }
        entry.mimeType = context.odfLoadingContext().mimeTypeForPath(entry.path);
        entry.isDir = false;
        entry.contents = fileContent;

        result = entry;
    }

    return result;
}

void storeXmlRecursive(const KoXmlElement &el, KoXmlWriter &writer,
                       ObjectEntry *object, QHash<QString, QString> &unknownNamespaces)
{
    // Start the element;
    // keep the name in a QByteArray so that it stays valid until end element is called.
    const QByteArray name(el.nodeName().toLatin1());
    writer.startElement(name.constData());

    // Child elements
    // Loop through all the child elements of the draw:frame.
    KoXmlNode n = el.firstChild();
    for (; !n.isNull(); n = n.nextSibling()) {
        if (n.isElement()) {
            storeXmlRecursive(n.toElement(), writer, object, unknownNamespaces);
        }
        else if (n.isText()) {
            writer.addTextNode(n.toText().data()/*.toUtf8()*/);
        }
    }

    // End the element
    writer.endElement();
}


QVector<ObjectEntry> storeObjects(const KoXmlElement &element)
{
    QVector<ObjectEntry> result;

    // Loop through all the child elements of the draw:frame and save them.
    KoXmlNode n = element.firstChild();
    for (; !n.isNull(); n = n.nextSibling()) {
        debugFlake << "In draw:frame, node =" << n.nodeName();

        // This disregards #text, but that's not in the spec anyway so
        // it doesn't need to be saved.
        if (!n.isElement())
            continue;
        KoXmlElement el = n.toElement();

        ObjectEntry object;

        QByteArray contentsTmp;
        QBuffer buffer(&contentsTmp); // the member
        KoXmlWriter writer(&buffer);

        // 1. Find out the objectName
        // Save the normalized filename, i.e. without a starting "./".
        // An empty string is saved if no name is found.
        QString  name = el.attributeNS(KoXmlNS::xlink, "href", QString());
        if (name.startsWith(QLatin1String("./")))
            name.remove(0, 2);
        object.objectName = name;

        // 2. Copy the XML code.
        QHash<QString, QString> unknownNamespaces;
        storeXmlRecursive(el, writer, &object, unknownNamespaces);
        object.objectXmlContents = contentsTmp;

        // 3, 4: the isDir and manifestEntry members are not set here,
        // but initialize them anyway. .
        object.isDir = false;  // Has to be initialized to something.

        result.append(object);
    }

    return result;
}
}

#include <svg/SvgShapeFactory.h>
#include "kis_debug.h"
#include <QMimeDatabase>
#include <KoUnit.h>
#include <KoDocumentResourceManager.h>
#include <KoShapeController.h>
#include <KoShapeGroupCommand.h>


KoShape * KoShapeRegistry::createShapeFromOdf(const KoXmlElement & e, KoShapeLoadingContext & context) const
{
    debugFlake << "Going to check for" << e.namespaceURI() << ":" << e.tagName();

    KoShape * shape = 0;

    // Handle the case where the element is a draw:frame differently from other cases.
    if (e.tagName() == "frame" && e.namespaceURI() == KoXmlNS::draw) {
        // If the element is in a frame, the frame is already added by the
        // application and we only want to create a shape from the
        // embedded element. The very first shape we create is accepted.
        //
        // FIXME: we might want to have some code to determine which is
        //        the "best" of the creatable shapes.

        if (e.hasChildNodes()) {
            // if we don't ignore white spaces it can be that the first child is not a element so look for the first element
            KoXmlNode node = e.firstChild();
            KoXmlElement element;
            while (!node.isNull() && element.isNull()) {
                element = node.toElement();
                node = node.nextSibling();
            }

            if (!element.isNull()) {
                // Check for draw:object
                if (element.tagName() == "object" && element.namespaceURI() == KoXmlNS::draw && element.hasChildNodes()) {
                    // Loop through the elements and find the first one
                    // that is handled by any shape.
                    KoXmlNode n = element.firstChild();
                    for (; !n.isNull(); n = n.nextSibling()) {
                        if (n.isElement()) {
                            debugFlake << "trying for element " << n.toElement().tagName();
                            shape = d->createShapeInternal(e, context, n.toElement());
                            break;
                        }
                    }
                    if (shape)
                        debugFlake << "Found a shape for draw:object";
                    else
                        debugFlake << "Found NO shape shape for draw:object";
                }
                else {
                    // If not draw:object, e.g draw:image or draw:plugin
                    shape = d->createShapeInternal(e, context, element);
                }
            }

            if (shape) {
                debugFlake << "A shape supporting the requested type was found.";
            }
            else {
                // If none of the registered shapes could handle the frame
                // contents, try to fetch SVG it from an embedded link

                const KoXmlElement &frameElement = e;
                const int frameZIndex = SvgShapeFactory::calculateZIndex(frameElement, context);

                QList<KoShape*> resultShapes;

                QVector<ObjectEntry> objects = storeObjects(frameElement);
                Q_FOREACH (const ObjectEntry &object, objects) {
                    if (object.objectName.isEmpty()) continue;

                    boost::optional<FileEntry> file = storeFile(object.objectName, context);
                    if (file && !file->contents.isEmpty()) {
                        QMimeDatabase db;
                        QMimeType mime = db.mimeTypeForData(file->contents);

                        const int zIndex = SvgShapeFactory::calculateZIndex(element, context);

                        if (mime.inherits("image/svg+xml")) {


                            KoXmlDocument xmlDoc;

                            int line, col;
                            QString errormessage;
                            const bool parsed = xmlDoc.setContent(file->contents, &errormessage, &line, &col);
                            if (!parsed) continue;

                            const QRectF bounds = context.documentResourceManager()->documentRectInPixels();

                            // WARNING: Krita 3.x expects all the embedded objects to
                            //          be loaded in default resolution of 72.0 ppi.
                            //          Don't change it to the correct data in the image,
                            //          it will change back compatibility (and this code will
                            //          be deprecated some time soon
                            // UPDATE (DK): There is actually no difference in what resolution we
                            //              load these shapes, because they will be scaled into
                            //              the bounds of the parent odf-frame
                            const qreal pixelsPerInch = 72.0;
                            const qreal forcedFontSizeResolution = 72.0;

                            QPointF pos;
                            pos.setX(KoUnit::parseValue(frameElement.attributeNS(KoXmlNS::svg, "x", QString::number(bounds.x()))));
                            pos.setY(KoUnit::parseValue(frameElement.attributeNS(KoXmlNS::svg, "y", QString::number(bounds.y()))));

                            QSizeF size;
                            size.setWidth(KoUnit::parseValue(frameElement.attributeNS(KoXmlNS::svg, "width", QString::number(bounds.width()))));
                            size.setHeight(KoUnit::parseValue(frameElement.attributeNS(KoXmlNS::svg, "height", QString::number(bounds.height()))));

                            KoShape *shape = SvgShapeFactory::createShapeFromSvgDirect(xmlDoc.documentElement(),
                                                                                       QRectF(pos, size),
                                                                                       pixelsPerInch,
                                                                                       forcedFontSizeResolution,
                                                                                       zIndex,
                                                                                       context);

                            if (shape) {
                                // NOTE: here we are expected to stretch the internal to the bounds of
                                //       the frame! Sounds weird, but it is what Krita 3.x did.

                                const QRectF shapeRect = shape->absoluteOutlineRect();
                                const QPointF offset = shapeRect.topLeft();
                                const QSizeF fragmentSize = shapeRect.size();

                                if (fragmentSize.isValid()) {
                                    /**
                                     * Yes, you see what you see. The previous versions of Krita used
                                     * QSvgRenderer to render the object, which allegedly truncated the
                                     * object on sides. Even though we don't use pre-rendering now,
                                     * we should still reproduce the old way...
                                     */
                                    const QSizeF newSize = QSizeF(int(size.width()), int(size.height()));

                                    shape->applyAbsoluteTransformation(
                                                QTransform::fromTranslate(-offset.x(), -offset.y()) *
                                                QTransform::fromScale(
                                                    newSize.width() / fragmentSize.width(),
                                                    newSize.height() / fragmentSize.height()) *
                                                QTransform::fromTranslate(pos.x(), pos.y()));
                                    resultShapes.append(shape);
                                }
                            }

                        } else {
                            // TODO: implement raster images?
                        }
                    }
                }

                if (resultShapes.size() == 1) {
                    shape = resultShapes.takeLast();
                } else if (resultShapes.size() > 1) {
                    KoShapeGroup *groupShape = new KoShapeGroup;
                    KoShapeGroupCommand cmd(groupShape, resultShapes);
                    cmd.redo();
                    groupShape->setZIndex(frameZIndex);
                    shape = groupShape;
                }
            }
        }
    }

    // Hardwire the group shape into the loading as it should not appear
    // in the shape selector
    else if (e.localName() == "g" && e.namespaceURI() == KoXmlNS::draw) {
        KoShapeGroup * group = new KoShapeGroup();

        context.odfLoadingContext().styleStack().save();
        bool loaded = group->loadOdf(e, context);
        context.odfLoadingContext().styleStack().restore();

        if (loaded) {
            shape = group;
        }
        else {
            delete group;
        }
    } else {
        shape = d->createShapeInternal(e, context, e);
    }

    if (shape) {
        context.shapeLoaded(shape);
    }

    return shape;
}

KoShape *KoShapeRegistry::Private::createShapeInternal(const KoXmlElement &fullElement,
                                                       KoShapeLoadingContext &context,
                                                       const KoXmlElement &element) const
{
    // Pair of namespace, tagname
    QPair<QString, QString> p = QPair<QString, QString>(element.namespaceURI(), element.tagName());

    // Remove duplicate lookup.
    if (!factoryMap.contains(p))
        return 0;

    QMultiMap<int, KoShapeFactoryBase*> priorityMap = factoryMap.value(p);
    QList<KoShapeFactoryBase*> factories = priorityMap.values();

#ifndef NDEBUG
    debugFlake << "Supported factories for=" << p;
    foreach (KoShapeFactoryBase *f, factories)
        debugFlake << f->id() << f->name();
#endif

    // Loop through all shape factories. If any of them supports this
    // element, then we let the factory create a shape from it. This
    // may fail because the element itself is too generic to draw any
    // real conclusions from it - we actually have to try to load it.
    // An example of this is the draw:image element which have
    // potentially hundreds of different image formats to support,
    // including vector formats.
    //
    // If it succeeds, then we use this shape, if it fails, then just
    // try the next.
    //
    // Higher numbers are more specific, map is sorted by keys.
    for (int i = factories.size() - 1; i >= 0; --i) {
        KoShapeFactoryBase * factory = factories[i];
        if (factory->supports(element, context)) {
            KoShape *shape = factory->createShapeFromOdf(fullElement, context);
            if (shape) {
                debugFlake << "Shape found for factory " << factory->id() << factory->name();
                // we return the top-level most shape as that's the one that we'll have to
                // add to the KoShapeManager for painting later (and also to avoid memory leaks)
                // but don't go past a KoShapeLayer as KoShape adds those from the context
                // during loading and those are already added.
                while (shape->parent() && dynamic_cast<KoShapeLayer*>(shape->parent()) == 0)
                    shape = shape->parent();

                return shape;
            }
            // Maybe a shape with a lower priority can load our
            // element, but this attempt has failed.
        }
        else {
            debugFlake << "No support for" << p << "by" << factory->id();
        }
    }

    return 0;
}

QList<KoShapeFactoryBase*> KoShapeRegistry::factoriesForElement(const QString &nameSpace, const QString &elementName)
{
    // Pair of namespace, tagname
    QPair<QString, QString> p = QPair<QString, QString>(nameSpace, elementName);

    QMultiMap<int, KoShapeFactoryBase*> priorityMap = d->factoryMap.value(p);
    QList<KoShapeFactoryBase*> shapeFactories;
    // sort list by priority
    Q_FOREACH (KoShapeFactoryBase *f, priorityMap.values()) {
        shapeFactories.prepend(f);
    }

    return shapeFactories;
}
