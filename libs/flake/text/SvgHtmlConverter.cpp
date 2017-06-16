/* This file is part of the KDE project

   Copyright (C) 2012 Mojtaba Shahi Senobari <mojtaba.shahi3000@gmail.com>
   Copyright (C) 2012 Inge Wallin            <inge@lysator.liu.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/


// Own
#include "OdtHtmlConverter.h"

// Qt
#include <QStringList>
#include <QBuffer>

// KF5
#include <klocalizedstring.h>

// Calligra
#include <KoStore.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoUnit.h>

// EPUB filter
#include "FileCollector.h"
#include "SharedExportDebug.h"

// ================================================================
//                         Style parsing


StyleInfo::StyleInfo()
    : isDefaultStyle(false)
    , defaultOutlineLevel(-1)
    , shouldBreakChapter(false)
    , inUse(false)
{
}


OdtHtmlConverter::OdtHtmlConverter()
    : m_currentChapter(1)
    , m_mediaId(1)
{
    qDeleteAll(m_styles);
}

OdtHtmlConverter::~OdtHtmlConverter()

{
}

// ================================================================
//                         HTML conversion


const OdtHtmlConverter::ConversionOptions defaultOptions = {
    true,                       // Put styles into styles.css
    true,                        // Do break the output into chapters
    false                       // It doesn't use Mobi convention
};


KoFilter::ConversionStatus
OdtHtmlConverter::convertContent(KoStore *odfStore,
                                 QHash<QString, QString> &metaData,
                                 QHash<QString, QString> *manifest,
                                 OdtHtmlConverter::ConversionOptions *options,
                                 FileCollector *collector,
                                 // Out parameters:
                                 QHash<QString, QSizeF> &images, QHash<QString, QString> &mediaFiles)
{
    if (options)
        m_options = options;
    else
        m_options = &defaultOptions;
    m_collector = collector;
    m_manifest = manifest;
    m_odfStore = odfStore;

    m_doIndent = !m_options->useMobiConventions;
    m_imgIndex = 1;

    // 1. Parse styles

    KoFilter::ConversionStatus  status = collectStyles(odfStore, m_styles);
    if (status != KoFilter::OK) {
        return status;
    }

#if 0 // Debug
    debugSharedExport << "======== >> Styles";
    foreach(const QString &name, m_styles.keys()) {
        debugSharedExport << "==" << name << ":\t"
                      << m_styles.value(name)->parent
                      << m_styles.value(name)->family
                      << m_styles.value(name)->isDefaultStyle
                      << m_styles.value(name)->shouldBreakChapter
                      << m_styles.value(name)->attributes
            ;
    }
    debugSharedExport << "======== << Styles";
#endif

    // Propagate style inheritance.
    fixStyleTree(m_styles);

    // 2. Create CSS contents and store it in the file collector.
    status = createCSS(m_styles, m_cssContent);
    //debugSharedExport << "Styles:" << m_styles;
    //debugSharedExport << "CSS:" << m_cssContent;
    if (status != KoFilter::OK) {
        delete odfStore;
        return status;
    }
    if (m_options->stylesInCssFile) {
        m_collector->addContentFile("stylesheet",
                                    m_collector->pathPrefix() + "styles.css",
                                    "text/css", m_cssContent);
    }

    // ----------------------------------------------------------------
    // Parse body from content.xml

    if (!odfStore->open("content.xml")) {
        debugSharedExport << "Can not open content.xml .";
        return KoFilter::FileNotFound;
    }

    KoXmlDocument doc;
    QString errorMsg;
    int errorLine;
    int errorColumn;
    if (!doc.setContent(odfStore->device(), true, &errorMsg, &errorLine, &errorColumn)) {
        debugSharedExport << "Error occurred while parsing content.xml "
                      << errorMsg << " in Line: " << errorLine
                      << " Column: " << errorColumn;
        odfStore->close();
        return KoFilter::ParsingError;
    }

    KoXmlNode currentNode = doc.documentElement();
    KoXmlElement nodeElement;    // currentNode as Element

    currentNode = KoXml::namedItemNS(currentNode, KoXmlNS::office, "body");
    currentNode = KoXml::namedItemNS(currentNode, KoXmlNS::office, "text");

    // 3. Collect information about internal links.
    KoXmlElement element = currentNode.toElement(); // node for passing it to collectInter...()
    int chapter = 1; // Only necessary for the recursion.
    collectInternalLinksInfo(element, chapter);

    // 4. Start the actual conversion.

    // Write the beginning of the output.
    beginHtmlFile(metaData);

    QString currentChapterTitle;

    m_currentChapter = 1;       // Number of current output chapter.
    forEachElement (nodeElement, currentNode) {

        // text:h and text:p are treated special since they can have
        // styling that makes us start on a new html file,
        // a.k.a. chapter.
        if (nodeElement.namespaceURI() == KoXmlNS::text && (nodeElement.localName() == "p"
                                                            || nodeElement.localName() == "h")) {

            // Check if this paragraph should break the text into a new chapter. 
            //
            // This should happen either if the paragraph has
            // outline-level = 1 or if the style indicates that the
            // break should happen. The styles come into this function
            // preprocessed.
            //
            // Only create a new chapter if it is a top-level
            // paragraph and not at the very first node.
            //
            StyleInfo *style = m_styles.value(nodeElement.attribute("style-name"));
            bool  hasOutlineLevel1 = (nodeElement.attribute("outline-level") == "1"
                                      || (nodeElement.attribute("outline-level").isEmpty()
                                          && style && style->defaultOutlineLevel == 1));
            if (m_options->doBreakIntoChapters
                && (hasOutlineLevel1 || (style && style->shouldBreakChapter)))
            {
                //debugSharedExport << "Found paragraph which breaks into new chapter";

                // Write out any footnotes
                if (!m_footNotes.isEmpty()) {
                    writeFootNotes(m_htmlWriter);
                }

                // And finally close all tags.
                endHtmlFile(); 

                // Write the result to the file collector object.
                QString fileId = m_collector->filePrefix() + QString::number(m_currentChapter);
                QString fileName = m_collector->pathPrefix() + fileId + m_collector->fileSuffix();
                m_collector->addContentFile(fileId, fileName,
                                            "application/xhtml+xml", m_htmlContent, currentChapterTitle);


                if (nodeElement.localName() == "h") {
                    currentChapterTitle = nodeElement.text();
                } else {
                    currentChapterTitle.clear();
                }

                // And begin a new chapter.
                beginHtmlFile(metaData);
                m_currentChapter++;
            }

            // Actually handle the contents.
            if (nodeElement.localName() == "p")
                handleTagP(nodeElement, m_htmlWriter);
            else
                handleTagH(nodeElement, m_htmlWriter);
        }
        else if (nodeElement.localName() == "span" && nodeElement.namespaceURI() == KoXmlNS::text) {
            handleTagSpan(nodeElement, m_htmlWriter);
        }
        else if (nodeElement.localName() == "table" && nodeElement.namespaceURI() == KoXmlNS::table) {
            // Handle table
            handleTagTable(nodeElement, m_htmlWriter);
        }
        else if (nodeElement.localName() == "frame" && nodeElement.namespaceURI() == KoXmlNS::draw)  {
            // Handle frame
            m_htmlWriter->startElement("div", m_doIndent);
            handleTagFrame(nodeElement, m_htmlWriter);
            m_htmlWriter->endElement(); // end div
        }
        else if (nodeElement.localName() == "soft-page-break" &&
                 nodeElement.namespaceURI() == KoXmlNS::text) {

            handleTagPageBreak(nodeElement, m_htmlWriter);
        }
        else if (nodeElement.localName() == "list" && nodeElement.namespaceURI() == KoXmlNS::text) {
            handleTagList(nodeElement, m_htmlWriter);
        }
        else if (nodeElement.localName() == "a" && nodeElement.namespaceURI() == KoXmlNS::text) {
            handleTagA(nodeElement, m_htmlWriter);
        }
        else if (nodeElement.localName() == "table-of-content" &&
                 nodeElement.namespaceURI() == KoXmlNS::text) {

            handleTagTableOfContent(nodeElement, m_htmlWriter);
        }
        else if (nodeElement.localName() == "line-break" && nodeElement.namespaceURI() == KoXmlNS::text) {
            handleTagLineBreak(m_htmlWriter);
        }
        else {
            m_htmlWriter->startElement("div", m_doIndent);
            handleUnknownTags(nodeElement, m_htmlWriter);
            m_htmlWriter->endElement();
        }
    }

    // Write out any footnotes
    if (!m_footNotes.isEmpty()) {
        writeFootNotes(m_htmlWriter);
    }

    endHtmlFile();

    // Write output of the last file to the file collector object.
    QString fileId = m_collector->filePrefix();
    if (m_options->doBreakIntoChapters)
        fileId += QString::number(m_currentChapter);
    QString fileName = m_collector->pathPrefix() + fileId + m_collector->fileSuffix();
    m_collector->addContentFile(fileId, fileName, "application/xhtml+xml", m_htmlContent, currentChapterTitle);

    // 5. Write any data that we have collected on the way.

    // If we had end notes, make a new chapter for end notes
    if (!m_endNotes.isEmpty()) {

        // Write the beginning of the output for the next file.
        beginHtmlFile(metaData);
        writeEndNotes(m_htmlWriter);
        endHtmlFile();

        QString fileId = "chapter-endnotes";
        QString fileName = m_collector->pathPrefix() + fileId + m_collector->fileSuffix();
        m_collector->addContentFile(fileId, fileName, "application/xhtml+xml", m_htmlContent, i18n("End notes"));
    }

    // Write media document file.
    if (!m_mediaFilesList.isEmpty()) {
        writeMediaOverlayDocumentFile();
    }
    odfStore->close();

    // Return the list of images.
    images = m_images;
    // Return the list of media files source.
    mediaFiles = m_mediaFilesList;

    return KoFilter::OK;
}

void OdtHtmlConverter::beginHtmlFile(QHash<QString, QString> &metaData)
{
    m_htmlContent.clear();
    m_outBuf = new QBuffer(&m_htmlContent);
    m_htmlWriter = new KoXmlWriter(m_outBuf);

    m_htmlWriter->startElement("html", m_doIndent);
    if (!m_options->useMobiConventions)
        m_htmlWriter->addAttribute("xmlns", "http://www.w3.org/1999/xhtml");
    createHtmlHead(m_htmlWriter, metaData);
    m_htmlWriter->startElement("body", m_doIndent);

    // NOTE: At this point we have two open tags: <html> and <body>.
}

void OdtHtmlConverter::endHtmlFile()
{
    // NOTE: At this point we have two open tags: <html> and <body>.

    // Close the two tags opened by beginHtmlFile().
    m_htmlWriter->endElement();
    m_htmlWriter->endElement();

    // Prepare for the next file.
    delete m_htmlWriter;
    delete m_outBuf;
}



void OdtHtmlConverter::createHtmlHead(KoXmlWriter *writer, QHash<QString, QString> &metaData)
{
    writer->startElement("head", m_doIndent);

    // We don't have title and meta tags in Mobi.
    if (!m_options->useMobiConventions) {
        writer->startElement("title", m_doIndent);
        writer->addTextNode(metaData.value("title"));
        writer->endElement(); // title

        writer->startElement("meta", m_doIndent);
        writer->addAttribute("http-equiv", "Content-Type");
        writer->addAttribute("content", "text/html; charset=utf-8");
        writer->endElement(); // meta

        // write meta tag
        // m-meta <Tagname, Text>
        // <meta name = "Tagname" content = "Text" />
        foreach (const QString &name, metaData.keys()) {
            // Title is handled above.
            if (name == "title")
                continue;

            writer->startElement("meta", m_doIndent);
            writer->addAttribute("name", name);
            writer->addAttribute("content", metaData.value(name));
            writer->endElement(); // meta
        }
    }

    // Refer to the stylesheet or put the styles in the html file.
    if (m_options->stylesInCssFile) {
        writer->startElement("link", m_doIndent);
        writer->addAttribute("href", "styles.css");
        writer->addAttribute("type", "text/css");
        writer->addAttribute("rel", "stylesheet");
        writer->endElement(); // link
    }
    else {
        writer->startElement("style", m_doIndent);
        writer->addTextNode(m_cssContent);
        writer->endElement(); // style
    }

    writer->endElement(); // head
}


// ----------------------------------------------------------------
//                 Traversal of the XML contents


void OdtHtmlConverter::handleTagTable(KoXmlElement &nodeElement, KoXmlWriter *htmlWriter)
{
    QString styleName = cssClassName(nodeElement.attribute("style-name"));
    StyleInfo *styleInfo = m_styles.value(styleName);
    htmlWriter->startElement("table", m_doIndent);
    if (styleInfo) {
        styleInfo->inUse = true;
        htmlWriter->addAttribute("class", styleName);
    }
    htmlWriter->addAttribute("style", "border-collapse: collapse");

    KoXmlElement tableElement;
    forEachElement (tableElement, nodeElement) {

        //Table headers
        if (tableElement.localName() == "table-header-rows" && tableElement.namespaceURI() == KoXmlNS::table) {
            htmlWriter->startElement("thead", m_doIndent);

            KoXmlElement headerRow;
            forEachElement (headerRow, tableElement) {
                handleTagTableRow(headerRow, htmlWriter, TableHeaderType);
            }

            htmlWriter->endElement(); //thead
        }

        //Table body
        if (tableElement.localName() == "table-rows" && tableElement.namespaceURI() == KoXmlNS::table) {
            htmlWriter->startElement("tbody", m_doIndent);

            KoXmlElement rowElement;
            forEachElement (rowElement, tableElement) {
                handleTagTableRow(rowElement, htmlWriter);
            }

            htmlWriter->endElement(); //tbody
        }

        //Tables without headers have no table-rows element and instead embed rows directly in the table,
        //so handle that properly.
        if (tableElement.localName() == "table-row" && tableElement.namespaceURI() == KoXmlNS::table) {
            handleTagTableRow(tableElement, htmlWriter);
        }
    }

    htmlWriter->endElement(); //table
}

void OdtHtmlConverter::handleTagTableRow(KoXmlElement& nodeElement, KoXmlWriter* htmlWriter, OdtHtmlConverter::TableCellType type)
{
    htmlWriter->startElement("tr", m_doIndent);

    KoXmlElement cellElement;
    forEachElement (cellElement, nodeElement) {

        if (cellElement.localName() == "covered-table-cell") {
            continue;
        }

        htmlWriter->startElement(type == TableHeaderType ? "th" : "td", m_doIndent);

        if (cellElement.hasAttributeNS(KoXmlNS::table, "style-name")) {
            QString styleName = cssClassName(cellElement.attribute("style-name"));
            StyleInfo *styleInfo = m_styles.value(styleName);
            if(styleInfo) {
                styleInfo->inUse = true;
                htmlWriter->addAttribute("class", styleName);
            }
        }

        if (cellElement.hasAttributeNS(KoXmlNS::table, "number-rows-spanned")) {
            htmlWriter->addAttribute("rowspan", cellElement.attribute("number-rows-spanned"));
        }

        if (cellElement.hasAttributeNS(KoXmlNS::table, "number-columns-spanned")) {
            htmlWriter->addAttribute("colspan", cellElement.attribute("number-columns-spanned"));
        }

        // ==== cell text ====
        // FIXME: This is wrong. A cell element can contain
        //        the same tags as the full contents, not just
        //        what is inside a paragraph. (Beside, this
        //        function has a strange name.)
        handleInsideElementsTag(cellElement, htmlWriter);

        htmlWriter->endElement(); //td
    }

    htmlWriter->endElement(); //tr
}

void OdtHtmlConverter::handleTagFrame(KoXmlElement &nodeElement, KoXmlWriter *htmlWriter)
{
    QString styleName = cssClassName(nodeElement.attribute("style-name"));
    StyleInfo *styleInfo = m_styles.value(styleName);

    // Find height and width
    QString height = nodeElement.attribute("height");
    QString width  = nodeElement.attribute("width");

    // Remove characters "in" or "pt" from their end.
    //
    // FIXME: This is WRONG!  
    //        First, there is no way to tell if the unit is 2 chars
    //        Second, it is not sure that there *is* a unit.
    //        Instead, use some function in KoUnit that converts the size.  /IW
    height = height.left(height.length()-2);
    width  = width.left(width.length()-2);

    // Convert them to real.
    qreal qHeight = height.toFloat();
    qreal qWidth = width.toFloat();
    QSizeF size(qWidth, qHeight);

    // Go through the frame's content and see what we can handle.
    KoXmlElement framePartElement;
    forEachElement (framePartElement, nodeElement) {

        // Handle at least a few types of objects (hopefully more in the future).
        if (framePartElement.localName() == "object"
            && framePartElement.namespaceURI() == KoXmlNS::draw)
        {
            QString href = framePartElement.attribute("href");
            if (href.isEmpty()) {
                // Check for inline stuff.
                // So far only math:math is supported.
                if (!framePartElement.hasChildNodes())
                    continue;

                // Handle inline math:math
                KoXmlElement childElement = framePartElement.firstChildElement();
                if (childElement.localName() == "math"
                    && childElement.namespaceURI() == KoXmlNS::math)
                {
                    QHash<QString, QString> unknownNamespaces;
                    copyXmlElement(childElement, *htmlWriter, unknownNamespaces);

                    // We are done with the whole frame.
                    break;
                }

                // We couldn't handle this inline object. Check for
                // object replacements (pictures).
                continue;
            }

            // If we get here, this frame part was not an inline object.
            // We already have an object reference.

            // Normalize the object reference
            if (href.startsWith(QLatin1String("./")))
                href.remove(0, 2);
            QString type = m_manifest->value(href);

            // So far we can only an handle embedded object (formula).
            // In the future we will probably be able to handle more types.
            if (type == "application/vnd.oasis.opendocument.formula") {

                handleEmbeddedFormula(href, htmlWriter);
                break; // Only one object per frame.
            }
            // ...more types here in the future, e.g. video.

            // Ok, so we couldn't handle this one.
            continue;
        }
        else if (framePartElement.localName() == "image"
                 && framePartElement.namespaceURI() == KoXmlNS::draw)
        {
            // Handle image
            htmlWriter->startElement("img", m_doIndent);
            if (styleInfo) {
                styleInfo->inUse = true;
                htmlWriter->addAttribute("class", styleName);
            }
            htmlWriter->addAttribute("alt", "(No Description)");

            QString href = framePartElement.attribute("href");
            QString imgSrc = href.section('/', -1);
            //debugSharedExport << "image source:" << href << imgSrc;

            if (m_options->useMobiConventions) {
                // Mobi
                // First check for repeated images.
                if (m_imagesIndex.contains(imgSrc)) {
                    htmlWriter->addAttribute("recindex", QString::number(m_imagesIndex.value(imgSrc)));
                }
                else {
                    htmlWriter->addAttribute("recindex", QString::number(m_imgIndex));
                    m_imagesIndex.insert(imgSrc, m_imgIndex);
                    m_imgIndex++;
                }
            }
            else {
                htmlWriter->addAttribute("src", imgSrc);
            }

            m_images.insert(framePartElement.attribute("href"), size);

            htmlWriter->endElement(); // end img
            break; // Only one image per frame.
        }
        // Handle video
        else if (framePartElement.localName() == "plugin"
                 && framePartElement.namespaceURI() == KoXmlNS::draw) {
            QString videoSource = framePartElement.attribute("href");
            QString videoId = "media_id_" + QString::number(m_mediaId);
            m_mediaId++;

            htmlWriter->addAttribute("id", videoId);
            QString id = "chapter" + QString::number(m_currentChapter) +
                    m_collector->fileSuffix() + "#" + videoId;
            m_mediaFilesList.insert(id, videoSource);
        }
    } // foreach
}

void OdtHtmlConverter::handleEmbeddedFormula(const QString &href, KoXmlWriter *htmlWriter)
{
    // FIXME: Track down why we need to close() the store here and
    //        whip that code with a wet noodle.
    m_odfStore->close();

    // Open the formula content file if possible.
    if (!m_odfStore->open(href + "/content.xml")) {
        debugSharedExport << "Can not open" << href << "/content.xml .";
        return;
    }

    // Copy the math:math xml tree.
    KoXmlDocument doc;
    QString errorMsg;
    int errorLine;
    int errorColumn;
    if (!doc.setContent(m_odfStore->device(), true, &errorMsg, &errorLine, &errorColumn)) {
        debugSharedExport << "Error occurred while parsing content.xml "
                      << errorMsg << " in Line: " << errorLine
                      << " Column: " << errorColumn;
        m_odfStore->close();
        return;
    }

    KoXmlNode n = doc.documentElement();
    for (; !n.isNull(); n = n.nextSibling()) {
        if (n.isElement()) {
            KoXmlElement el = n.toElement();
            if (el.tagName() == "math") {
                QHash<QString, QString> unknownNamespaces;
                copyXmlElement(el, *htmlWriter, unknownNamespaces);

                // No need to continue once we have the math:math node.
                break;
            }
        }
    }

    m_odfStore->close();
}

// Note: This code was copied from libs/flake/KoUnavailShape.  It
// should probably be placed near /libs/odf/KoXml* instead.

void OdtHtmlConverter::copyXmlElement(const KoXmlElement &el, KoXmlWriter &writer,
                                      QHash<QString, QString> &unknownNamespaces)
{
    // Start the element;
    // keep the name in a QByteArray so that it stays valid until end element is called.
    const QByteArray name(el.nodeName().toLatin1());
    debugSharedExport << "Copying element;" << name;
    writer.startElement(name.constData());

    // Copy all the attributes, including namespaces.
    const QList< QPair<QString, QString> >  &attributeNames = el.attributeFullNames();
    for (int i = 0; i < attributeNames.size(); ++i) {
        const QPair<QString, QString>  &attrPair(attributeNames.value(i));
        if (attrPair.first.isEmpty()) {
            debugSharedExport << "Copying attribute;" << attrPair.second;
            writer.addAttribute(attrPair.second.toLatin1(), el.attribute(attrPair.second));
        }
        else {
            // This somewhat convoluted code is because we need the
            // namespace, not the namespace URI.
            QString nsShort = KoXmlNS::nsURI2NS(attrPair.first.toLatin1());
            // in case we don't find the namespace in our list create a own one and use that
            // so the document created on saving is valid.
            if (nsShort.isEmpty()) {
                nsShort = unknownNamespaces.value(attrPair.first);
                if (nsShort.isEmpty()) {
                    nsShort = QString("ns%1").arg(unknownNamespaces.size() + 1);
                    unknownNamespaces.insert(attrPair.first, nsShort);
                }
                QString s = QString("xmlns:") + nsShort.toLatin1();
                writer.addAttribute(s.toLatin1(), attrPair.first);
            }
            QString attr(nsShort + ':' + attrPair.second);
            writer.addAttribute(attr.toLatin1(), el.attributeNS(attrPair.first,
                                                               attrPair.second));
        }
    }

    // Child elements
    // Loop through all the child elements of the draw:frame.
    KoXmlNode n = el.firstChild();
    for (; !n.isNull(); n = n.nextSibling()) {
        if (n.isElement()) {
            copyXmlElement(n.toElement(), writer, unknownNamespaces);
        }
        else if (n.isText()) {
            writer.addTextNode(n.toText().data()/*.toUtf8()*/);
        }
    }

    // End the element
    writer.endElement();
}

