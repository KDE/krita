/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_asl_writer.h"

#include <QDomDocument>
#include <QIODevice>

#include "kis_dom_utils.h"

#include "kis_debug.h"
#include "psd.h"
#include "psd_utils.h"

#include "kis_asl_patterns_writer.h"
#include "kis_asl_writer_utils.h"

namespace Private
{
using namespace KisAslWriterUtils;

template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
void parseElement(const QDomElement &el, QIODevice &device, bool forceTypeInfo = false)
{
    KIS_ASSERT_RECOVER_RETURN(el.tagName() == "node");

    QString type = el.attribute("type", "<unknown>");
    QString key = el.attribute("key", "");

    // should be filtered on a higher level
    KIS_ASSERT_RECOVER_RETURN(key != ResourceType::Patterns);

    if (type == "Descriptor") {
        if (!key.isEmpty()) {
            writeVarString<byteOrder>(key, device);
        }

        if (!key.isEmpty() || forceTypeInfo) {
            writeFixedString<byteOrder>("Objc", device);
        }

        QString classId = el.attribute("classId", "");
        QString name = el.attribute("name", "");

        writeUnicodeString<byteOrder>(name, device);
        writeVarString<byteOrder>(classId, device);

        quint32 numChildren = static_cast<quint32>(el.childNodes().size());
        SAFE_WRITE_EX(byteOrder, device, numChildren);

        QDomNode child = el.firstChild();
        while (!child.isNull()) {
            parseElement<byteOrder>(child.toElement(), device);
            child = child.nextSibling();
        }

    } else if (type == "List") {
        writeVarString<byteOrder>(key, device);
        writeFixedString<byteOrder>("VlLs", device);

        quint32 numChildren = static_cast<quint32>(el.childNodes().size());
        SAFE_WRITE_EX(byteOrder, device, numChildren);

        QDomNode child = el.firstChild();
        while (!child.isNull()) {
            parseElement<byteOrder>(child.toElement(), device, true);
            child = child.nextSibling();
        }
    } else if (type == "Double") {
        double v = KisDomUtils::toDouble(el.attribute("value", "0"));

        writeVarString<byteOrder>(key, device);
        writeFixedString<byteOrder>("doub", device);
        SAFE_WRITE_EX(byteOrder, device, v);

    } else if (type == "UnitFloat") {
        double v = KisDomUtils::toDouble(el.attribute("value", "0"));
        QString unit = el.attribute("unit", "#Pxl");

        writeVarString<byteOrder>(key, device);
        writeFixedString<byteOrder>("UntF", device);
        writeFixedString<byteOrder>(unit, device);
        SAFE_WRITE_EX(byteOrder, device, v);
    } else if (type == "Text") {
        QString v = el.attribute("value", "");
        writeVarString<byteOrder>(key, device);
        writeFixedString<byteOrder>("TEXT", device);
        writeUnicodeString<byteOrder>(v, device);
    } else if (type == "Enum") {
        QString v = el.attribute("value", "");
        QString typeId = el.attribute("typeId", "DEAD");
        writeVarString<byteOrder>(key, device);
        writeFixedString<byteOrder>("enum", device);
        writeVarString<byteOrder>(typeId, device);
        writeVarString<byteOrder>(v, device);
    } else if (type == "Integer") {
        quint32 v = static_cast<quint32>(KisDomUtils::toInt(el.attribute("value", "0")));
        writeVarString<byteOrder>(key, device);
        writeFixedString<byteOrder>("long", device);
        SAFE_WRITE_EX(byteOrder, device, v);
    } else if (type == "Boolean") {
        quint8 v = static_cast<quint8>(KisDomUtils::toInt(el.attribute("value", "0")));

        writeVarString<byteOrder>(key, device);
        writeFixedString<byteOrder>("bool", device);
        SAFE_WRITE_EX(byteOrder, device, v);
    } else {
        warnKrita << "WARNING: XML (ASL) Unknown element type:" << type << ppVar(key);
    }
}

int calculateNumStyles(const QDomElement &root)
{
    int numStyles = 0;
    QDomNode child = root.firstChild();

    while (!child.isNull()) {
        QDomElement el = child.toElement();
        QString classId = el.attribute("classId", "");

        if (classId == "null") {
            numStyles++;
        }

        child = child.nextSibling();
    }

    return numStyles;
}

// No need for endianness, Photoshop-specific
void writeFileImpl(QIODevice &device, const QDomDocument &doc)
{
    {
        quint16 stylesVersion = 2;
        SAFE_WRITE_EX(psd_byte_order::psdBigEndian, device, stylesVersion);
    }

    {
        QString signature("8BSL");
        if (!device.write(signature.toLatin1().data(), 4)) {
            throw ASLWriteException("Failed to write ASL signature");
        }
    }

    {
        quint16 patternsVersion = 3;
        SAFE_WRITE_EX(psd_byte_order::psdBigEndian, device, patternsVersion);
    }

    {
        KisAslWriterUtils::OffsetStreamPusher<quint32, psd_byte_order::psdBigEndian> patternsSizeField(device);

        KisAslPatternsWriter patternsWriter(doc, device, psd_byte_order::psdBigEndian);
        patternsWriter.writePatterns();
    }

    QDomElement root = doc.documentElement();
    KIS_ASSERT_RECOVER_RETURN(root.tagName() == "asl");

    int numStyles = calculateNumStyles(root);
    KIS_ASSERT_RECOVER_RETURN(numStyles > 0);

    {
        const quint32 numStylesTag = static_cast<quint32>(numStyles);
        SAFE_WRITE_EX(psd_byte_order::psdBigEndian, device, numStylesTag);
    }

    QDomNode child = root.firstChild();

    for (int styleIndex = 0; styleIndex < numStyles; styleIndex++) {
        KisAslWriterUtils::OffsetStreamPusher<quint32, psd_byte_order::psdBigEndian> theOnlyStyleSizeField(device);

        KIS_ASSERT_RECOVER_RETURN(!child.isNull());

        {
            quint32 stylesFormatVersion = 16;
            SAFE_WRITE_EX(psd_byte_order::psdBigEndian, device, stylesFormatVersion);
        }

        while (!child.isNull()) {
            QDomElement el = child.toElement();
            QString key = el.attribute("key", "");

            if (key != ResourceType::Patterns)
                break;

            child = child.nextSibling();
        }

        parseElement(child.toElement(), device);
        child = child.nextSibling();

        {
            quint32 stylesFormatVersion = 16;
            SAFE_WRITE_EX(psd_byte_order::psdBigEndian, device, stylesFormatVersion);
        }

        parseElement(child.toElement(), device);
        child = child.nextSibling();

        // ASL files' size should be 4-bytes aligned
        const qint64 paddingSize = 4 - (device.pos() & 0x3);
        if (paddingSize != 4) {
            QByteArray padding(paddingSize, '\0');
            device.write(padding);
        }
    }
}

template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
void writePsdLfx2SectionImpl(QIODevice &device, const QDomDocument &doc)
{
    QDomElement root = doc.documentElement();
    KIS_ASSERT_RECOVER_RETURN(root.tagName() == "asl");

    int numStyles = calculateNumStyles(root);
    KIS_ASSERT_RECOVER_RETURN(numStyles == 1);

    {
        quint32 objectEffectsVersion = 0;
        SAFE_WRITE_EX(byteOrder, device, objectEffectsVersion);
    }

    {
        quint32 descriptorVersion = 16;
        SAFE_WRITE_EX(byteOrder, device, descriptorVersion);
    }

    QDomNode child = root.firstChild();

    while (!child.isNull()) {
        QDomElement el = child.toElement();
        QString key = el.attribute("key", "");

        if (key != ResourceType::Patterns)
            break;

        child = child.nextSibling();
    }

    parseElement<byteOrder>(child.toElement(), device);
    child = child.nextSibling();

    // ASL files' size should be 4-bytes aligned
    const qint64 paddingSize = 4 - (device.pos() & 0x3);
    if (paddingSize != 4) {
        QByteArray padding(static_cast<int>(paddingSize), '\0');
        device.write(padding);
    }
}

} // namespace

KisAslWriter::KisAslWriter(psd_byte_order byteOrder)
    : m_byteOrder(byteOrder)
{
}

void KisAslWriter::writeFile(QIODevice &device, const QDomDocument &doc)
{
    try {
        Private::writeFileImpl(device, doc);
    } catch (Private::ASLWriteException &e) {
        warnKrita << "WARNING: ASL:" << e.what();
    }
}

void KisAslWriter::writePsdLfx2SectionEx(QIODevice &device, const QDomDocument &doc)
{
    switch (m_byteOrder) {
    case psd_byte_order::psdLittleEndian:
        Private::writePsdLfx2SectionImpl<psd_byte_order::psdLittleEndian>(device, doc);
        break;
    default:
        Private::writePsdLfx2SectionImpl(device, doc);
        break;
    }
}
