/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef SVGTEXTSHORTCUTS_H
#define SVGTEXTSHORTCUTS_H

#include <QAction>

class QString;
class KoSvgTextProperties;

class SvgTextShortCuts
{
public:
    static QStringList possibleActions();
    static bool configureAction(QAction *action, const QString &name);

    static bool actionEnabled(QAction *action, const QList<KoSvgTextProperties> currentProperties);

    static KoSvgTextProperties getModifiedProperties(const QAction *action, QList<KoSvgTextProperties> currentProperties);
private:
};

#endif // SVGTEXTSHORTCUTS_H