// ----------------------------------------------------------------

void OdtHtmlConverter::handleTagP(KoXmlElement &nodeElement, KoXmlWriter *htmlWriter)
{
    QString styleName = cssClassName(nodeElement.attribute("style-name"));
    StyleInfo *styleInfo = m_styles.value(styleName);
    htmlWriter->startElement("p", m_doIndent);
    if (styleInfo) {
        styleInfo->inUse = true;
        htmlWriter->addAttribute("class", styleName);
    }
    handleInsideElementsTag(nodeElement, htmlWriter);
    htmlWriter->endElement();
}

void OdtHtmlConverter::handleCharacterData(KoXmlNode &node, KoXmlWriter *htmlWriter)
{
    KoXmlText charData = node.toText();
    htmlWriter->addTextNode(charData.data());
}

void OdtHtmlConverter::handleTagSpan(KoXmlElement &nodeElement, KoXmlWriter *htmlWriter)
{
    QString styleName = cssClassName(nodeElement.attribute("style-name"));
    StyleInfo *styleInfo = m_styles.value(styleName);
    htmlWriter->startElement("span", m_doIndent);
    if (styleInfo) {
        styleInfo->inUse = true;
        htmlWriter->addAttribute("class", styleName);
    }
    handleInsideElementsTag(nodeElement, htmlWriter);
    htmlWriter->endElement(); // span
}

