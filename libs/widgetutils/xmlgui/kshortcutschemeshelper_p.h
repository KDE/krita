/* This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2008 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KSHORTCUTSCHEMESHELPER_P_H
#define KSHORTCUTSCHEMESHELPER_P_H

#include <QString>
#include <QHash>

class KActionCollection;
class KXMLGUIClient;
class KConfigBase;

class KShortcutSchemesHelper
{
public:

    /**
     * @return the name of the (writable) file to save the shortcut scheme to.
     */
    static QString shortcutSchemeFileName(const QString &schemeName);

    static bool saveShortcutScheme();

    /**
     * @return a list of files to save the shortcut scheme to. Does not include "Default"
     * @see shortcutSchemeFileName, exportActionCollection
     */
    static QHash<QString, QString> schemeFileLocations();

};

#endif
