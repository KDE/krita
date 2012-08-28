/* This file is part of the KDE project
 *
 * Copyright (C) 2010-2011 Inge Wallin <inge@lysator.liu.se>
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
#include "KoUnavailShape.h"

// Qt
#include <QPen>
#include <QPainter>
#include <QByteArray>
#include <QBuffer>
#include <QDataStream>
#include <QPixmap>
#include <QStringList>
#include <QSvgRenderer>

// KDE
#include <kstandarddirs.h>
#include <KDebug>

// Calligra
#include "KoUnit.h"
#include "KoStore.h"
#include "KoXmlNS.h"
#include "KoXmlReader.h"
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoOdfManifestEntry.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoEmbeddedDocumentSaver.h>
#include "KoShapeContainerDefaultModel.h"
#include "KoShapeRegistry.h"
#include <KoShapeBackground.h>


// The XML of a frame looks something like this:
// 
// 1. <draw:frame ...attributes...>
// 2.   <draw:object xlink:href="./Object1" ...more attributes>
// 3.   <draw:image xlink:href="./ObjectReplacements/Object1" ...more attributes>
// 4. </draw:frame>
//
// or
// 
// 1. <draw:frame ...attributes...>
// 2.   <math:math>...inline xml here...</math:math>    
// 3.   <draw:image xlink:href="./ObjectReplacements/Object1" ...more attributes>
// 4. </draw:frame>
//
// We define each Xml statement on lines 2 and 3 above as an "object".
// (Strictly only the first child element is an object in the ODF sense,
// but we have to have some terminology here.)
// 
// In an ODF frame, only the first line, i.e. the first object
// contains the real contents.  All the rest of the objects are used /
// shown if we cannot handle the first one.  The most common cases are
// that there is only one object inside the frame OR that there are 2
// and the 2nd is a picture.
//
// Sometimes, e.g. in the case of an embedded document, the reference
// points not to a file but to a directory structure inside the ODF
// store. 
//
// When we load and save in the UnavailShape, we have to be general
// enough to cover all possible cases of references and inline XML,
// embedded files and embedded directory structures.
//
// We also have to be careful because we cannot reuse the object names
// that are in the original files when saving.  Instead we need to
// create new object names because the ones that were used in the
// original file may already be used by other embedded files/objects
// that are saved by other shapes.
//
// FIXME: There should only be ONE place where new object / file names
//        are generated, not 2(?) like there are now:
//        KoEmbeddedDocumentSaver and the KoImageCollection.
//


// An ObjectEntry is used to store information about objects in the
// frame, as defined above.
struct ObjectEntry {
    QByteArray objectXmlContents; // the XML tree in the object
    QString objectName;       // object name in the frame without "./"
    // This is extracted from objectXmlContents.
    bool isDir;
    KoOdfManifestEntry *manifestEntry; // manifest entry for the above.
};

// A FileEntry is used to store information about embedded files
// inside (i.e. referred to by) an object.
struct FileEntry {
    QString path;           // Normalized filename, i.e. without "./".
    QString mimeType;
    bool  isDir;
    QByteArray contents;
};


class KoUnavailShape::Private
{
public:
    Private(KoUnavailShape* qq);
    ~Private();

    void draw(QPainter &painter) const;
    void drawNull(QPainter &painter) const;

    void storeObjects(const KoXmlElement &element);
    void storeXmlRecursive(const KoXmlElement &el, KoXmlWriter &writer,
                           ObjectEntry *object, QHash<QString, QString> &unknownNamespaces);
    void storeFile(const QString &filename, KoShapeLoadingContext &context);
    QByteArray loadFile(const QString &filename, KoShapeLoadingContext &context);

    // Objects inside the frame.  For each file, we store:
    //  - The XML code for the object
    //  - Any embedded files (names, contents) that are referenced by xlink:href
    //  - Whether they are directories, i.e. if they contain a file tree and not just one file.
    //  - The manifest entries
    QList<ObjectEntry*> objectEntries;

    // Embedded files
    QList<FileEntry*> embeddedFiles; // List of embedded files.

    // Some cached values.
    QPixmap questionMark;
    QPixmap pixmapPreview;
    QSvgRenderer *scalablePreview;

    KoUnavailShape* q;
};

KoUnavailShape::Private::Private(KoUnavailShape* qq)
: scalablePreview(new QSvgRenderer())
, q(qq)
{
    // Get the question mark "icon".
    questionMark.load(KStandardDirs::locate("data", "calligra/icons/questionmark.png"));
}

KoUnavailShape::Private::~Private()
{
    qDeleteAll(objectEntries);
    qDeleteAll(embeddedFiles);

    // It's a QObject, but we haven't parented it.
    delete(scalablePreview);
}


// ----------------------------------------------------------------
//                         The main class


KoUnavailShape::KoUnavailShape()
: KoFrameShape( "", "" )
, KoShapeContainer(new KoShapeContainerDefaultModel())
, d(new Private(this))
{
    setShapeId(KoUnavailShape_SHAPEID);

    // Default size of the shape.
    KoShape::setSize( QSizeF( CM_TO_POINT( 5 ), CM_TO_POINT( 3 ) ) );
}

KoUnavailShape::~KoUnavailShape()
{
    delete d;
}


void KoUnavailShape::paint(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintContext)
{
    applyConversion(painter, converter);

    // If the frame is empty, just draw a background.
    kDebug(30006) << "Number of objects:" << d->objectEntries.size();
    if (d->objectEntries.isEmpty()) {
        // But... only try to draw the background if there's one such
        if (background()) {
            QPainterPath p;
            p.addRect(QRectF(QPointF(), size()));
            background()->paint(painter, converter, paintContext, p);
        }
    } else {
        if(shapes().isEmpty()) {
            d->draw(painter);
        }
    }
}

void KoUnavailShape::paintComponent(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

void KoUnavailShape::Private::draw(QPainter &painter) const
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    // Run through the previews in order of preference. Draw a placeholder
    // questionmark if there is no preview available for rendering.
    if (scalablePreview->isValid()) {
        QRect bounds(0, 0, q->boundingRect().width(), q->boundingRect().height());
        scalablePreview->render(&painter, bounds);
    }
    else if (!pixmapPreview.isNull()) {
        QRect bounds(0, 0, q->boundingRect().width(), q->boundingRect().height());
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawPixmap(bounds, pixmapPreview);
    }
    else if (q->shapes().isEmpty()) {
        // Draw a nice question mark with a frame around it if there
        // is no other preview image. If there is a contained image
        // shape, we don't need to draw anything.

        // Get the question mark "icon".
        // FIXME: We should be able to use d->questionMark here.
        QPixmap questionMark;
        questionMark.load(KStandardDirs::locate("data", "calligra/icons/questionmark.png"));

        // The size of the image is:
        //  - the size of the shape if  shapesize < 2cm
        //  - 2 cm                  if  2cm <= shapesize <= 8cm
        //  - shapesize / 4         if  shapesize > 8cm
        qreal  width = q->size().width();
        qreal  height = q->size().height();
        qreal  picSize = CM_TO_POINT(2); // Default size is 2 cm.
        if (width < CM_TO_POINT(2) || height < CM_TO_POINT(2))
            picSize = qMin(width, height);
        else if (width > CM_TO_POINT(8) && height > CM_TO_POINT(8))
            picSize = qMin(width, height) / qreal(4.0);

        painter.drawPixmap((width - picSize) / qreal(2.0), (height - picSize) / qreal(2.0),
                           picSize, picSize, questionMark);

        // Draw a gray rectangle around the shape.
        painter.setPen(QPen(QColor(172, 196, 206)));
        painter.drawRect(QRectF(QPointF(0,0), q->size()));

    }
    painter.restore();
}

void KoUnavailShape::Private::drawNull(QPainter &painter) const
{
    QRectF  rect(QPointF(0,0), q->size());
    painter.save();

    // Draw a simple cross in a rectangle just to indicate that there is something here.
    painter.drawLine(rect.topLeft(), rect.bottomRight());
    painter.drawLine(rect.bottomLeft(), rect.topRight());

    painter.restore();
}


// ----------------------------------------------------------------
//                         Loading and Saving


void KoUnavailShape::saveOdf(KoShapeSavingContext & context) const
{
    kDebug(30006) << "START SAVING ##################################################";

    KoEmbeddedDocumentSaver &fileSaver = context.embeddedSaver();
    KoXmlWriter &writer = context.xmlWriter();

    writer.startElement("draw:frame");

    // See also loadOdf() in loadOdfAttributes.
    saveOdfAttributes( context, OdfAllAttributes );

    // Write the stored XML to the file, but don't reuse object names.
    int lap = 0;
    QString newName;
    foreach (const ObjectEntry *object, d->objectEntries) {
        QByteArray xmlArray(object->objectXmlContents);
        QString objectName(object->objectName); // Possibly empty.
        KoOdfManifestEntry *manifestEntry(object->manifestEntry);

        // Create a name for this object. If this is not the first
        // object, i.e. a replacement object (most likely a picture),
        // then reuse the name but put it in ReplacementObjects.
        if (++lap == 1) {
            // The first lap in the loop is the actual object.  All
            // other laps are replacement objects.
            newName = fileSaver.getFilename("Object ");
        }
        else if (lap == 2) {
            newName = "ObjectReplacements/" + newName;
        }
        else
            // FIXME: what should replacement 2 and onwards be called?
            newName = newName + "_";

        // If there was a previous object name, replace it with the new one.
        if (!objectName.isEmpty() && manifestEntry) {
            // FIXME: We must make a copy of the byte array here because
            //        otherwise we won't be able to save > 1 time.
            xmlArray.replace(objectName.toLatin1(), newName.toLatin1());
        }

        writer.addCompleteElement(xmlArray.data());

        // If the objectName is empty, this may be inline XML.
        // If so, we are done now.
        if (objectName.isEmpty() || !manifestEntry) {
            continue;
        }

        // Save embedded files for this object.
        foreach (FileEntry *entry, d->embeddedFiles) {
            QString  fileName(entry->path);

            // If we found a file for this object, we need to write it
            // but with the new object name instead of the old one.
            if (!fileName.startsWith(objectName))
                continue;

            kDebug(30006) << "Object name: " << objectName << "newName: " << newName
            << "filename: " << fileName << "isDir: " << entry->isDir;

            fileName.replace(objectName, newName);
            fileName.prepend("./");
            kDebug(30006) << "New filename: " << fileName;

            // FIXME: Check if we need special treatment of directories.
            fileSaver.saveFile(fileName, entry->mimeType.toLatin1(), entry->contents);
        }

        // Write the manifest entry for the object itself.  If it's a
        // file, the manifest is already written by saveFile, so skip
        // it here.
        if (object->isDir) {
            fileSaver.saveManifestEntry(newName + '/', manifestEntry->mediaType(),
                                        manifestEntry->version());
        }
    }

    writer.endElement(); // draw:frame
}


bool KoUnavailShape::loadOdf(const KoXmlElement &frameElement, KoShapeLoadingContext &context)
{
    kDebug(30006) << "START LOADING ##################################################";
    //kDebug(30006) << "Loading ODF frame in the KoUnavailShape. Element = "
    //              << frameElement.tagName();

    loadOdfAttributes(frameElement, context, OdfAllAttributes);

    // NOTE: We cannot use loadOdfFrame() because we want to save all
    //       the things inside the frame, not just one of them, like
    //       loadOdfFrame() provides.

    // Get the manifest.
    QList<KoOdfManifestEntry*> manifest = context.odfLoadingContext().manifestEntries();

#if 0   // Enable to show all manifest entries.
    kDebug(30006) << "MANIFEST: ";
    foreach (KoOdfManifestEntry *entry, manifest) {
        kDebug(30006) << entry->mediaType() << entry->fullPath() << entry->version();
    }
#endif

    // 1. Get the XML contents of the objects from the draw:frame.  As
    //    a side effect, this extracts the object names from all
    //    xlink:href and stores them into d->objectNames.  The saved
    //    xml contents itself is saved into d->objectXmlContents
    //    (QByteArray) so we can save it back from saveOdf().
    d->storeObjects(frameElement);

#if 1
    // Debug only
    kDebug(30006) << "----------------------------------------------------------------";
    kDebug(30006) << "After storeObjects():";
    foreach (ObjectEntry *object, d->objectEntries) {
        kDebug(30006) << "objectXmlContents: " << object->objectXmlContents
        << "objectName: " << object->objectName;
        // Note: at this point, isDir and manifestEntry are not set.
#endif
    }

    // 2. Loop through the objects that were found in the frame and
    //    save all the files associated with them.  Some of the
    //    objects are files, and some are directories.  The
    //    directories are searched and the files within are saved as
    //    well.
    //
    // In this loop, isDir and manifestEntry of each ObjectEntry are set.
    bool foundPreview = false;
    foreach (ObjectEntry *object, d->objectEntries) {
        QString objectName = object->objectName;

        if (objectName.isEmpty())
            continue;

        kDebug(30006) << "Storing files for object named:" << objectName;

        // Try to find out if the entry is a directory.
        // If the object is a directory, then save all the files
        // inside it, otherwise save the file as it is.
        QString dirName = objectName + '/';
        bool isDir = !context.odfLoadingContext().mimeTypeForPath(dirName).isEmpty();
        if (isDir) {
            // A directory: the files can be found in the manifest.
            foreach (KoOdfManifestEntry *entry, manifest) {
                if (entry->fullPath() == dirName)
                    continue;

                if (entry->fullPath().startsWith(dirName)) {
                    d->storeFile(entry->fullPath(), context);
                }
            }
        }
        else {
            // A file: save it.
            d->storeFile(objectName, context);
        }

        // Get the manifest entry for this object.
        KoOdfManifestEntry *entry = 0;
        QString entryName = isDir ? dirName : objectName;
        for (int j = 0; j < manifest.size(); ++j) {
            KoOdfManifestEntry *temp = manifest.value(j);

            if (temp->fullPath() == entryName) {
                entry = new KoOdfManifestEntry(*temp);
                break;
            }
        }
        object->isDir = isDir;
        object->manifestEntry = entry;

        // If we have not already found a preview in previous times
        // through the loop, then see if this one may be a preview.
        if (!foundPreview) {
            kDebug(30006) << "Attempting to load preview from " << objectName;
            QByteArray previewData = d->loadFile(objectName, context);
            // Check to see if we know the mimetype for this entry. Specifically:
            // 1. Check to see if the item is a loadable SVG file

            // FIXME: Perhaps check in the manifest first? But this
            //        seems to work well.
            d->scalablePreview->load(previewData);
            if (d->scalablePreview->isValid()) {
                kDebug(30006) << "Found scalable preview image!";
                d->scalablePreview->setViewBox(d->scalablePreview->boundsOnElement("svg"));
                foundPreview = true;
                continue;
            }
            // 2. Otherwise check to see if it's a loadable pixmap file
            d->pixmapPreview.loadFromData(previewData);
            if (!d->pixmapPreview.isNull()) {
                kDebug(30006) << "Found pixel based preview image!";
                foundPreview = true;
            }
        }
    }

#if 0   // Enable to get more detailed debug messages
    kDebug(30006) << "Object manifest entries:";
    for (int i = 0; i < d->manifestEntries.size(); ++i) {
        KoOdfManifestEntry *entry = d->manifestEntries.value(i);
        kDebug(30006) << i << ":" << entry;
        if (entry)
            kDebug(30006) << entry->fullPath() << entry->mediaType() << entry->version();
        else
            kDebug(30006) << "--";
    }
    kDebug(30006) << "END LOADING ####################################################";
#endif

    return true;
}


// Load the actual contents inside the frame.
bool KoUnavailShape::loadOdfFrameElement(const KoXmlElement & /*element*/,
                                         KoShapeLoadingContext &/*context*/)
{
    return true;
}


