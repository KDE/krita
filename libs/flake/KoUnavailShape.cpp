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

// KDE
#include <kstandarddirs.h>
#include <KDebug>

// KOffice
#include "KoUnit.h"
#include "KoStore.h"
#include "KoXmlNS.h"
#include "KoXmlReader.h"
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoOdfManifest.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoEmbeddedFileSaver.h>


struct FileEntry {
    QString     path;           // Normalized filename, i.e. without "./".
    QString     mimeType;
    QByteArray  contents;
};

class KoUnavailShape::Private
{
public:
    Private();
    ~Private();

    void saveXml(const KoXmlElement &element);
    void saveXmlRecursive(const KoXmlElement &el, KoXmlWriter &writer);
    void saveFile(const QString &filename, KoShapeLoadingContext &context);

    // Objects inside the frame.  We store:
    //  - The XML code for each object
    //  - Any embedded files (names, contents) that are referenced by xlink:href
    //  - The manifest entries
    QList<QByteArray>    frameContents; // A list of the XML trees in the frame, each of them one object
    QStringList          objectNames;   // A list of objects names in the frame without "./"
                                        // These are extracted from frameContents
    QList<KoOdfManifestEntry*> manifestEntries; // A list of manifest entries for the above.

    QList<FileEntry*>    embeddedFiles; // List of embedded files.
};

KoUnavailShape::Private::Private()
{
}

KoUnavailShape::Private::~Private()
{
    qDeleteAll(manifestEntries);
}


// ----------------------------------------------------------------
//                         The main class


KoUnavailShape::KoUnavailShape()
    : KoFrameShape( "", "" )
    , d(new Private())
{
    setShapeId(KoUnavailShape_SHAPEID);

   // Default size of the shape.
    KoShape::setSize( QSizeF( CM_TO_POINT( 5 ), CM_TO_POINT( 3 ) ) );
}

KoUnavailShape::~KoUnavailShape()
{
    delete d;
}


void KoUnavailShape::paint(QPainter &painter, const KoViewConverter &converter)
{
    applyConversion(painter, converter);
    draw(painter);
}

void KoUnavailShape::paintDecorations(QPainter &painter, const KoViewConverter &converter,
                                      const KoCanvasBase *canvas)
{
    // There are no decorations for the Unavail Shape
    Q_UNUSED(painter);
    Q_UNUSED(converter);
    Q_UNUSED(canvas);
    return;
#if 0
    applyConversion(painter, converter);
    painter.setRenderHint(QPainter::Antialiasing);

    draw(painter);
#endif
}

void KoUnavailShape::draw(QPainter &painter) const
{
#if 0
    // Draw a frame and a cross.
    drawNull(painter);
#else
    // Draw a nice question mark.

    // Get the question mark "icon".
    QPixmap questionMark;
    questionMark.load(KStandardDirs::locate("data", "koffice/icons/questionmark.png"));

    // The size of the image is:
    //  - the size of the shape if  shapesize < 2cm
    //  - 2 cm                  if  2cm <= shapesize <= 8cm
    //  - shapesize / 4         if  shapesize > 8cm
    qreal  width = size().width();
    qreal  height = size().height();
    qreal  picSize = CM_TO_POINT(2); // Default size is 2 cm.
    if (width < CM_TO_POINT(2) || height < CM_TO_POINT(2))
        picSize = qMin(width, height);
    else if (width > CM_TO_POINT(8) && height > CM_TO_POINT(8))
        picSize = qMin(width, height) / qreal(4.0);

    painter.drawPixmap((width - picSize) / qreal(2.0), (height - picSize) / qreal(2.0),
                       picSize, picSize, questionMark);
#endif
}

void KoUnavailShape::drawNull(QPainter &painter) const
{
    QRectF  rect(QPointF(0,0), size());
    painter.save();

    // Draw a simple cross in a rectangle just to indicate that there is something here.
    painter.setPen(QPen(QColor(172, 196, 206)));
    painter.drawRect(rect);
    painter.drawLine(rect.topLeft(), rect.bottomRight());
    painter.drawLine(rect.bottomLeft(), rect.topRight());

    painter.restore();
}


// ----------------------------------------------------------------
//                         Loading and Saving


