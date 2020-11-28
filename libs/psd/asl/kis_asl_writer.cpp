/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_asl_writer.h"

#include <QDomDocument>
#include <QIODevice>

#include "kis_dom_utils.h"

#include "kis_debug.h"
#include "psd_utils.h"

#include "kis_asl_patterns_writer.h"
#include "kis_asl_writer_utils.h"


namespace Private {

using namespace KisAslWriterUtils;

void parseElement(const QDomElement &el, QIODevice *device, bool forceTypeInfo = false)
{
    KIS_ASSERT_RECOVER_RETURN(el.tagName() == "node");

    QString type = el.attribute("type", "<unknown>");
    QString key = el.attribute("key", "");

    // should be filtered on a higher level
    KIS_ASSERT_RECOVER_RETURN(key != ResourceType::Patterns);

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
        double v = KisDomUtils::toDouble(el.attribute("value", "0"));

        writeVarString(key, device);
        writeFixedString("doub", device);
        SAFE_WRITE_EX(device, v);

    } else if (type == "UnitFloat") {
        double v = KisDomUtils::toDouble(el.attribute("value", "0"));
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
        quint32 v = KisDomUtils::toInt(el.attribute("value", "0"));
        writeVarString(key, device);
        writeFixedString("long", device);
        SAFE_WRITE_EX(device, v);
    } else if (type == "Boolean") {
        quint8 v = KisDomUtils::toInt(el.attribute("value", "0"));

        writeVarString(key, device);
        writeFixedString("bool", device);
        SAFE_WRITE_EX(device, v);
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

void writeFileImpl(QIODevice *device, const QDomDocument &doc)
{
    {
        quint16 stylesVersion = 2;
        SAFE_WRITE_EX(device, stylesVersion);
    }

    {
        QString signature("8BSL");
        if (!device->write(signature.toLatin1().data(), 4)) {
            throw ASLWriteException("Failed to write ASL signature");
        }
    }

    {
        quint16 patternsVersion = 3;
        SAFE_WRITE_EX(device, patternsVersion);
    }

    {
        KisAslWriterUtils::OffsetStreamPusher<quint32> patternsSizeField(device);

        KisAslPatternsWriter patternsWriter(doc, device);
        patternsWriter.writePatterns();
    }

    QDomElement root = doc.documentElement();
    KIS_ASSERT_RECOVER_RETURN(root.tagName() == "asl");

    int numStyles = calculateNumStyles(root);
    KIS_ASSERT_RECOVER_RETURN(numStyles > 0);

    {
        quint32 numStylesTag = numStyles;
        SAFE_WRITE_EX(device, numStylesTag);
    }

    QDomNode child = root.firstChild();

    for (int styleIndex = 0; styleIndex < numStyles; styleIndex++) {
        KisAslWriterUtils::OffsetStreamPusher<quint32> theOnlyStyleSizeField(device);

        KIS_ASSERT_RECOVER_RETURN(!child.isNull());

        {
            quint32 stylesFormatVersion = 16;
            SAFE_WRITE_EX(device, stylesFormatVersion);
        }

        while (!child.isNull()) {
            QDomElement el = child.toElement();
            QString key = el.attribute("key", "");

            if (key != ResourceType::Patterns) break;

            child = child.nextSibling();
        }

        parseElement(child.toElement(), device);
        child = child.nextSibling();

        {
            quint32 stylesFormatVersion = 16;
            SAFE_WRITE_EX(device, stylesFormatVersion);
        }

        parseElement(child.toElement(), device);
        child = child.nextSibling();

        // ASL files' size should be 4-bytes aligned
        const qint64 paddingSize = 4 - (device->pos() & 0x3);
        if (paddingSize != 4) {
            QByteArray padding(paddingSize, '\0');
            device->write(padding);
        }
    }
}

void writePsdLfx2SectionImpl(QIODevice *device, const QDomDocument &doc)
{
    QDomElement root = doc.documentElement();
    KIS_ASSERT_RECOVER_RETURN(root.tagName() == "asl");

    int numStyles = calculateNumStyles(root);
    KIS_ASSERT_RECOVER_RETURN(numStyles == 1);

    {
        quint32 objectEffectsVersion = 0;
        SAFE_WRITE_EX(device, objectEffectsVersion);
    }

    {
        quint32 descriptorVersion = 16;
        SAFE_WRITE_EX(device, descriptorVersion);
    }

    QDomNode child = root.firstChild();

    while (!child.isNull()) {
        QDomElement el = child.toElement();
        QString key = el.attribute("key", "");

        if (key != ResourceType::Patterns) break;

        child = child.nextSibling();
    }

    parseElement(child.toElement(), device);
    child = child.nextSibling();

    // ASL files' size should be 4-bytes aligned
    const qint64 paddingSize = 4 - (device->pos() & 0x3);
    if (paddingSize != 4) {
        QByteArray padding(paddingSize, '\0');
        device->write(padding);
    }
}

} // namespace

void KisAslWriter::writeFile(QIODevice *device, const QDomDocument &doc)
{
    try {
        Private::writeFileImpl(device, doc);
    } catch (Private::ASLWriteException &e) {
        warnKrita << "WARNING: ASL:" << e.what();
    }
}

void KisAslWriter::writePsdLfx2SectionEx(QIODevice *device, const QDomDocument &doc)
{
    Private::writePsdLfx2SectionImpl(device, doc);
}
