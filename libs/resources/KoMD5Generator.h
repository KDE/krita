#/*
 * SPDX-FileCopyrightText: 2015 Stefano Bonicatti <smjert@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/
#ifndef KOMD5GENERATOR_H
#define KOMD5GENERATOR_H

#include <QByteArray>
#include <QString>

#include <kritaresources_export.h>

class QIODevice;

class KRITARESOURCES_EXPORT KoMD5Generator
{
public:
    /**
     * @brief generateHash reads the given file and generates
     * a hex-encoded md5sum for the file.
     * @param filename the file to open
     * @return a hex-encoded string representation of the md5sum
     */
    static QString generateHash(const QString &filename);

    /**
     * @brief generateHash calculates the md5sum of the given bytes
     * @param QByteArray the contents to be calculated
     * @return a hex-encoded string representation of the md5sum
     */
    static QString generateHash(const QByteArray &array);

    /**
     * @brief generateHash calculates the md5sum of the given device
     * @param QIODevice
     * @return a hex-encoded string representation of the md5sum
     */
    static QString generateHash(QIODevice *device);
};

#endif