void OdtHtmlConverter::handleTagPageBreak(KoXmlElement &nodeElement, KoXmlWriter *htmlWriter)
{
    htmlWriter->addTextNode(nodeElement.text().toUtf8());
}

void OdtHtmlConverter::handleTagH(KoXmlElement &nodeElement, KoXmlWriter *htmlWriter)
{
    QString styleName = cssClassName(nodeElement.attribute("style-name"));
    StyleInfo *styleInfo = m_styles.value(styleName);
    htmlWriter->startElement("h1", m_doIndent);
    if (styleInfo) {
        styleInfo->inUse = true;
        htmlWriter->addAttribute("class", styleName);
    }
    handleInsideElementsTag(nodeElement, htmlWriter);
    htmlWriter->endElement();
}

void OdtHtmlConverter::handleTagList(KoXmlElement &nodeElement, KoXmlWriter *htmlWriter)
{
    QString styleName = cssClassName(nodeElement.attribute("style-name"));
    StyleInfo *styleInfo = m_styles.value(styleName);
    htmlWriter->startElement("ul", m_doIndent);
    if (styleInfo) {
        styleInfo->inUse = true;
        htmlWriter->addAttribute("class", styleName);
    }

    KoXmlElement listItem;
    forEachElement (listItem, nodeElement) {
        htmlWriter->startElement("li", m_doIndent);
        handleInsideElementsTag(listItem, htmlWriter);
        htmlWriter->endElement();
    }
    htmlWriter->endElement();
}

