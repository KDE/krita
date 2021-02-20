/*
   This file is part of the KDE project
   SPDX-FileCopyrightText: 2000 Werner Trobin <trobin@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIS_TEMPLATES_H
#define KIS_TEMPLATES_H

class QString;

namespace KisTemplates
{
    /**
     * Removes all spaces from the given string, not just from
     * start and end.
     */
     QString trimmed(const QString &string);
}

#endif