// ----------------------------------------------------------------
//                         Private functions

void KoUnavailShape::Private::storeObjects(const KoXmlElement &element)
{
    // Loop through all the child elements of the draw:frame and save them.
    KoXmlNode n = element.firstChild();
    for (; !n.isNull(); n = n.nextSibling()) {
        kDebug(30006) << "In draw:frame, node =" << n.nodeName();

        // This disregards #text, but that's not in the spec anyway so
        // it doesn't need to be saved.
        if (!n.isElement())
            continue;
        KoXmlElement el = n.toElement();

        ObjectEntry  *object = new ObjectEntry;

        QByteArray contentsTmp;
        QBuffer buffer(&contentsTmp); // the member
        KoXmlWriter writer(&buffer);

        // 1. Find out the objectName
        // Save the normalized filename, i.e. without a starting "./".
        // An empty string is saved if no name is found.
        QString  name = el.attributeNS(KoXmlNS::xlink, "href", QString());
        if (name.startsWith("./"))
            name = name.mid(2);
        object->objectName = name;

        // 2. Copy the XML code.
        QHash<QString, QString> unknownNamespaces;
        storeXmlRecursive(el, writer, object, unknownNamespaces);
        object->objectXmlContents = contentsTmp;

        // 3, 4: the isDir and manifestEntry members are not set here,
        // but initialize them anyway. .
        object->isDir = false;  // Has to be initialized to something.
        object->manifestEntry = 0;

        objectEntries.append(object);
    }
}

