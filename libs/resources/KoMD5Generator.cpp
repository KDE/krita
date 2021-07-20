/*
 * SPDX-FileCopyrightText: 2015 Stefano Bonicatti <smjert@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/
#include "KoMD5Generator.h"

#include <QIODevice>
#include <QFile>
#include <QCryptographicHash>

QByteArray KoMD5Generator::generateHash(const QByteArray &array)
{
    QByteArray result;

    if (!array.isEmpty()) {
        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData(array);
        result = md5.result();
    }

    return result;
}

QByteArray KoMD5Generator::generateHash(const QString &filename)
{
    QByteArray result;

    QFile f(filename);
    if (f.exists() && f.open(QIODevice::ReadOnly)) {
        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData(&f);
        result = md5.result();
    }

    return result;
}
