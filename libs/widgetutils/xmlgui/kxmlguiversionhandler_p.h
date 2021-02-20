/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2000 Simon Hausmann <hausmann@kde.org>
   SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
   SPDX-FileCopyrightText: 2007 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
