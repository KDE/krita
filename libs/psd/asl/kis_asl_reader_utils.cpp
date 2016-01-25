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

#include "kis_asl_reader_utils.h"

namespace KisAslReaderUtils {

QString readStringCommon(QIODevice *device, int length)
{
    QByteArray data;
    data.resize(length);
    qint64 dataRead = device->read(data.data(), length);

    if (dataRead != length) {
        QString msg =
            QString("Failed to read a string! "
                    "Bytes read: %1 Expected: %2")
            .arg(dataRead).arg(length);
        throw ASLParseException(msg);
    }

    return QString(data);
}

QString readFixedString(QIODevice *device) {
    return readStringCommon(device, 4);
}

QString readVarString(QIODevice *device) {
    quint32 length = 0;
    SAFE_READ_EX(device, length);

    if (!length) {
        length = 4;
    }

    return readStringCommon(device, length);
}

QString readPascalString(QIODevice *device) {
    quint8 length = 0;
    SAFE_READ_EX(device, length);

    return readStringCommon(device, length);
}

QString readUnicodeString(QIODevice *device) {
    QString string;

    if (!psdread_unicodestring(device, string)) {
        QString msg = QString("Failed to read a unicode string!");
        throw ASLParseException(msg);
    }

    return string;
}

}
