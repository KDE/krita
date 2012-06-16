/*
 *  Copyright (c) 2010 Mani Chandrasekar <maninc@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef ONLINEDOCUMENT_H
#define ONLINEDOCUMENT_H

#include <kparts/plugin.h>

class GoogleDocumentService;
class LoginWindow;

class OnlineDocument : public KParts::Plugin
{
    Q_OBJECT
public:
    enum DocumentType {
        WORDS,
        STAGE,
        SHEETS,
        UNKNOWN
    };

    OnlineDocument(QObject *parent, const QVariantList &);
    virtual ~OnlineDocument();

private slots:
    void slotOnlineDocument();
    void receivedOnlineDocument(QString path);

private:
    LoginWindow *m_login;
    DocumentType m_type;
};

#endif