void OdtHtmlConverter::handleTagA(KoXmlElement &nodeElement, KoXmlWriter *htmlWriter)
{
    htmlWriter->startElement("a", m_doIndent);
    QString reference = nodeElement.attribute("href");
    QString chapter = m_linksInfo.value(reference);
    if (!chapter.isEmpty() && !m_options->stylesInCssFile) {
        // This is internal link.
        reference.remove('|');
        reference.remove(' ');// remove spaces
        reference = chapter+reference;
        htmlWriter->addAttribute("href", reference);
    }
    else {
        // This is external link.
        htmlWriter->addAttribute("href", reference);
    }

    handleInsideElementsTag(nodeElement, htmlWriter);
    htmlWriter->endElement();
}

void OdtHtmlConverter::handleTagTab (KoXmlWriter *htmlWriter)
{
    for (int i = 0; i <10; ++i)
        htmlWriter->addTextNode("\u00a0");
}

void OdtHtmlConverter::handleTagTableOfContent(KoXmlElement &nodeElement, KoXmlWriter *htmlWriter)
{
    KoXmlNode indexBody = KoXml::namedItemNS(nodeElement, KoXmlNS::text, "index-body");
    KoXmlElement  element;
    forEachElement (element, indexBody) {
        if (element.localName() == "index-title" && element.namespaceURI() == KoXmlNS::text) {
            handleInsideElementsTag(element, htmlWriter);
        }
        else
            handleTagTableOfContentBody(element, htmlWriter);
    }
}

