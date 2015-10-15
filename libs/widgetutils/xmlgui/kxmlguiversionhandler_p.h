/* This file is part of the KDE libraries
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2000 Kurt Granroth <granroth@kde.org>
   Copyright     2007 David Faure <faure@kde.org>

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

#ifndef KXMLGUIVERSIONHANDLER_P_H
#define KXMLGUIVERSIONHANDLER_P_H

#include <QStringList>

/**
 * @internal
 * Helper class for KXMLGUIClient::setXMLFile
 * Finds the xml file with the largest version number and takes
 * care of keeping user settings (from the most local file)
 * like action shortcuts or toolbar customizations.
 *
 * This is about handling upgrades (a new version of the application
 * has been installed, with a new xmlgui file, and the user might have
 * a local modified version of an older xmlgui file).
 */
class KXmlGuiVersionHandler
{
public:
    KXmlGuiVersionHandler(const QStringList &files);

    QString finalFile() const
    {
        return m_file;
    }
    QString finalDocument() const
    {
        return m_doc;
    }

    static QString findVersionNumber(const QString &xml);   // used by the unit test

private:
    QString m_file;
    QString m_doc;
};

#endif /* KXMLGUIVERSIONHANDLER_P_H */
