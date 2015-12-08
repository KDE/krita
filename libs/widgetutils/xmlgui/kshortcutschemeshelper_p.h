/* This file is part of the KDE libraries
    Copyright (C) 2008 Alexander Dymo <adymo@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KSHORTCUTSCHEMESHELPER_P_H
#define KSHORTCUTSCHEMESHELPER_P_H

#include <QString>

class KActionCollection;
class KXMLGUIClient;

class KShortcutSchemesHelper
{
public:

    /**
     * Saves actions from the collection to shortcut scheme file.
     * If the url is given, saves to that directory, otherwise saves to user appdata directory.
     *
     * This doesn't save anything if there's no parent xmlgui client for action collection.
     *
     * The name of the resulting file is: shortcuts/<scheme name>.rc .
     *
     *  @return true if the action collection was exported
     */
    static bool exportActionCollection(KActionCollection *collection, const QString &schemeName,
                                       const QString &dir = QString());

    /**
     * @return the current shortcut scheme name for the application.
     */
    static QString currentShortcutSchemeName();

    /**
     * @return the name of the (writable) file to save the shortcut scheme to.
     */
    static QString shortcutSchemeFileName(const QString &schemeName);

    static bool saveShortcutScheme();
};

#endif