void OdtHtmlConverter::handleTagTableOfContentBody(KoXmlElement &nodeElement,
                                                   KoXmlWriter *htmlWriter)
{
    if (nodeElement.localName() == "p" && nodeElement.namespaceURI() == KoXmlNS::text) {
        handleTagP(nodeElement, htmlWriter);
    }
}

void OdtHtmlConverter::handleTagLineBreak(KoXmlWriter *htmlWriter)
{
    htmlWriter->startElement("br", m_doIndent);
    htmlWriter->endElement();
}

void OdtHtmlConverter::handleTagBookMark(KoXmlElement &nodeElement, KoXmlWriter *htmlWriter)
{
    QString anchor = nodeElement.attribute("name");
    // This is haed codevalidator gets error for characters "|" and spaces
    // FIXME : we should handle ids better after move file to class
    anchor.remove('|');
    anchor.remove(' ');//remove spaces
    htmlWriter->startElement("a", m_doIndent);
    htmlWriter->addAttribute("id", anchor);
}

void OdtHtmlConverter::handleTagBookMarkStart(KoXmlElement &nodeElement, KoXmlWriter *htmlWriter)
{
    QString anchor = nodeElement.attribute("name");
    htmlWriter->startElement("a", m_doIndent);
    htmlWriter->addAttribute("id", anchor);
}

void OdtHtmlConverter::handleTagBookMarkEnd(KoXmlWriter *htmlWriter)
{
    htmlWriter->endElement();

}

void OdtHtmlConverter::handleUnknownTags(KoXmlElement &nodeElement, KoXmlWriter *htmlWriter)
{
    // Just go deeper to find known tags.
    handleInsideElementsTag(nodeElement, htmlWriter);
}


void OdtHtmlConverter::handleTagNote(KoXmlElement &nodeElement, KoXmlWriter *htmlWriter)
{
    QString noteClass = nodeElement.attribute("note-class");
    if (noteClass != "footnote" && noteClass != "endnote") {
        return;
    }

    QString id = nodeElement.attribute("id");
    KoXmlElement noteElements;
    forEachElement(noteElements, nodeElement) {
        if (noteElements.localName() == "note-citation" && noteElements.namespaceURI() == KoXmlNS::text) {
            htmlWriter->startElement("sup", m_doIndent);

            htmlWriter->startElement("a", m_doIndent);
            if (noteClass == "footnote")
                htmlWriter->addAttribute("href", "#" + id + "n"); // n rerence to note foot-note or end-note
            else { // endnote
                QString endRef = "chapter-endnotes" + m_collector->fileSuffix() + '#' + id + 'n';
                htmlWriter->addAttribute("href", endRef);
            }
            htmlWriter->addAttribute("id", id + "t"); // t is for text
            htmlWriter->addTextNode(noteElements.text());
            htmlWriter->endElement();

            htmlWriter->endElement();
        }
        else if (noteElements.localName() == "note-body" && noteElements.namespaceURI() == KoXmlNS::text) {
            if (noteClass == "footnote")
                m_footNotes.insert(id, noteElements);
            else {
                QString noteChapter = m_collector->filePrefix();
                if (m_options->doBreakIntoChapters)
                    noteChapter += QString::number(m_currentChapter);
                m_endNotes.insert(noteChapter + "/" + id, noteElements);
                // we insert this: m_currentChapter/id
                // to can add reference for text in end note
            }
        }
    }
}

void OdtHtmlConverter::handleInsideElementsTag(KoXmlElement &nodeElement, KoXmlWriter *htmlWriter)
{
    KoXmlNode node = nodeElement.firstChild();
    KoXmlElement element = node.toElement();

    // handle it with tag bookmark-end but for bookmark we may have
    // bookmark-end or we may not have it so we control it with "insideBookmarkTag".
    bool insideBookmarkTag = false;

    // We have characterData or image or span or s  or soft-page break in a tag p
    // FIXME: we should add if there are more tags.
    while (!node.isNull()) {

        if (node.isText()) {
            handleCharacterData(node, htmlWriter);
            if (insideBookmarkTag) {
                // End tag <a> started in bookmark or bookmark-start.
                htmlWriter->endElement(); // end tag "a"
                insideBookmarkTag = false;
            }
        }
        else if (element.localName() == "p" && element.namespaceURI() == KoXmlNS::text) {
            handleTagP(element, htmlWriter);
        }
        else if (element.localName() == "h" && element.namespaceURI() == KoXmlNS::text) {
            handleTagH(element, htmlWriter);
        }
        else if (element.localName() == "table" && element.namespaceURI() == KoXmlNS::table) {
            handleTagTable(element, htmlWriter);
        }
        else if (element.localName() == "span" && element.namespaceURI() == KoXmlNS::text) {
            handleTagSpan(element, htmlWriter);
        }
        else if (element.localName() == "frame" && element.namespaceURI() == KoXmlNS::draw) {
            handleTagFrame(element, htmlWriter);
        }
        else if (nodeElement.localName() == "list" && nodeElement.namespaceURI() == KoXmlNS::text) {
            handleTagList(nodeElement, htmlWriter);
        }
        else if (element.localName() == "soft-page-break" && element.namespaceURI() == KoXmlNS::text) {
            handleTagPageBreak(element, htmlWriter);
        }
        else if (element.localName() == "a" && element.namespaceURI() == KoXmlNS::text) {
            handleTagA(element, htmlWriter);
        }
        else if (element.localName() == "s" && element.namespaceURI() == KoXmlNS::text) {
            htmlWriter->addTextNode("\u00a0");
        }
        else if (element.localName() == "line-break" && element.namespaceURI() == KoXmlNS::text) {
            handleTagLineBreak(htmlWriter);
        }
        else if (element.localName() == "tab" && element.namespaceURI() == KoXmlNS::text) {
            handleTagTab(htmlWriter);
        }
        else if (element.localName() == "bookmark" && element.namespaceURI() == KoXmlNS::text) {
            handleTagBookMark(element, htmlWriter);
            insideBookmarkTag = true;
        }
        else if (element.localName() == "bookmark-start" && element.namespaceURI() == KoXmlNS::text) {
            handleTagBookMarkStart(element, htmlWriter);
            insideBookmarkTag = true;
        }
        else if (element.localName() == "bookmark-end" && element.namespaceURI() == KoXmlNS::text) {
            // End tag <a> started in bookmark or bookmark-start.
//            handleTagBookMarkEnd(htmlWriter);
        }
        else if (element.localName() == "note" && element.namespaceURI() == KoXmlNS::text) {
            handleTagNote(element, htmlWriter);
        }
        else {
            // FIXME: The same code in convertContent() inserts <div>
            //        around this call.
            handleUnknownTags(element, htmlWriter);
        }

        node = node.nextSibling();
        element = node.toElement();
    }
}


