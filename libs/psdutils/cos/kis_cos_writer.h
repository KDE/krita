/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCOSWRITER_H
#define KISCOSWRITER_H

#include  <QJsonDocument>
#include "kritapsdutils_export.h"

class KRITAPSDUTILS_EXPORT KisCosWriter
{
public:
    static QByteArray writeCosFromVariantHash(const QVariantHash doc);

    static QByteArray writeTxt2FromVariantHash(const QVariantHash doc);
};

#endif // KISCOSWRITER_H
