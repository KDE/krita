/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2000 Werner Trobin <trobin@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KisTemplates.h"

#include <QString>
#include <QChar>

namespace KisTemplates
{
QString trimmed(const QString &string)
{

    QString ret;
    for (int i = 0; i < string.length(); ++i) {
        QChar tmp(string[i]);
        if (!tmp.isSpace())
            ret += tmp;
    }
    return ret;
}
}
