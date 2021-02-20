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

class KRITARESOURCES_EXPORT KoMD5Generator
{
public:
    static QByteArray generateHash(const QString &filename);
    static QByteArray generateHash(const QByteArray &array);
};

#endif
