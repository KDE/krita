/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_asl_writer.h"

#include <stdexcept>
#include <string>

#include <QDomDocument>
#include <QIODevice>

#include "kis_dom_utils.h"

#include "kis_debug.h"
#include "psd_utils.h"

namespace Private {

/**
 * Exception that is emitted when any write error appear.
 */
struct ASLWriteException : public std::runtime_error
{
    ASLWriteException(const QString &msg)
        : std::runtime_error(msg.toAscii().data())
    {
    }
};

#define SAFE_WRITE_EX(device, varname)                                  \
    if (!psdwrite(device, varname)) {                                   \
        QString msg = QString("Failed to write \'%1\' tag!").arg(#varname); \
        throw ASLWriteException(msg);                                   \
    }

void writeUnicodeString(const QString &value, QIODevice *device)
{
    quint32 len = value.length() + 1;
    SAFE_WRITE_EX(device, len);

    const quint16 *ptr = value.utf16();
    for (quint32 i = 0; i < len; i++) {
        SAFE_WRITE_EX(device, ptr[i]);
    }
}

void writeVarString(const QString &value, QIODevice *device)
{
    quint32 lenTag = value.length() != 4 ? value.length() : 0;
    SAFE_WRITE_EX(device, lenTag);

    if (!device->write(value.toAscii().data(), value.length())) {
        qWarning() << "WARNING: ASL: Failed to write ASL string" << ppVar(value);
        return;
    }
}

void writeFixedString(const QString &value, QIODevice *device)
{
    KIS_ASSERT_RECOVER_RETURN(value.length() == 4);

    if (!device->write(value.toAscii().data(), value.length())) {
        qWarning() << "WARNING: ASL: Failed to write ASL string" << ppVar(value);
        return;
    }
}

void parseElement(const QDomElement &el, QIODevice *device, bool forceTypeInfo = false)
{
    KIS_ASSERT_RECOVER_RETURN(el.tagName() == "node");

    QString type = el.attribute("type", "<unknown>");
    QString key = el.attribute("key", "");

    if (type == "Descriptor") {
        if (!key.isEmpty()) {
            writeVarString(key, device);
        }

        if (!key.isEmpty() || forceTypeInfo) {
            writeFixedString("Objc", device);
        }


        QString classId = el.attribute("classId", "");
        QString name = el.attribute("name", "");

        writeUnicodeString(name, device);
        writeVarString(classId, device);

        quint32 numChildren = el.childNodes().size();
        SAFE_WRITE_EX(device, numChildren);

        QDomNode child = el.firstChild();
        while (!child.isNull()) {
            parseElement(child.toElement(), device);
            child = child.nextSibling();
        }

    } else if (type == "List") {
        writeVarString(key, device);
        writeFixedString("VlLs", device);

        quint32 numChildren = el.childNodes().size();
        SAFE_WRITE_EX(device, numChildren);

        QDomNode child = el.firstChild();
        while (!child.isNull()) {
            parseElement(child.toElement(), device, true);
            child = child.nextSibling();
        }
    } else if (type == "Double") {
        double v = KisDomUtils::Private::stringToDouble(el.attribute("value", "0"));

        writeVarString(key, device);
        writeFixedString("doub", device);
        SAFE_WRITE_EX(device, v);

    } else if (type == "UnitFloat") {
        double v = KisDomUtils::Private::stringToDouble(el.attribute("value", "0"));
        QString unit = el.attribute("unit", "#Pxl");

        writeVarString(key, device);
        writeFixedString("UntF", device);
        writeFixedString(unit, device);
        SAFE_WRITE_EX(device, v);
    } else if (type == "Text") {
        QString v = el.attribute("value", "");
        writeVarString(key, device);
        writeFixedString("TEXT", device);
        writeUnicodeString(v, device);
    } else if (type == "Enum") {
        QString v = el.attribute("value", "");
        QString typeId = el.attribute("typeId", "DEAD");
        writeVarString(key, device);
        writeFixedString("enum", device);
        writeVarString(typeId, device);
        writeVarString(v, device);
    } else if (type == "Integer") {
        quint32 v = KisDomUtils::Private::stringToInt(el.attribute("value", "0"));
        writeVarString(key, device);
        writeFixedString("long", device);
        SAFE_WRITE_EX(device, v);
    } else if (type == "Boolean") {
        quint8 v = KisDomUtils::Private::stringToInt(el.attribute("value", "0"));

        writeVarString(key, device);
        writeFixedString("bool", device);
        SAFE_WRITE_EX(device, v);
    } else {
        qWarning() << "WARNING: XML (ASL) Unknown element type:" << type << ppVar(key);
    }
}

void writeFileImpl(QIODevice *device, const QDomDocument &doc)
{
    {
        quint16 stylesVersion = 2;
        SAFE_WRITE_EX(device, stylesVersion);
    }

    {
        QString signature("8BSL");
        if (!device->write(signature.toAscii().data(), 4)) {
            throw ASLWriteException("Failed to write ASL signature");
        }
    }

    {
        quint16 patternsVersion = 3;
        SAFE_WRITE_EX(device, patternsVersion);
    }

    {
        // FIXME: not implemented yet
        quint32 patternsSize = 0;
        SAFE_WRITE_EX(device, patternsSize);
    }

    {
        // FIXME: real number of styles
        quint32 numStyles = 1;
        SAFE_WRITE_EX(device, numStyles);
    }

    const qint64 savedStylesSizeOffset = device->pos();

    {
        // FIXME: correctLater
        quint32 bytesToRead = 0xdeadbeef;
        SAFE_WRITE_EX(device, bytesToRead);
    }

    {
        quint32 stylesFormatVersion = 16;
        SAFE_WRITE_EX(device, stylesFormatVersion);
    }

    QDomElement root = doc.documentElement();
    KIS_ASSERT_RECOVER_RETURN(root.tagName() == "asl");

    QDomNode child = root.firstChild();

    parseElement(child.toElement(), device);
    child = child.nextSibling();

    {
        quint32 stylesFormatVersion = 16;
        SAFE_WRITE_EX(device, stylesFormatVersion);
    }

    while (!child.isNull()) {
        parseElement(child.toElement(), device);
        child = child.nextSibling();
    }

    // ASL files' size should be 4-bytes aligned
    const qint64 paddingSize = 4 - (device->pos() & 0x3);
    if (paddingSize != 4) {
        QByteArray padding(paddingSize, '\0');
        device->write(padding);
    }

    const qint64 fileSize = device->pos();
    qint32 stylesSize = fileSize - savedStylesSizeOffset - sizeof(quint32);

    KIS_ASSERT_RECOVER(stylesSize > 0) {
        stylesSize = 0;
    }

    device->seek(savedStylesSizeOffset);
    SAFE_WRITE_EX(device, (quint32)stylesSize);

    // jump back to the end of the file to be consistent
    device->seek(device->size());
}

} // namespace

void KisAslWriter::writeFile(QIODevice *device, const QDomDocument &doc)
{
    try {
        Private::writeFileImpl(device, doc);
    } catch (Private::ASLWriteException &e) {
        qWarning() << "WARNING: ASL:" << e.what();
    }
}