// ----------------------------------------------------------------


void OdtHtmlConverter::collectInternalLinksInfo(KoXmlElement &currentElement, int &chapter)
{
    KoXmlElement nodeElement;
    forEachElement (nodeElement, currentElement) {
        if ( (nodeElement.localName() == "p" || nodeElement.localName() == "h") 
             && nodeElement.namespaceURI() == KoXmlNS::text)
        {
            // A break-before in the style means create a new chapter here,
            // but only if it is a top-level paragraph and not at the very first node.
            StyleInfo *style = m_styles.value(nodeElement.attribute("style-name"));
            if (m_options->doBreakIntoChapters && style && style->shouldBreakChapter) {
                chapter++;
            }
        }
        else if ((nodeElement.localName() == "bookmark-start" || nodeElement.localName() == "bookmark")
                  && nodeElement.namespaceURI() == KoXmlNS::text) {
            QString key = "#" + nodeElement.attribute("name");
            QString value = m_collector->filePrefix();
            if (m_options->doBreakIntoChapters)
                value += QString::number(chapter);
            value += m_collector->fileSuffix();
            m_linksInfo.insert(key, value);
            continue;
        }

        // Check for links recursively also inside this element.
        collectInternalLinksInfo(nodeElement, chapter);
    }
}

void OdtHtmlConverter::writeFootNotes(KoXmlWriter *htmlWriter)
{
    htmlWriter->startElement("p", m_doIndent);
    handleTagLineBreak(htmlWriter);
    htmlWriter->addTextNode("___________________________________________");
    htmlWriter->endElement();

    htmlWriter->startElement("ul", m_doIndent);
    int noteCounts = 1;
    foreach(const QString &id, m_footNotes.keys()) {
        htmlWriter->startElement("li", m_doIndent);
        htmlWriter->addAttribute("id", id + "n");

        htmlWriter->startElement("a", m_doIndent);
        htmlWriter->addAttribute("href", "#" + id + "t"); // reference to text
        htmlWriter->addTextNode("[" + QString::number(noteCounts) + "]");
        htmlWriter->endElement();

        KoXmlElement bodyElement = m_footNotes.value(id);
        handleInsideElementsTag(bodyElement, htmlWriter);

        htmlWriter->endElement();
        noteCounts++;
    }
    htmlWriter->endElement();
    m_footNotes.clear(); // clear for next chapter
}

void OdtHtmlConverter::writeEndNotes(KoXmlWriter *htmlWriter)
{
    htmlWriter->startElement("h1", m_doIndent);
    htmlWriter->addTextNode("End Notes");
    handleTagLineBreak(htmlWriter);
    htmlWriter->endElement();

    htmlWriter->startElement("ul", m_doIndent);
    int noteCounts = 1;
    foreach(const QString &id, m_endNotes.keys()) {
        htmlWriter->startElement("li", m_doIndent);
        htmlWriter->addAttribute("id", id.section("/", 1) + "n");

        htmlWriter->startElement("a", m_doIndent);
        // id = chapter-endnotes.xhtml/endnoteId
        htmlWriter->addAttribute("href",id.section("/", 0, 0) + "#" + id.section("/", 1) + "t");
        htmlWriter->addTextNode("["+QString::number(noteCounts)+"]");
        htmlWriter->endElement();

        KoXmlElement bodyElement = m_endNotes.value(id);
        handleInsideElementsTag(bodyElement, htmlWriter);

        htmlWriter->endElement();
        noteCounts++;
    }
    htmlWriter->endElement();
}

void OdtHtmlConverter::writeMediaOverlayDocumentFile()
{
    QByteArray mediaContent;
    QBuffer buff(&mediaContent);
    KoXmlWriter writer(&buff);

    writer.startElement("smil");
    writer.addAttribute("xmlns", "http://www.w3.org/ns/SMIL");
    writer.addAttribute("version", "3.0");

    writer.startElement("body");

    foreach (const QString &mediaReference, m_mediaFilesList.keys()) {
        writer.startElement("par");

        writer.startElement("text");
        writer.addAttribute("src", mediaReference);
        writer.endElement(); // text

        writer.startElement("audio");
        writer.addAttribute("src", m_mediaFilesList.value(mediaReference).section("/", -1));
        writer.endElement();

        writer.endElement(); // par
    }
    writer.endElement(); // body
    writer.endElement(); // smil

    m_collector->addContentFile(QString("smil"), QString(m_collector->pathPrefix() + "media.smil")
                                ,"application/smil", mediaContent);
}

// ================================================================
//                         Style handling