void KoUnavailShape::Private::storeXmlRecursive(const KoXmlElement &el, KoXmlWriter &writer,
                                                ObjectEntry *object, QHash<QString, QString> &unknownNamespaces)
{
    // Start the element;
    // keep the name in a QByteArray so that it stays valid until end element is called.
    const QByteArray name(el.nodeName().toAscii());
    writer.startElement(name.constData());

    // Copy all the attributes, including namespaces.
    QList< QPair<QString, QString> >  attributeNames = el.attributeFullNames();
    for (int i = 0; i < attributeNames.size(); ++i) {
        QPair<QString, QString> attrPair(attributeNames.value(i));
        if (attrPair.first.isEmpty()) {
            writer.addAttribute(attrPair.second.toAscii(), el.attribute(attrPair.second));
        }
        else {
            // This somewhat convoluted code is because we need the
            // namespace, not the namespace URI.
            QString nsShort = KoXmlNS::nsURI2NS(attrPair.first.toAscii());
            // in case we don't find the namespace in our list create a own one and use that
            // so the document created on saving is valid.
            if (nsShort.isEmpty()) {
                nsShort = unknownNamespaces.value(attrPair.first);
                if (nsShort.isEmpty()) {
                    nsShort = QString("ns%1").arg(unknownNamespaces.size() + 1);
                    unknownNamespaces.insert(attrPair.first, nsShort);
                }
                writer.addAttribute("xmlns:" + nsShort.toAscii(), attrPair.first);
            }
            QString attr(nsShort + ':' + attrPair.second);
            writer.addAttribute(attr.toAscii(), el.attributeNS(attrPair.first,
                                                               attrPair.second));
        }
    }

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

/**
 * This function stores the embedded file in an internal store - it does not save files to disk,
 * and thus it is named in this manner, to avoid the function being confused with functions which
 * save files to disk.
 */
void KoUnavailShape::Private::storeFile(const QString &fileName, KoShapeLoadingContext &context)
{
    kDebug(30006) << "Saving file: " << fileName;

    // Directories need to be saved too, but they don't have any file contents.
    if (fileName.endsWith('/')) {
        FileEntry *entry = new FileEntry;
        entry->path = fileName;
        entry->mimeType = context.odfLoadingContext().mimeTypeForPath(entry->path);
        entry->isDir = true;
        embeddedFiles.append(entry);
    }

    QByteArray fileContent = loadFile(fileName, context);
    if (fileContent.isNull())
        return;

    // Actually store the file in the list.
        FileEntry *entry = new FileEntry;
        entry->path = fileName;
        if (entry->path.startsWith("./"))
                    entry->path = entry->path.mid(2);
        entry->mimeType = context.odfLoadingContext().mimeTypeForPath(entry->path);
        entry->isDir = false;
        entry->contents = fileContent;
        embeddedFiles.append(entry);

        kDebug(30006) << "File length: " << fileContent.size();
}

QByteArray KoUnavailShape::Private::loadFile(const QString &fileName, KoShapeLoadingContext &context)
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

    //kDebug(30006) << "File content: " << fileContent;
    return fileContent;
}
