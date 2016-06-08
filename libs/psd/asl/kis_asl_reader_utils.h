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

#ifndef __KIS_ASL_READER_UTILS_H
#define __KIS_ASL_READER_UTILS_H

#include "psd_utils.h"

#include <stdexcept>
#include <string>

#include <QtEndian>

/**
 * Default value for variable read from a file
 */

#define GARBAGE_VALUE_MARK 999

namespace KisAslReaderUtils {

/**
 * Exception that is emitted when any parse error appear.
 * Thanks to KisOffsetOnExitVerifier parsing can be continued
 * most of the time, based on the offset values written in PSD.
 */

struct KRITAPSD_EXPORT ASLParseException : public std::runtime_error
{
    ASLParseException(const QString &msg)
        : std::runtime_error(msg.toAscii().data())
    {
    }
};

}

#define SAFE_READ_EX(device, varname)                                   \
    if (!psdread(device, &varname)) {                                   \
        QString msg = QString("Failed to read \'%1\' tag!").arg(#varname); \
        throw KisAslReaderUtils::ASLParseException(msg);                \
    }

#define SAFE_READ_SIGNATURE_EX(device, varname, expected)               \
    if (!psdread(device, &varname) || varname != expected) {            \
        QString msg = QString("Failed to check signature \'%1\' tag!\n" \
                              "Value: \'%2\' Expected: \'%3\'")         \
            .arg(#varname).arg(varname).arg(expected);                  \
        throw KisAslReaderUtils::ASLParseException(msg);                \
    }

#define SAFE_READ_SIGNATURE_2OPS_EX(device, varname, expected1, expected2) \
    if (!psdread(device, &varname) || (varname != expected1 && varname != expected2)) { \
        QString msg = QString("Failed to check signature \'%1\' tag!\n" \
                              "Value: \'%2\' Expected1: \'%3\' Expected2: \'%4\'") \
            .arg(#varname).arg(varname).arg(expected1).arg(expected2);  \
        throw KisAslReaderUtils::ASLParseException(msg);                \
    }

template <typename T>
inline bool TRY_READ_SIGNATURE_2OPS_EX(QIODevice *device, T expected1, T expected2)
{
    T var;

    qint64 bytesRead = device->peek((char*)&var, sizeof(T));
    if (bytesRead != sizeof(T)) {
        return false;
    }

    var = qFromBigEndian<T>(var);

    bool result = var == expected1 || var == expected2;

    // If read successfully, adjust current position of the io device

    if (result) {
        // read, not seek, to support sequential devices
        bytesRead = device->read((char*)&var, sizeof(T));
        if (bytesRead != sizeof(T)) {
            return false;
        }
    }

    return result;
}

namespace KisAslReaderUtils {

/**
 * String fetch functions
 *
 * ASL has 4 types of strings:
 *
 * - fixed length (4 bytes)
 * - variable length (length (4 bytes) + string (var))
 * - pascal (length (1 byte) + string (var))
 * - unicode string (length (4 bytes) + null-terminated unicode string (var)
 */

KRITAPSD_EXPORT QString readFixedString(QIODevice *device);
KRITAPSD_EXPORT QString readVarString(QIODevice *device);
KRITAPSD_EXPORT QString readPascalString(QIODevice *device);
KRITAPSD_EXPORT QString readUnicodeString(QIODevice *device);

}

#endif /* __KIS_ASL_READER_UTILS_H */