KoFilter::ConversionStatus OdtHtmlConverter::collectStyles(KoStore *odfStore,
                                                           QHash<QString, StyleInfo*> &styles)
{
    KoXmlDocument doc;
    QString errorMsg;
    int errorLine;
    int errorColumn;

    // ----------------------------------------------------------------
    // Get style info from content.xml.

    // Try to open content.xml. Return if it failed.
    //debugSharedExport << "parse content.xml styles";
    if (!odfStore->open("content.xml")) {
        errorSharedExport << "Unable to open input file! content.xml" << endl;
        return KoFilter::FileNotFound;
    }

    if (!doc.setContent(odfStore->device(), true, &errorMsg, &errorLine, &errorColumn)) {
        debugSharedExport << "Error occurred while parsing styles.xml "
                 << errorMsg << " in Line: " << errorLine
                 << " Column: " << errorColumn;
        odfStore->close();
        return KoFilter::ParsingError;
    }

    // Get the xml node that contains the styles.
    KoXmlNode stylesNode = doc.documentElement();
    stylesNode = KoXml::namedItemNS(stylesNode, KoXmlNS::office, "automatic-styles");

    // Collect info about the styles.
    collectStyleSet(stylesNode, styles);

    odfStore->close(); // end of parsing styles in content.xml

    // ----------------------------------------------------------------
    // Get style info from styles.xml.

    // Try to open and set styles.xml as a KoXmlDocument. Return if it failed.
    if (!odfStore->open("styles.xml")) {
        errorSharedExport << "Unable to open input file! style.xml" << endl;
        return KoFilter::FileNotFound;
    }
    if (!doc.setContent(odfStore->device(), true, &errorMsg, &errorLine, &errorColumn)) {
        debugSharedExport << "Error occurred while parsing styles.xml "
                 << errorMsg << " in Line: " << errorLine
                 << " Column: " << errorColumn;
        odfStore->close();
        return KoFilter::ParsingError;
    }

    // Parse properties of the named styles referred by the automatic
    // styles. Only those styles that are actually used in the
    // document are converted.
    stylesNode = doc.documentElement();
    stylesNode = KoXml::namedItemNS(stylesNode, KoXmlNS::office, "styles");

    // Collect info about the styles.
    collectStyleSet(stylesNode, styles);

    odfStore->close();
    return KoFilter::OK;
}

void OdtHtmlConverter::collectStyleSet(KoXmlNode &stylesNode, QHash<QString, StyleInfo*> &styles)
{
    KoXmlElement styleElement;
    forEachElement (styleElement, stylesNode) {

        // FIXME: Handle text:outline-style also.
        QString tagName = styleElement.tagName();
        if (tagName != "style" && tagName != "default-style")
            continue;

        StyleInfo *styleInfo = new StyleInfo;

        // Get the style name. "." is an illegal character in css style
        // names and needs to be replaced.
        QString styleName = cssClassName(styleElement.attribute("name"));

        // Default styles don't have a name so give them a constructed
        // name by combining "default" and the style family in a way
        // that should not collide with any real style name.
        if (tagName == "default-style") {
            // This name should not collide with any real name.
            styleName = QString("default%") + styleElement.attribute("family");
            styleInfo->isDefaultStyle = true;
        }

        styleInfo->family = styleElement.attribute("family");

        // Every style should have a parent. If the style has no
        // parent, then use the appropriate default style.
        QString parentName = cssClassName(styleElement.attribute("parent-style-name"));
        if (!styleInfo->isDefaultStyle && parentName.isEmpty()) {
            parentName = QString("default%") + styleInfo->family;
        }
        styleInfo->parent = parentName;

        // Limit picture size to 99% of the page size whatever that may be.
        // NOTE: This only makes sense when we convert to HTML.
        if (styleElement.attribute("family") == "graphic") {
            styleInfo->attributes.insert("max-height", "99%");
            styleInfo->attributes.insert("max-width", "99%");
            styleInfo->attributes.insert("height", "auto");
            styleInfo->attributes.insert("width", "auto");
        }

        // Collect default outline level separately because it's used
        // to determine chapter breaks.
        QString dummy = styleElement.attribute("default-outline-level");
        bool  ok;
        styleInfo->defaultOutlineLevel = dummy.toInt(&ok);
        if (!ok)
            styleInfo->defaultOutlineLevel = -1;

        // Go through all property lists (like text-properties,
        // paragraph-properties, etc) and collect the relevant
        // attributes from them.
        styleInfo->shouldBreakChapter = false;
        KoXmlElement propertiesElement;
        forEachElement (propertiesElement, styleElement) {
#if 0 // Disable - use outline-level = 1 instead.
            // Check for fo:break-before
            if (propertiesElement.attribute("break-before") == "page") {
                //debugSharedExport << "Found break-before=page in style" << styleName;
                styleInfo->shouldBreakChapter = true;
            }
#endif

            // Collect general formatting attributes that we can
            // translate to CSS.
            collectStyleAttributes(propertiesElement, styleInfo);
        }

#if 0 // debug
        debugSharedExport << "==" << styleName << ":\t"
                      << styleInfo->parent
                      << styleInfo->family
                      << styleInfo->isDefaultStyle
                      << styleInfo->shouldBreakChapter
                      << styleInfo->attributes;
#endif
        styles.insert(styleName, styleInfo);
    }
}

