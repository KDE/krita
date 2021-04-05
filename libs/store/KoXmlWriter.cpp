/* This file is part of the KDE projectz
   SPDX-FileCopyrightText: 2004 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KoXmlWriter.h"

#include <StoreDebug.h>
#include <QByteArray>
#include <QStack>
#include <float.h>
#include "../global/kis_dom_utils.h"

static const int s_indentBufferLength = 100;
static const int s_escapeBufferLen = 10000;

class Q_DECL_HIDDEN KoXmlWriter::Private
{
public:
    Private(QIODevice* dev_, int indentLevel = 0)
        : dev(dev_)
        , baseIndentLevel(indentLevel)
    {}

    ~Private() {
        delete[] indentBuffer;
        delete[] escapeBuffer;
        //TODO: look at if we must delete "dev". For me we must delete it otherwise we will leak it
    }

    QIODevice* dev;
    QStack<Tag> tags;
    int baseIndentLevel;

    char* indentBuffer; // maybe make it static, but then it needs a K_GLOBAL_STATIC
    // and would eat 1K all the time... Maybe refcount it :)
    char* escapeBuffer; // can't really be static if we want to be thread-safe
};

KoXmlWriter::KoXmlWriter(QIODevice* dev, int indentLevel)
        : d(new Private(dev, indentLevel))
{
    d->indentBuffer = new char[ s_indentBufferLength ];
    memset(d->indentBuffer, ' ', s_indentBufferLength);
    *d->indentBuffer = '\n'; // write newline before indentation, in one go

    d->escapeBuffer = new char[s_escapeBufferLen];
    if (!d->dev->isOpen())
        d->dev->open(QIODevice::WriteOnly);

}

KoXmlWriter::~KoXmlWriter()
{
    delete d;
}

void KoXmlWriter::startDocument(const char* rootElemName, const char* publicId, const char* systemId)
{
    Q_ASSERT(d->tags.isEmpty());
    writeCString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    // There isn't much point in a doctype if there's no DTD to refer to
    // (I'm told that files that are validated by a RelaxNG schema cannot refer to the schema)
    if (publicId) {
        writeCString("<!DOCTYPE ");
        writeCString(rootElemName);
        writeCString(" PUBLIC \"");
        writeCString(publicId);
        writeCString("\" \"");
        writeCString(systemId);
        writeCString("\"");
        writeCString(">\n");
    }
}

void KoXmlWriter::endDocument()
{
    // just to do exactly like QDom does (newline at end of file).
    writeChar('\n');
    Q_ASSERT(d->tags.isEmpty());
}

// returns the value of indentInside of the parent
bool KoXmlWriter::prepareForChild(bool indentInside)
{
    if (!d->tags.isEmpty()) {
        Tag& parent = d->tags.top();
        if (!parent.hasChildren) {
            closeStartElement(parent);
            parent.hasChildren = true;
            parent.lastChildIsText = false;
        }
        if (parent.indentInside && indentInside) {
            writeIndent();
        }
        return parent.indentInside && indentInside;
    }
    return indentInside;
}

void KoXmlWriter::prepareForTextNode()
{
    if (d->tags.isEmpty())
        return;
    Tag& parent = d->tags.top();
    if (!parent.hasChildren) {
        closeStartElement(parent);
        parent.hasChildren = true;
        parent.lastChildIsText = true;
    }
}

void KoXmlWriter::startElement(const char* tagName, bool indentInside)
{
    Q_ASSERT(tagName != 0);

    // Tell parent that it has children
    indentInside = prepareForChild(indentInside);

    d->tags.push(Tag(tagName, indentInside));
    writeChar('<');
    writeCString(tagName);
    //kDebug(s_area) << tagName;
}

void KoXmlWriter::addCompleteElement(QIODevice* indev)
{
    prepareForChild();
    const bool wasOpen = indev->isOpen();
    // Always (re)open the device in readonly mode, it might be
    // already open but for writing, and we need to rewind.
    const bool openOk = indev->open(QIODevice::ReadOnly);
    Q_ASSERT(openOk);
    if (!openOk) {
        warnStore << "Failed to re-open the device! wasOpen=" << wasOpen;
        return;
    }

    QString indentString;
    indentString.fill((' '), d->tags.size() + d->baseIndentLevel);
    QByteArray indentBuf(indentString.toUtf8());

    QByteArray buffer;
    while (!indev->atEnd()) {
        buffer = indev->readLine();

        d->dev->write(indentBuf);
        d->dev->write(buffer);
    }

    if (!wasOpen) {
        // Restore initial state
        indev->close();
    }
}

void KoXmlWriter::endElement()
{
    if (d->tags.isEmpty())
        warnStore << "EndElement() was called more times than startElement(). "
                     "The generated XML will be invalid! "
                     "Please report this bug (by saving the document to another format...)" << endl;

    Tag tag = d->tags.pop();

    if (!tag.hasChildren) {
        writeCString("/>");
    } else {
        if (tag.indentInside && !tag.lastChildIsText) {
            writeIndent();
        }
        writeCString("</");
        Q_ASSERT(tag.tagName != 0);
        writeCString(tag.tagName);
        writeChar('>');
    }
}

void KoXmlWriter::addTextNode(const QByteArray& cstr)
{
    // Same as the const char* version below, but here we know the size
    prepareForTextNode();
    char* escaped = escapeForXML(cstr.constData(), cstr.size());
    writeCString(escaped);
    if (escaped != d->escapeBuffer)
        delete[] escaped;
}

void KoXmlWriter::addTextNode(const char* cstr)
{
    prepareForTextNode();
    char* escaped = escapeForXML(cstr, -1);
    writeCString(escaped);
    if (escaped != d->escapeBuffer)
        delete[] escaped;
}

void KoXmlWriter::addAttribute(const char* attrName, const QByteArray& value)
{
    // Same as the const char* one, but here we know the size
    writeChar(' ');
    writeCString(attrName);
    writeCString("=\"");
    char* escaped = escapeForXML(value.constData(), value.size());
    writeCString(escaped);
    if (escaped != d->escapeBuffer)
        delete[] escaped;
    writeChar('"');
}

void KoXmlWriter::addAttribute(const char* attrName, const char* value)
{
    writeChar(' ');
    writeCString(attrName);
    writeCString("=\"");
    char* escaped = escapeForXML(value, -1);
    writeCString(escaped);
    if (escaped != d->escapeBuffer)
        delete[] escaped;
    writeChar('"');
}

void KoXmlWriter::addAttribute(const char* attrName, double value)
{
    addAttribute(attrName, KisDomUtils::toString(value));
}

void KoXmlWriter::addAttribute(const char* attrName, float value)
{
    addAttribute(attrName, KisDomUtils::toString(value));
}

void KoXmlWriter::writeIndent()
{
    // +1 because of the leading '\n'
    d->dev->write(d->indentBuffer, qMin(d->tags.size() + d->baseIndentLevel + 1,
                                        s_indentBufferLength));
}


// In case of a reallocation (ret value != d->buffer), the caller owns the return value,
// it must delete it (with [])
char* KoXmlWriter::escapeForXML(const char* source, int length = -1) const
{
    // we're going to be pessimistic on char length; so lets make the outputLength less
    // the amount one char can take: 6
    char* destBoundary = d->escapeBuffer + s_escapeBufferLen - 6;
    char* destination = d->escapeBuffer;
    char* output = d->escapeBuffer;
    const char* src = source; // src moves, source remains
    for (;;) {
        if (destination >= destBoundary) {
            // When we come to realize that our escaped string is going to
            // be bigger than the escape buffer (this shouldn't happen very often...),
            // we drop the idea of using it, and we allocate a bigger buffer.
            // Note that this if() can only be hit once per call to the method.
            if (length == -1)
                length = qstrlen(source);   // expensive...
            uint newLength = length * 6 + 1; // worst case. 6 is due to &quot; and &apos;
            char* buffer = new char[ newLength ];
            destBoundary = buffer + newLength;
            uint amountOfCharsAlreadyCopied = destination - d->escapeBuffer;
            memcpy(buffer, d->escapeBuffer, amountOfCharsAlreadyCopied);
            output = buffer;
            destination = buffer + amountOfCharsAlreadyCopied;
        }
        switch (*src) {
        case 60: // <
            memcpy(destination, "&lt;", 4);
            destination += 4;
            break;
        case 62: // >
            memcpy(destination, "&gt;", 4);
            destination += 4;
            break;
        case 34: // "
            memcpy(destination, "&quot;", 6);
            destination += 6;
            break;
#if 0 // needed?
        case 39: // '
            memcpy(destination, "&apos;", 6);
            destination += 6;
            break;
#endif
        case 38: // &
            memcpy(destination, "&amp;", 5);
            destination += 5;
            break;
        case 0:
            *destination = '\0';
            return output;
        // Control codes accepted in XML 1.0 documents.
        case 9:
        case 10:
        case 13:
            *destination++ = *src++;
            continue;
        default:
            // Don't add control codes not accepted in XML 1.0 documents.
            if (*src > 0 && *src < 32) {
                ++src;
            } else {
                *destination++ = *src++;
            }
            continue;
        }
        ++src;
    }
    // NOTREACHED (see case 0)
    return output;
}

void KoXmlWriter::addManifestEntry(const QString& fullPath, const QString& mediaType)
{
    startElement("manifest:file-entry");
    addAttribute("manifest:media-type", mediaType);
    addAttribute("manifest:full-path", fullPath);
    endElement();
}

// TODO check return value!!!
void KoXmlWriter::writeCString(const char* cstr) {
    d->dev->write(cstr, qstrlen(cstr));
}

// TODO check return value!!!
void KoXmlWriter::writeChar(char c) {
    d->dev->putChar(c);
}