void KoUnavailShape::saveOdf(KoShapeSavingContext & context) const
{
    kDebug(30006) << "START SAVING ##################################################";

    KoEmbeddedFileSaver &fileSaver = context.embeddedFileSaver();
    KoXmlWriter         &writer    = context.xmlWriter();
    //KoXmlWriter         &manifestWriter = context.manifestWriter;

    writer.startElement( "draw:frame" );
    // See also loadOdf() in loadOdfAttributes.
    saveOdfAttributes( context, OdfAllAttributes );

#if 0
    kDebug(30006) << "Object names: " << d->objectNames.size();
    for (int i = 0; i < d->objectNames.size(); ++i) {
        kDebug(30006) << i << ':' << d->objectNames.value(i);
    }
    kDebug(30006) << "Object manifest entries: " << d->manifestEntries.size();
    for (int i = 0; i < d->manifestEntries.size(); ++i) {
        KoOdfManifestEntry *entry = d->manifestEntries.value(i);
        kDebug(30006) << i << ":" << entry;
        if (entry)
            kDebug(30006) << entry->fullPath() << entry->mediaType() << entry->version();
        else
            kDebug(30006) << "--";
    }
#endif

    // Write the already saved XML.
    for (int i = 0; i < d->frameContents.size(); ++i) {
        QByteArray          xmlArray(d->frameContents.value(i));
        QString             objectName(d->objectNames.value(i)); // Possibly empty.
        KoOdfManifestEntry *manifestEntry(d->manifestEntries.value(i));

        QString newName = objectName;
        if (!objectName.isEmpty()) {
            newName = fileSaver.getFilename("UObject");
            if (objectName.endsWith('/'))
                newName += '/';
            xmlArray.replace(objectName.toLatin1(), newName.toLatin1());
        }

        writer.addCompleteElement(xmlArray.data());

        // If the objectName is empty, this may be an inline object.
        // If so, we are done now.
        if (objectName.isEmpty())
            continue;

        // Save embedded files for this object.
        for (int j = 0; j < d->embeddedFiles.size(); ++j) {
            QString  fileName(d->embeddedFiles.value(j)->path);
            //kDebug(30006) << "Object name: " << objectName << "filename: " << fileName;

            if (fileName.startsWith(objectName)) {
                fileName.replace(objectName, newName);
                fileName.prepend("./");
                //kDebug(30006) << "New filename: " << fileName;
                fileSaver.saveFile(fileName, d->embeddedFiles.value(j)->mimeType.toLatin1(),
                                   d->embeddedFiles.value(j)->contents);
            }
        }

        // Write the manifest entry for the object itself.  If it's a
        // file, the manifest is already written by saveFile, so skip
        // it here.
        if (newName.endsWith('/'))
            fileSaver.saveManifestEntry(newName, manifestEntry->mediaType(),
                                        manifestEntry->version());
    }

    writer.endElement(); // draw:frame
}


bool KoUnavailShape::loadOdf(const KoXmlElement & frameElement, KoShapeLoadingContext &context)
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
#if 0
    kDebug(30006) << "MANIFEST: ";
    foreach (KoOdfManifestEntry *entry, manifest) {
        kDebug(30006) << entry->fullPath << entry->mediaType << entry->version;
    }
#endif
    // Get the XML contents from the draw:frame.  As a side effect,
    // this extracts the object names from all xlink:href and stores
    // them into d->objectNames.  The saved xml contents itself is
    // saved into d->frameContents (QByteArray) so we can save it back from
    // saveOdf().
    d->saveXml(frameElement);

    //kDebug(30006) << "frameContents: " << d->frameContents;
    //kDebug(30006) << "objectNames:   " << d->objectNames;

    // Loop through the objects that were found in the frame and save
    // all the files associated with them.  Some of the objects are
    // files, and some are directories.  The directories are searched
    // and the files within are saved as well.
    for (int i = 0; i < d->objectNames.size(); ++i) {
        QString objectName = d->objectNames.value(i);

        if (objectName.isEmpty())
            continue;

        // Try to find out if the entry is a directory.

        QString dirName = objectName + '/';

        // If the object is a directory, then save all the files
        // inside it, otherwise save the file as it is.
        bool isDir = !context.odfLoadingContext().mimeTypeForPath(dirName).isEmpty();
        if (isDir) {
            // The files can be found in the manifest.
            foreach (KoOdfManifestEntry *entry, manifest) {
                if (entry->fullPath() == dirName)
                    continue;

                if (entry->fullPath().startsWith(dirName)) {
                    d->saveFile(entry->fullPath(), context);
                }
            }
        }
        else {
            // A file: save it.
            d->saveFile(objectName, context);
        }

        // Get the manifest entry for this object.
        KoOdfManifestEntry *entry = 0;
        QString             entryName = isDir ? dirName : objectName;
        for (int j = 0; j < manifest.size(); ++j) {
            KoOdfManifestEntry *temp = manifest.value(j);

            if (temp->fullPath() == entryName) {
                entry = new KoOdfManifestEntry(*manifest.value(j));
                break;
            }
        }
        d->manifestEntries.append(entry);
        if (isDir)
            d->objectNames[i].append('/');
    }

