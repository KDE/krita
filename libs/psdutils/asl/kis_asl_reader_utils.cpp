/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_asl_reader_utils.h"

namespace KisAslReaderUtils
{
QString readStringCommon(QIODevice &device, int length)
{
    QByteArray data;
    data.resize(length);
    qint64 dataRead = device.read(data.data(), length);

    if (dataRead != length) {
        QString msg = QString(
                          "Failed to read a string! "
                          "Bytes read: %1 Expected: %2")
                          .arg(dataRead)
                          .arg(length);
        throw ASLParseException(msg);
    }

    return QString(data);
}

QString readFixedString(QIODevice &device)
{
    return readStringCommon(device, 4);
}

QString readVarString(QIODevice &device)
{
    quint32 length = 0;
    SAFE_READ_EX(device, length);

    if (!length) {
        length = 4;
    }

    return readStringCommon(device, length);
}

QString readPascalString(QIODevice &device)
{
    quint8 length = 0;
    SAFE_READ_EX(device, length);

    return readStringCommon(device, length);
}

QString readUnicodeString(QIODevice &device)
{
    QString string;

    if (!psdread_unicodestring(device, string)) {
        QString msg = QString("Failed to read a unicode string!");
        throw ASLParseException(msg);
    }

    return string;
}

}
