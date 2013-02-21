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

#ifndef GOOGLEDOCUMENTSERVICE_H
#define GOOGLEDOCUMENTSERVICE_H

#include <QObject>
#include <QXmlInputSource>
#include <QNetworkAccessManager>

#include "onlinedocument.h"
#include "documentlistwindow.h"

class GoogleDocument;
class GoogleContentHandler;
class DocumentListWindow;

class GoogleDocumentService : public QObject
{
    Q_OBJECT

public:
    explicit GoogleDocumentService(OnlineDocument::DocumentType type);
    virtual ~GoogleDocumentService();
    void clientLogin(const QString & username, const QString & password);
    void getDocument();
    void downloadDocument (const QString & url, const QString & type);
    bool alreadyAuthenticated() { return loggedin; }
    void showDocumentListWindow(bool visible);

signals:
    void receivedDocument(QString path);
    void userAuthenticated(bool success, QString error);
    void progressUpdate(const QString &msg);
    void showingDocumentList();

private slots:
    void handleNetworkData(QNetworkReply *networkReply);

private:
    void listDocuments();

    QXmlSimpleReader xmlReader;
    QXmlInputSource  xmlInput;
    bool newInformation;
    bool waitingForDoc;
    bool haveDocAuthToken;
    GoogleContentHandler *gHandler;
    QString authToken;
    DocumentListWindow *documentList;
    QString docAuthToken;
    QString spreadAuthToken;
    QString username;
    QString password;
    QNetworkAccessManager networkManager;
    bool loggedin;
    OnlineDocument::DocumentType m_type;
};

#endif // GOOGLEDOCUMENTSERVICE_H