#if 0
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

void KoUnavailShape::Private::saveXml(const KoXmlElement & element)
{
    // Loop through all the child elements of the draw:frame and save them.
    KoXmlNode n = element.firstChild();
    for (; !n.isNull(); n = n.nextSibling()) {
        QByteArray  contentsTmp;
        QBuffer     buffer(&contentsTmp); // the member
        KoXmlWriter writer(&buffer);

        if (n.isElement())
            saveXmlRecursive(n.toElement(), writer);

        frameContents.append(contentsTmp);
    }
}

void KoUnavailShape::Private::saveXmlRecursive(const KoXmlElement &el, KoXmlWriter &writer)
{
    // Start the element;
    writer.startElement(el.nodeName().toAscii());

    // Write the attributes, including namespaces.
    // FIXME: We should only handle xlink:href's on the top level.  (or?)
    QList< QPair<QString, QString> >  attributeNames = el.attributeFullNames();
    for (int i = 0; i < attributeNames.size(); ++i) {
        // This somewhat convoluted code is because we need the
        // namespace, not the namespace URI.
        QPair<QString, QString> attrPair(attributeNames.value(i));
        if (attrPair.first.isEmpty()) {
            writer.addAttribute(attrPair.second.toAscii(), el.attribute(attrPair.second));
        }
        else {
            QString attr(QString(KoXmlNS::nsURI2NS(attrPair.first.toAscii()))
                         + ':' + attrPair.second);
            writer.addAttribute(attr.toAscii(), el.attributeNS(attrPair.first,
                                                               attrPair.second));
        }
    }

    // Save the normalized filename, i.e. without a starting "./".
    // An empty string is saved if no name is found.
    QString  name = el.attributeNS(KoXmlNS::xlink, "href", QString());
    if (name.startsWith("./"))
        name = name.mid(2);
    objectNames << name;

    // Child elements
    // Loop through all the child elements of the draw:frame.
    KoXmlNode n = el.firstChild();
    for (; !n.isNull(); n = n.nextSibling()) {
        if (n.isElement()) {
            saveXmlRecursive(n.toElement(), writer);
        }
        else if (n.isText()) {
            writer.addTextNode(n.toText().data()/*.toUtf8()*/);
        }
    }

    // End the element
    writer.endElement();

#if 0
    case TextNode:
        writer.addTextNode();
        break;

    case CDATASectionNode:
    case ProcessingInstructionNode:
    case DocumentNode:
    case DocumentTypeNode:
    case NullNode:
        // FALLTHROUGH
    default:
        // Shouldn't happen
        break;
#endif
}


void KoUnavailShape::Private::saveFile(const QString &fileName, KoShapeLoadingContext &context)
{
    //kDebug(30006) << "Saving file: " << fileName;

    // For now, don't handle directories.
    if (fileName.endsWith('/'))
        return;

    KoStore    *store = context.odfLoadingContext().store();
    QByteArray  fileContent;

    if (!store->open(fileName)) {
        store->close();
        return;
    }

    int fileSize = store->size();
    fileContent = store->read(fileSize);
    store->close();

    FileEntry *entry = new FileEntry;
    entry->path = fileName;
    if (entry->path.startsWith("./"))
        entry->path = entry->path.mid(2);
    entry->mimeType = context.odfLoadingContext().mimeTypeForPath(entry->path);
    entry->contents = fileContent;
    embeddedFiles.append(entry);

    //kDebug(30006) << "File content: " << fileContent;
}
