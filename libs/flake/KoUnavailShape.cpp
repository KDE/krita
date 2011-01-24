/* This file is part of the KDE project
 *
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
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeSavingContext.h>


class KoUnavailShape::Private
{
public:
    Private();
    ~Private();

    void saveXml(const KoXmlElement & element);
    void saveXmlRecursive(const KoXmlElement &el, KoXmlWriter &writer);
    void saveFile(const QString &filename, KoShapeLoadingContext &context);

    // Objects inside the frame.  We store:
    //  - The XML code for each object
    //  - Any embedded files (names, contents) that are referenced by xlink:href
    QList<QByteArray>                  contents;      // A list of the XML trees in the frame
    QStringList                        objectNames;   // A list of objects names in the files
    QList<QPair<QString, QByteArray> > embeddedFiles; // List of <objectNames,contents> of embedded files.
    KoOdfManifestEntry                *manifestEntry; // The manifest entry for this embedded object
};

KoUnavailShape::Private::Private()
{
}

KoUnavailShape::Private::~Private()
{
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
    kDebug(30006) << "Drawrect = " << rect;
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
    KoXmlWriter&  writer = context.xmlWriter();

    writer.startElement( "draw:frame" );
    // See also loadOdf() in loadOdfAttributes.
    saveOdfAttributes( context, OdfAllAttributes );

    // Write the already saved XML
    // FIXME: Add hrefs and stuff
    for (int i = 0; i < d->contents.size(); ++i) {
        QByteArray  xmlArray(d->contents.value(i));
        QString     objectName(d->objectNames.value(i)); // Possibly empty.

        if (!(objectName.isEmpty())) {
            // FIXME: Create new object name and fix the string
        }

        writer.addCompleteElement(xmlArray.data());

        if (objectName.isEmpty())
            continue;

        // Remove the prefix ./ (as in ./Object1) from object names.
        if (objectName.startsWith("./"))
            objectName = objectName.mid(2);

        // Save embedded files for this object.
        for (int j = 0; j < d->embeddedFiles.size(); ++j) {
            QString  fileName(d->embeddedFiles.value(i).first);
            kDebug(30006) << "Object name: " << objectName << "filename: " << fileName;

            if (fileName.startsWith(objectName)) {
                //manifestWriter.addManifestEntry(fileName, d->manifestEntry->mediaType, d->manifestEntry->version); // TODO: version
                // FIXME: Add the file here.
            }
        }
    }

    writer.endElement(); // draw:frame
}


bool KoUnavailShape::loadOdf(const KoXmlElement & frameElement, KoShapeLoadingContext &context)
{
    kDebug(30006) << "START LOADING ##################################################";
    kDebug(30006) << "Loading ODF frame in the KoUnavailShape. Element = "
                  << frameElement.tagName();

    loadOdfAttributes(frameElement, context, OdfAllAttributes);

    // NOTE: We cannot use loadOdfFrame() because we want to save all
    //       the things inside the frame, not just one of them, like
    //       loadOdfFrame() provides.

    // Get the XML contents from the draw:frame.  As a side effect,
    // this extracts the object names from all xlink:href and stores
    // them into d->objectNames.  The saved xml contents itself is
    // saved into d->contents (QByteArray) so we can save it back from
    // saveOdf().
    d->saveXml(frameElement);

    kDebug(30006) << "Contents: " << d->contents;
    kDebug(30006) << "objectNames: " << d->objectNames;

    kDebug(30006) << "MANIFEST: ";
    QList<KoOdfManifestEntry*> manifestEntries = context.odfLoadingContext().manifestEntries();
    // Loop through the objects that were found in the frame and save
    // all the files associated with them.  Some of the objects are
    // files, and some are directories.  The directories are searched
    // and the files within are saved as well.
    for (int i = 0; i < d->objectNames.size(); ++i) {
        QString objectName = d->objectNames.value(i);

        if (objectName.isEmpty())
            continue;

        // Try to find out if the entry is a directory.

        // Remove the prefix ./ (as in ./Object1) from object names
        // because the filenames in the manifest never contains this.
        if (objectName.startsWith("./"))
            objectName = objectName.mid(2);

        QString dirName = objectName + '/';

        // If the object is a directory, then save all the files
        // inside it, otherwise save the file as it is.
        if (!context.odfLoadingContext().mimeTypeForPath(dirName).isEmpty()) {
            // The files can be found in the manifest.
            foreach (KoOdfManifestEntry *entry, manifestEntries) {
                if (entry->fullPath.startsWith(dirName)) {
                    d->saveFile(entry->fullPath, context);
                }
            }
        }
        else {
            // A file: save it.
            d->saveFile(objectName, context);
        }
    }

    kDebug(30006) << "END LOADING ####################################################";
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

        contents.append(contentsTmp);
    }
}

void KoUnavailShape::Private::saveXmlRecursive(const KoXmlElement &el, KoXmlWriter &writer)
{
    // Start the element;
    writer.startElement(el.nodeName().toAscii());

    // Write the attributes, including namespaces.
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

    // Save filenames. An empty string is saved if none is found.
    objectNames << el.attributeNS(KoXmlNS::xlink, "href", QString());

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
    kDebug(30006) << "Saving file: " << fileName;

    // For now, don't handle directories.
    if (fileName.endsWith('/'))
        return;

    KoStore *store = context.odfLoadingContext().store();
    QByteArray fileContent;

    if (!store->open(fileName)) {
        store->close();
        return;
    }

    int fileSize = store->size();
    fileContent = store->read(fileSize);
    store->close();

    kDebug(30006) << "File content: " << fileContent;
    embeddedFiles.append(QPair<QString, QByteArray>(fileName, fileContent));
}
