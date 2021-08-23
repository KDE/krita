/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_asl_reader.h"

#include "kis_dom_utils.h"

#include <stdexcept>
#include <string>

#include <QBuffer>
#include <QDomDocument>
#include <QIODevice>

#include "compression.h"
#include "kis_offset_on_exit_verifier.h"
#include "psd.h"
#include "psd_utils.h"

#include "kis_asl_reader_utils.h"
#include "kis_asl_writer_utils.h"

namespace Private
{
/**
 * Numerical fetch functions
 *
 * We read numbers and convert them to strings to be able to store
 * them in XML.
 */

template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
QString readDoubleAsString(QIODevice &device)
{
    double value = 0.0;
    SAFE_READ_EX(byteOrder, device, value);

    return KisDomUtils::toString(value);
}

template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
QString readIntAsString(QIODevice &device)
{
    quint32 value = 0.0;
    SAFE_READ_EX(byteOrder, device, value);

    return KisDomUtils::toString(value);
}

template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
QString readBoolAsString(QIODevice &device)
{
    quint8 value = 0.0;
    SAFE_READ_EX(byteOrder, device, value);

    return KisDomUtils::toString(value);
}

/**
 * XML generation functions
 *
 * Add a node and fill the corresponding attributes
 */

QDomElement appendXMLNodeCommon(const QString &key, const QString &value, const QString &type, QDomElement *parent, QDomDocument *doc)
{
    QDomElement el = doc->createElement("node");
    if (!key.isEmpty()) {
        el.setAttribute("key", key);
    }
    el.setAttribute("type", type);
    el.setAttribute("value", value);
    parent->appendChild(el);

    return el;
}

QDomElement appendXMLNodeCommonNoValue(const QString &key, const QString &type, QDomElement *parent, QDomDocument *doc)
{
    QDomElement el = doc->createElement("node");
    if (!key.isEmpty()) {
        el.setAttribute("key", key);
    }
    el.setAttribute("type", type);
    parent->appendChild(el);

    return el;
}

void appendIntegerXMLNode(const QString &key, const QString &value, QDomElement *parent, QDomDocument *doc)
{
    appendXMLNodeCommon(key, value, "Integer", parent, doc);
}

void appendDoubleXMLNode(const QString &key, const QString &value, QDomElement *parent, QDomDocument *doc)
{
    appendXMLNodeCommon(key, value, "Double", parent, doc);
}

void appendTextXMLNode(const QString &key, const QString &value, QDomElement *parent, QDomDocument *doc)
{
    appendXMLNodeCommon(key, value, "Text", parent, doc);
}

void appendPointXMLNode(const QString &key, const QPointF &pt, QDomElement *parent, QDomDocument *doc)
{
    QDomElement el = appendXMLNodeCommonNoValue(key, "Descriptor", parent, doc);
    el.setAttribute("classId", "CrPt");
    el.setAttribute("name", "");

    appendDoubleXMLNode("Hrzn", KisDomUtils::toString(pt.x()), &el, doc);
    appendDoubleXMLNode("Vrtc", KisDomUtils::toString(pt.x()), &el, doc);
}

/**
 * ASL -> XML parsing functions
 */

template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
void readDescriptor(QIODevice &device, const QString &key, QDomElement *parent, QDomDocument *doc);

template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
void readChildObject(QIODevice &device, QDomElement *parent, QDomDocument *doc, bool skipKey = false)
{
    using namespace KisAslReaderUtils;

    QString key;

    if (!skipKey) {
        key = readVarString<byteOrder>(device);
    }

    QString OSType = readFixedString<byteOrder>(device);

    // dbgKrita << "Child" << ppVar(key) << ppVar(OSType);

    if (OSType == "obj ") {
        throw KisAslReaderUtils::ASLParseException("OSType 'obj' not implemented");

    } else if (OSType == "Objc" || OSType == "GlbO") {
        readDescriptor<byteOrder>(device, key, parent, doc);

    } else if (OSType == "VlLs") {
        quint32 numItems = GARBAGE_VALUE_MARK;
        SAFE_READ_EX(byteOrder, device, numItems);

        QDomElement el = appendXMLNodeCommonNoValue(key, "List", parent, doc);
        for (quint32 i = 0; i < numItems; i++) {
            readChildObject<byteOrder>(device, &el, doc, true);
        }

    } else if (OSType == "doub") {
        appendDoubleXMLNode(key, readDoubleAsString<byteOrder>(device), parent, doc);

    } else if (OSType == "UntF") {
        const QString unit = readFixedString<byteOrder>(device);
        const QString value = readDoubleAsString<byteOrder>(device);

        QDomElement el = appendXMLNodeCommon(key, value, "UnitFloat", parent, doc);
        el.setAttribute("unit", unit);

    } else if (OSType == "TEXT") {
        QString unicodeString = readUnicodeString<byteOrder>(device);
        appendTextXMLNode(key, unicodeString, parent, doc);

    } else if (OSType == "enum") {
        const QString typeId = readVarString<byteOrder>(device);
        const QString value = readVarString<byteOrder>(device);

        QDomElement el = appendXMLNodeCommon(key, value, "Enum", parent, doc);
        el.setAttribute("typeId", typeId);

    } else if (OSType == "long") {
        appendIntegerXMLNode(key, readIntAsString<byteOrder>(device), parent, doc);

    } else if (OSType == "bool") {
        const QString value = readBoolAsString<byteOrder>(device);
        appendXMLNodeCommon(key, value, "Boolean", parent, doc);

    } else if (OSType == "type") {
        throw KisAslReaderUtils::ASLParseException("OSType 'type' not implemented");
    } else if (OSType == "GlbC") {
        throw KisAslReaderUtils::ASLParseException("OSType 'GlbC' not implemented");
    } else if (OSType == "alis") {
        throw KisAslReaderUtils::ASLParseException("OSType 'alis' not implemented");
    } else if (OSType == "tdta") {
        throw KisAslReaderUtils::ASLParseException("OSType 'tdta' not implemented");
    }
}

template<psd_byte_order byteOrder>
void readDescriptor(QIODevice &device, const QString &key, QDomElement *parent, QDomDocument *doc)
{
    using namespace KisAslReaderUtils;

    QString name = readUnicodeString(device);
    QString classId = readVarString(device);

    quint32 numChildren = GARBAGE_VALUE_MARK;
    SAFE_READ_EX(byteOrder, device, numChildren);

    QDomElement el = appendXMLNodeCommonNoValue(key, "Descriptor", parent, doc);
    el.setAttribute("classId", classId);
    el.setAttribute("name", name);

    // dbgKrita << "Descriptor" << ppVar(key) << ppVar(classId) << ppVar(numChildren);

    for (quint32 i = 0; i < numChildren; i++) {
        readChildObject<byteOrder>(device, &el, doc);
    }
}

template<psd_byte_order byteOrder>
QImage readVirtualArrayList(QIODevice &device, int numPlanes)
{
    using namespace KisAslReaderUtils;

    quint32 arrayVersion = GARBAGE_VALUE_MARK;
    SAFE_READ_EX(byteOrder, device, arrayVersion);

    if (arrayVersion != 3) {
        throw ASLParseException("VAList version is not '3'!");
    }

    quint32 arrayLength = GARBAGE_VALUE_MARK;
    SAFE_READ_EX(byteOrder, device, arrayLength);

    SETUP_OFFSET_VERIFIER(vaEndVerifier, device, arrayLength, 100);

    quint32 x0, y0, x1, y1;
    SAFE_READ_EX(byteOrder, device, y0);
    SAFE_READ_EX(byteOrder, device, x0);
    SAFE_READ_EX(byteOrder, device, y1);
    SAFE_READ_EX(byteOrder, device, x1);
    QRect arrayRect(x0, y0, x1 - x0, y1 - y0);

    quint32 numberOfChannels = GARBAGE_VALUE_MARK;
    SAFE_READ_EX(byteOrder, device, numberOfChannels);

    if (numberOfChannels != 24) {
        throw ASLParseException("VAList: Krita doesn't support ASL files with 'numberOfChannels' flag not equal to 24 (it is not documented)!");
    }

    // dbgKrita << ppVar(arrayVersion);
    // dbgKrita << ppVar(arrayLength);
    // dbgKrita << ppVar(arrayRect);
    // dbgKrita << ppVar(numberOfChannels);

    if (numPlanes != 1 && numPlanes != 3) {
        throw ASLParseException("VAList: unsupported number of planes!");
    }

    QVector<QByteArray> dataPlanes;
    dataPlanes.resize(3);

    quint32 pixelDepth1 = GARBAGE_VALUE_MARK;

    for (int i = 0; i < numPlanes; i++) {
        quint32 arrayWritten = GARBAGE_VALUE_MARK;
        if (!psdread<byteOrder>(device, arrayWritten) || !arrayWritten) {
            throw ASLParseException("VAList plane has not-written flag set!");
        }

        quint32 arrayPlaneLength = GARBAGE_VALUE_MARK;
        if (!psdread<byteOrder>(device, arrayPlaneLength) || !arrayPlaneLength) {
            throw ASLParseException("VAList has plane length set to zero!");
        }

        SETUP_OFFSET_VERIFIER(planeEndVerifier, device, arrayPlaneLength, 0);
        qint64 nextPos = device.pos() + arrayPlaneLength;

        SAFE_READ_EX(byteOrder, device, pixelDepth1);

        quint32 x0, y0, x1, y1;
        SAFE_READ_EX(byteOrder, device, y0);
        SAFE_READ_EX(byteOrder, device, x0);
        SAFE_READ_EX(byteOrder, device, y1);
        SAFE_READ_EX(byteOrder, device, x1);
        QRect planeRect(x0, y0, x1 - x0, y1 - y0);

        if (planeRect != arrayRect) {
            throw ASLParseException("VAList: planes are not uniform. Not supported yet!");
        }

        quint16 pixelDepth2 = GARBAGE_VALUE_MARK;
        SAFE_READ_EX(byteOrder, device, pixelDepth2);

        quint8 useCompression = 9;
        SAFE_READ_EX(byteOrder, device, useCompression);

        // dbgKrita << "plane index:" << ppVar(i);
        // dbgKrita << ppVar(arrayWritten);
        // dbgKrita << ppVar(arrayPlaneLength);
        // dbgKrita << ppVar(pixelDepth1);
        // dbgKrita << ppVar(planeRect);
        // dbgKrita << ppVar(pixelDepth2);
        // dbgKrita << ppVar(useCompression);

        if (pixelDepth1 != pixelDepth2) {
            throw ASLParseException("VAList: two pixel depths of the plane are not equal (it is not documented)!");
        }

        if (pixelDepth1 != 8 && pixelDepth1 != 16) {
            throw ASLParseException(QString("VAList: unsupported pixel depth: %1!").arg(pixelDepth1));
        }

        const int channelSize = pixelDepth1 == 8 ? 1 : 2;

        const int dataLength = planeRect.width() * planeRect.height() * channelSize;

        if (useCompression == psd_compression_type::Uncompressed) {
            dataPlanes[i] = device.read(dataLength);

        } else if (useCompression == psd_compression_type::RLE) {
            const int numRows = planeRect.height();

            QVector<quint16> rowSizes;
            rowSizes.resize(numRows);

            for (int row = 0; row < numRows; row++) {
                quint16 rowSize = GARBAGE_VALUE_MARK;
                SAFE_READ_EX(byteOrder, device, rowSize);
                rowSizes[row] = rowSize;
            }

            for (int row = 0; row < numRows; row++) {
                const quint16 rowSize = rowSizes[row];

                QByteArray compressedData = device.read(rowSize);

                if (compressedData.size() != rowSize) {
                    throw ASLParseException("VAList: failed to read compressed data!");
                }

                QByteArray uncompressedData = Compression::uncompress(planeRect.width() * channelSize, compressedData, psd_compression_type::RLE);

                if (uncompressedData.size() != planeRect.width()) {
                    throw ASLParseException("VAList: failed to decompress data!");
                }

                dataPlanes[i].append(uncompressedData);
            }
        } else {
            throw ASLParseException("VAList: ZIP compression is not implemented yet!");
        }

        if (dataPlanes[i].size() != dataLength) {
            throw ASLParseException("VAList: failed to read/uncompress data plane!");
        }

        device.seek(nextPos);
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    const QImage::Format format = pixelDepth1 == 8 ? QImage::Format_ARGB32 : QImage::Format_RGBA64;
#else
    if (pixelDepth1 != 8) {
        throw ASLParseException("Qt does not support RGBA64!");
    }

    const QImage::Format format = QImage::Format_ARGB32;
#endif

    QImage image(arrayRect.size(), format);

    const int dataLength = arrayRect.width() * arrayRect.height();

    if (format == QImage::Format_ARGB32) {
        quint8 *dstPtr = image.bits();

        for (int i = 0; i < dataLength; i++) {
            for (int j = 2; j >= 0; j--) {
                const int plane = qMin(numPlanes, j);
                *dstPtr++ = dataPlanes[plane][i];
            }
            *dstPtr++ = 0xFF;
        }
    } else {
        quint16 *dstPtr = reinterpret_cast<quint16 *>(image.bits());

        for (int i = 0; i < dataLength; i++) {
            for (int j = 0; j <= 2; j++) {
                const int plane = qMin(numPlanes, j);
                const quint16 *dataPlane = reinterpret_cast<const quint16 *>(dataPlanes[plane].constData());
                *dstPtr++ = qFromBigEndian(dataPlane[i]);
            }
            *dstPtr++ = 0xFFFF;
        }
    }

    // static int i = -1; i++;
    // QString filename = QString("pattern_image_%1.png").arg(i);
    // dbgKrita << "### dumping pattern image" << ppVar(filename);
    // image.save(filename);

    return image.convertToFormat(QImage::Format_ARGB32, Qt::AutoColor | Qt::PreferDither);
}

template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
qint64 readPattern(QIODevice &device, QDomElement *parent, QDomDocument *doc)
{
    using namespace KisAslReaderUtils;

    quint32 patternSize = GARBAGE_VALUE_MARK;
    SAFE_READ_EX(byteOrder, device, patternSize);

    // patterns are always aligned by 4 bytes
    patternSize = KisAslWriterUtils::alignOffsetCeil(patternSize, 4);

    SETUP_OFFSET_VERIFIER(patternEndVerifier, device, patternSize, 0);

    quint32 patternVersion = GARBAGE_VALUE_MARK;
    SAFE_READ_EX(byteOrder, device, patternVersion);

    if (patternVersion != 1) {
        throw ASLParseException("Pattern version is not \'1\'");
    }

    quint32 patternImageMode = GARBAGE_VALUE_MARK;
    SAFE_READ_EX(byteOrder, device, patternImageMode);

    quint16 patternHeight = GARBAGE_VALUE_MARK;
    SAFE_READ_EX(byteOrder, device, patternHeight);

    quint16 patternWidth = GARBAGE_VALUE_MARK;
    SAFE_READ_EX(byteOrder, device, patternWidth);

    QString patternName;
    psdread_unicodestring<byteOrder>(device, patternName);

    QString patternUuid = readPascalString<byteOrder>(device);

    // dbgKrita << "--";
    // dbgKrita << ppVar(patternSize);
    // dbgKrita << ppVar(patternImageMode);
    // dbgKrita << ppVar(patternHeight);
    // dbgKrita << ppVar(patternWidth);
    // dbgKrita << ppVar(patternName);
    // dbgKrita << ppVar(patternUuid);

    int numPlanes = 0;
    psd_color_mode mode = static_cast<psd_color_mode>(patternImageMode);

    switch (mode) {
    case MultiChannel:
    case Grayscale:
        numPlanes = 1;
        break;
    case RGB:
        numPlanes = 3;
        break;
    default: {
        QString msg = QString("Unsupported image mode: %1!").arg(mode);
        throw ASLParseException(msg);
    }
    }

    /**
     * Create XML data
     */

    QDomElement pat = doc->createElement("node");

    pat.setAttribute("classId", "KisPattern");
    pat.setAttribute("type", "Descriptor");
    pat.setAttribute("name", "");

    QBuffer patternBuf;
    patternBuf.open(QIODevice::WriteOnly);

    { // ensure we don't keep resources for too long
        // XXX: this QImage should tolerate 16-bit and higher
        QString fileName = QString("%1.pat").arg(patternUuid);
        QImage patternImage = readVirtualArrayList<byteOrder>(device, numPlanes);
        KoPattern realPattern(patternImage, patternName, fileName);
        realPattern.savePatToDevice(&patternBuf);
    }

    /**
     * We are loading the pattern and convert it into ARGB right away,
     * so we need not store real image mode and size of the pattern
     * externally.
     */
    appendTextXMLNode("Nm  ", patternName, &pat, doc);
    appendTextXMLNode("Idnt", patternUuid, &pat, doc);

    QDomCDATASection dataSection = doc->createCDATASection(qCompress(patternBuf.buffer()).toBase64());

    QDomElement dataElement = doc->createElement("node");
    dataElement.setAttribute("type", "KisPatternData");
    dataElement.setAttribute("key", "Data");

    dataElement.appendChild(dataSection);
    pat.appendChild(dataElement);
    parent->appendChild(pat);

    return sizeof(patternSize) + patternSize;
}

QDomDocument readFileImpl(QIODevice &device)
{
    using namespace KisAslReaderUtils;

    QDomDocument doc;
    QDomElement root = doc.createElement("asl");
    doc.appendChild(root);

    {
        quint16 stylesVersion = GARBAGE_VALUE_MARK;
        SAFE_READ_SIGNATURE_EX(psd_byte_order::psdBigEndian, device, stylesVersion, 2);
    }

    {
        quint32 aslSignature = GARBAGE_VALUE_MARK;
        const quint32 refSignature = 0x3842534c; // '8BSL' in little-endian
        SAFE_READ_SIGNATURE_EX(psd_byte_order::psdBigEndian, device, aslSignature, refSignature);
    }

    {
        quint16 patternsVersion = GARBAGE_VALUE_MARK;
        SAFE_READ_SIGNATURE_EX(psd_byte_order::psdBigEndian, device, patternsVersion, 3);
    }

    // Patterns

    {
        quint32 patternsSize = GARBAGE_VALUE_MARK;
        SAFE_READ_EX(psd_byte_order::psdBigEndian, device, patternsSize);

        if (patternsSize > 0) {
            SETUP_OFFSET_VERIFIER(patternsSectionVerifier, device, patternsSize, 0);

            QDomElement patternsRoot = doc.createElement("node");
            patternsRoot.setAttribute("type", "List");
            patternsRoot.setAttribute("key", ResourceType::Patterns);
            root.appendChild(patternsRoot);

            try {
                qint64 bytesRead = 0;
                while (bytesRead < patternsSize) {
                    qint64 chunk = readPattern(device, &patternsRoot, &doc);
                    bytesRead += chunk;
                }
            } catch (ASLParseException &e) {
                warnKrita << "WARNING: ASL (emb. pattern):" << e.what();
            }
        }
    }

    // Styles

    quint32 numStyles = GARBAGE_VALUE_MARK;
    SAFE_READ_EX(psd_byte_order::psdBigEndian, device, numStyles);

    for (int i = 0; i < (int)numStyles; i++) {
        quint32 bytesToRead = GARBAGE_VALUE_MARK;
        SAFE_READ_EX(psd_byte_order::psdBigEndian, device, bytesToRead);

        SETUP_OFFSET_VERIFIER(singleStyleSectionVerifier, device, bytesToRead, 0);

        {
            quint32 stylesFormatVersion = GARBAGE_VALUE_MARK;
            SAFE_READ_SIGNATURE_EX(psd_byte_order::psdBigEndian, device, stylesFormatVersion, 16);
        }

        readDescriptor(device, "", &root, &doc);

        {
            quint32 stylesFormatVersion = GARBAGE_VALUE_MARK;
            SAFE_READ_SIGNATURE_EX(psd_byte_order::psdBigEndian, device, stylesFormatVersion, 16);
        }

        readDescriptor(device, "", &root, &doc);
    }

    return doc;
}

} // namespace

QDomDocument KisAslReader::readFile(QIODevice &device)
{
    QDomDocument doc;

    if (device.isSequential()) {
        warnKrita << "WARNING: *** KisAslReader::readFile: the supplied"
                  << "IO device is sequential. Chances are that"
                  << "the layer style will *not* be loaded correctly!";
    }

    try {
        doc = Private::readFileImpl(device);
    } catch (KisAslReaderUtils::ASLParseException &e) {
        warnKrita << "WARNING: ASL:" << e.what();
    }

    return doc;
}

template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
QDomDocument readLfx2PsdSectionImpl(QIODevice &device);

QDomDocument KisAslReader::readLfx2PsdSection(QIODevice &device, psd_byte_order byteOrder)
{
    switch (byteOrder) {
    case psd_byte_order::psdLittleEndian:
        return readLfx2PsdSectionImpl<psd_byte_order::psdLittleEndian>(device);
    default:
        return readLfx2PsdSectionImpl(device);
    }
}

template<psd_byte_order byteOrder>
QDomDocument readLfx2PsdSectionImpl(QIODevice &device)
{
    QDomDocument doc;

    if (device.isSequential()) {
        warnKrita << "WARNING: *** KisAslReader::readLfx2PsdSection: the supplied"
                  << "IO device is sequential. Chances are that"
                  << "the layer style will *not* be loaded correctly!";
    }

    try {
        {
            quint32 objectEffectsVersion = GARBAGE_VALUE_MARK;
            const quint32 ref = 0x00;
            SAFE_READ_SIGNATURE_EX(byteOrder, device, objectEffectsVersion, ref);
        }

        {
            quint32 descriptorVersion = GARBAGE_VALUE_MARK;
            const quint32 ref = 0x10;
            SAFE_READ_SIGNATURE_EX(byteOrder, device, descriptorVersion, ref);
        }

        QDomElement root = doc.createElement("asl");
        doc.appendChild(root);

        Private::readDescriptor<byteOrder>(device, "", &root, &doc);

    } catch (KisAslReaderUtils::ASLParseException &e) {
        warnKrita << "WARNING: PSD: lfx2 section:" << e.what();
    }

    return doc;
}

template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
QDomDocument readPsdSectionPatternImpl(QIODevice &device, qint64 bytesLeft);

QDomDocument KisAslReader::readPsdSectionPattern(QIODevice &device, qint64 bytesLeft, psd_byte_order byteOrder)
{
    switch (byteOrder) {
    case psd_byte_order::psdLittleEndian:
        return readPsdSectionPatternImpl<psd_byte_order::psdLittleEndian>(device, bytesLeft);
    default:
        return readPsdSectionPatternImpl(device, bytesLeft);
    }
}

template<psd_byte_order byteOrder>
QDomDocument readPsdSectionPatternImpl(QIODevice &device, qint64 bytesLeft)
{
    QDomDocument doc;

    QDomElement root = doc.createElement("asl");
    doc.appendChild(root);

    QDomElement pat = doc.createElement("node");
    root.appendChild(pat);

    pat.setAttribute("classId", ResourceType::Patterns);
    pat.setAttribute("type", "Descriptor");
    pat.setAttribute("name", "");

    try {
        qint64 bytesRead = 0;
        while (bytesRead < bytesLeft) {
            qint64 chunk = Private::readPattern<byteOrder>(device, &pat, &doc);
            bytesRead += chunk;
        }
    } catch (KisAslReaderUtils::ASLParseException &e) {
        warnKrita << "WARNING: PSD (emb. pattern):" << e.what();
    }

    return doc;
}
