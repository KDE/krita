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
/**
 * @brief The SvgTextShortCuts class
 *
 * Class to handle text property shortcuts generically.
 *
 * Many text property shorcuts are about toggling/enabling a single property.
 * Given there's a huge amount of them, it thus makes sense to generalize the
 * actions by adding a special QVariant to them and using that QVariant to
 * determine which property adjustment is at play.
 *
 */
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