void OdtHtmlConverter::collectStyleAttributes(KoXmlElement &propertiesElement, StyleInfo *styleInfo)
{
    // font properties
    QString attribute = propertiesElement.attribute("font-family");
    if (!attribute.isEmpty()) {
        attribute = '"' + attribute + '"';
        styleInfo->attributes.insert("font-family", attribute);
    }

    QStringList attributes;
    attributes
        // font
        << "font-style" << "font-variant" << "font-weight" << "font-size"
        // text
        << "text-indent" << "text-align" << "text-decoration" << "white-space"
        // color
        << "color" << "background-color"
        // visual formatting
        << "width" << "min-width" << "max-width"
        << "height" << "min-height" << "max-height" << "line-height" << "vertical-align"
        // border
        << "border-top-width" << "border-bottom-width"
        << "border-left-width" << "border-right-width" << "border-width"
        // border
        << "border-top-color" << "border-bottom-color"
        << "border-left-color" << "border-right-color" << "border-color"
        // border
        << "border-top-style" << "border-bottom-style"
        << "border-left-style" << "border-right-style" << "border-style"
        << "border-top" << "border-bottom" << "border-left" << "border-right" << "border"
        // padding
        << "padding-top" << "padding-bottom" << "padding-left" << "padding-right" << "padding"
        << "margin-top" << "margin-bottom" << "margin-left" << "margin-right" //<< "margin"
        << "auto";

    // Handle all general text formatting attributes
    foreach(const QString &attrName, attributes) {
        QString attrVal = propertiesElement.attribute(attrName);

        if (!attrVal.isEmpty()) {

            // Book readers don't supprt pt unit to can zoom in document.
            // We set em  for our font-size and margins value as unit
            if (attrName == "font-size" || attrName == "margin" || attrName == "margin-right" ||
                    attrName == "margin-left" || attrName == "margin-bottom" || attrName == "margin-top") {
                qreal ptSize = KoUnit::parseValue(attrVal);
                // Convert pt to em: pt/ 12
                qreal emSize = ptSize / 12.0;
                QString fontSize = QString::number(emSize,'g', 2) + "em";
                styleInfo->attributes.insert(attrName, fontSize);
            }
            else {
                styleInfo->attributes.insert(attrName, attrVal);
            }
        }
    }

    // Text Decorations
    attribute = propertiesElement.attribute("text-underline-style");
    if (!attribute.isEmpty() && attribute != "none") {
        styleInfo->attributes.insert("text-decoration", "underline");
    }
    attribute = propertiesElement.attribute("text-overline-style");
    if (!attribute.isEmpty() && attribute != "none") {
        styleInfo->attributes.insert("text-decoration", "overline");
    }
    attribute = propertiesElement.attribute("text-line-through-style");
    if (!attribute.isEmpty() && attribute != "none") {
        styleInfo->attributes.insert("text-decoration", "line-through");
    }

    // Visual Display Model
    attribute = propertiesElement.attribute("writing-mode");
    if (!attribute.isEmpty()) {
        if (attribute == "rl")
            attribute = "rtl";
        else if (attribute == "lr")
            attribute = "ltr";
        else
            attribute = "inherited";
        styleInfo->attributes.insert("direction", attribute);
    }

    // Image align
    attribute = propertiesElement.attribute("horizontal-pos");
    if (!attribute.isEmpty()) {
        //debugSharedExport << "horisontal pos attribute" << attribute;
        if (attribute == "right" || attribute == "from-left") {
            styleInfo->attributes.insert("float", "right");
            styleInfo->attributes.insert("margin", "5px 0 5px 15px");
        }
        // Center doesn't show very well.
//        else if (attribute == "center") {
//            styleInfo->attributes.insert("display", "block");
//            styleInfo->attributes.insert("margin", "10px auto");
//        }
        else if (attribute == "left") {
            styleInfo->attributes.insert("display", "inline");
            styleInfo->attributes.insert("float", "left");
            styleInfo->attributes.insert("margin","5px 15px 5px 0");
        }
    }

    // Lists and numbering
    if (propertiesElement.hasAttribute("num-format")) {
        attribute = propertiesElement.attribute("num-format");
        if (!attribute.isEmpty()) {
            if (attribute == "1")
                attribute = "decimal";
            else if (attribute == "i")
                attribute = "lower-roman";
            else if (attribute == "I")
                attribute = "upper-roman";
            else if (attribute == "a")
                attribute = "lower-alpha";
            else if (attribute == "A")
                attribute = "upper-alpha";
            else
                attribute = "decimal";
        }
        styleInfo->attributes.insert("list-style-type:", attribute);
        styleInfo->attributes.insert("list-style-position:", "outside");
    }
    else if (propertiesElement.hasAttribute("bullet-char")) {
        attribute = propertiesElement.attribute("bullet-char");
        if (!attribute.isEmpty()) {
            switch (attribute[0].unicode()) {
            case 0x2022:
                attribute = "disc";
                break;
            case 0x25CF:
                attribute = "disc";
                break;
            case 0x25CB:
                attribute = "circle";
                break;
            case 0x25A0:
                attribute = "square";
                break;
            default:
                attribute = "disc";
                break;
            }
        }
        styleInfo->attributes.insert("list-style-type:", attribute);
        styleInfo->attributes.insert("list-style-position:", "outside");
    }
}

void OdtHtmlConverter::fixStyleTree(QHash<QString, StyleInfo*> &styles)
{
    // For all styles:
    //    Propagate the shouldBreakChapter bool upwards in the inheritance tree.
    foreach (const QString &styleName, styles.keys()) {
        QVector<StyleInfo *> styleStack(styles.size());

        // Create a stack of styles that we have to check.
        //
        // After this, styleStack will contain a list of styles to
        // check with the deepest one last in the list.
        StyleInfo *style = styles[styleName];
        int index = 0;
        while (style) {
            styleStack[index++] = style;

            // Quit when we are at the bottom or found a break-before.
            if (style->shouldBreakChapter || style->parent.isEmpty()) {
                break;
            }

            style = styles[style->parent];
        }

        // If the bottom most has a break, then all the ones in the list should inherit it.
        if (styleStack[index - 1]->shouldBreakChapter) {
            for (int i = 0; i < index - 1; ++i) {
                styleStack[i]->shouldBreakChapter = true;
            }
        }
    }
}

KoFilter::ConversionStatus OdtHtmlConverter::createCSS(QHash<QString, StyleInfo*> &styles,
                                                       QByteArray &cssContent)
{
    // There is no equivalent to the ODF style inheritance using
    // parent-style-name in CSS. This means that to simulate the same
    // behaviour we have to "flatten" the style tree, i.e. we have to
    // transfer all the inherited attributes from a style's parent
    // into itself.
    flattenStyles(styles);

    QByteArray begin("{\n");
    QByteArray end("}\n");
    foreach (const QString &styleName, styles.keys()) {
        QByteArray head;
        QByteArray attributeList;

        StyleInfo *styleInfo = styles.value(styleName);
        // Disable the test for inUse since we moved the call to before the traversal of the content.
        if (!styleInfo/* || !styleInfo->inUse*/)
            continue;

        // The style name
        head = QString('.' + styleName).toUtf8();
        cssContent.append(head);
        cssContent.append(begin);

        foreach (const QString &propName, styleInfo->attributes.keys()) {
            attributeList += QString(propName + ':' + styleInfo->attributes.value(propName)).toUtf8() + ";\n";
        }

        cssContent.append(attributeList);
        cssContent.append(end);
    }

    return KoFilter::OK;
}

void OdtHtmlConverter::flattenStyles(QHash<QString, StyleInfo*> &styles)
{
    QSet<QString> doneStyles;
    foreach (const QString &styleName, styles.keys()) {
        if (!doneStyles.contains(styleName)) {
            flattenStyle(styleName, styles, doneStyles);
        }
    }
}

void OdtHtmlConverter::flattenStyle(const QString &styleName, QHash<QString, StyleInfo*> &styles,
                                    QSet<QString> &doneStyles)
{
    StyleInfo *styleInfo = styles.value(styleName);
    if (!styleInfo) {
        return;
    }

    // FIXME: Should we also handle styleInfo->defaultOutlineLevel and
    //        styleInfo->shouldBreakChapter?

    QString parentName = styleInfo->parent;
    if (parentName.isEmpty())
        return;

    flattenStyle(styleInfo->parent, styles, doneStyles);

    // Copy all attributes from the parent that is not already in
    // this style into this style.
    StyleInfo *parentInfo = styles.value(parentName);
    if (!parentInfo)
        return;

    foreach(const QString &paramName, parentInfo->attributes.keys()) {
        if (styleInfo->attributes.value(paramName).isEmpty()) {
            styleInfo->attributes.insert(paramName, parentInfo->attributes.value(paramName));
        }
    }

    doneStyles.insert(styleName);
}

QString OdtHtmlConverter::cssClassName(const QString& odfStyleName)
{
    QString retval = odfStyleName;
    QRegExp exp("[^a-zA-Z0-9_]"); //CSS class names are only allowed to use alphanumeric and underscore.
    return retval.replace(exp, "_sc_"); //Not pretty, but it should serve to create unique class names.
}
