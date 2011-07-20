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

#include <QMessageBox>

#include "googledocumentservice.h"
#include "googledocumentlist.h"
#include "googledocument.h"
#include "googlecontenthandler.h"

#include <QDir>
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>

const QString GoogleDocumentService::GOOGLE_DOCUMENT_URL = "docs.google.com";
const QString GoogleDocumentService::GOOGLE_SPREADSHEET_URL = "spreadsheets.google.com";

GoogleDocumentService::GoogleDocumentService()
        : newInformation(true)
          , waitingForDoc(false)
          , loggedin(false)
          , documentList(0)
{
    connect(&networkManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(handleNetworkData(QNetworkReply*)));

    gHandler = new GoogleContentHandler();
    xmlReader.setContentHandler(gHandler);
}

GoogleDocumentService::~GoogleDocumentService()
{
    delete documentList;
    documentList = 0;

    delete gHandler;
    gHandler = 0;
}

void GoogleDocumentService::clientLogin(const QString & username, const QString & password)
{
    QByteArray data;
    data.append(QString("Email=" + username + "&Passwd=" + password).toUtf8());

    if(!haveDocAuthToken) {
        data.append(QString("&service=writely&source=Calligrav2").toUtf8());
        this->username = username;
        this->password = password;
    } else {
        data.append(QString("&service=wise&source=Calligrav2").toUtf8());
    }
    QNetworkRequest req(QUrl("https://www.google.com/accounts/ClientLogin"));
    req.setRawHeader("Host", "www.google.com");
    req.setRawHeader("GData-Version", "3.0");
    req.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    req.setHeader(QNetworkRequest::ContentLengthHeader, data.length());

    networkManager.post(req, data);
}

void GoogleDocumentService::listDocuments()
{
    authToken = docAuthToken;
    QNetworkRequest requestHeader(QUrl("https://docs.google.com/feeds/default/private/full"));
    requestHeader.setRawHeader("Host", "docs.google.com");
    requestHeader.setRawHeader("User-Agent", "Calligra");
    requestHeader.setRawHeader("GData-Version", "3.0");
    requestHeader.setRawHeader("Content-Type", "application/atom+xml");
    requestHeader.setRawHeader("Authorization", authToken.toUtf8());

    networkManager.get(requestHeader);
    emit progressUpdate("Successfully authenticated!!! Retreiving document list...");
}

void GoogleDocumentService::handleNetworkData(QNetworkReply *networkReply)
{
    QUrl url = networkReply->url();
    if (!networkReply->error()) {
        if (!loggedin) {
            QString text(networkReply->readAll());
            text = text.right(text.length() - text.indexOf("Auth=") - 5);
            authToken = QString("GoogleLogin auth=") + text.left(text.indexOf("\n"));
            if(authToken.length() > 20) {
                if(!haveDocAuthToken) {
                    docAuthToken = authToken;
                    haveDocAuthToken = true;
                    clientLogin(this->username, this->password);
                    return;
                }
                listDocuments();
                spreadAuthToken = authToken;
                authToken = "";
                loggedin = true;
                emit userAuthenticated(loggedin, "");
            }
        }
        else if (waitingForDoc) {
            QByteArray data = networkReply->readAll();
            QFile file(QDir::tempPath() + "/" + documentList->currentDocument());
            file.open(QIODevice::ReadWrite);
            file.write(data);
            file.close();
            qDebug() << "Received Document!!!!! " << file.fileName();
            emit receivedDocument(file.fileName());
            waitingForDoc = false;
            hideDocumentListWindow();
        }
        else {
            xmlInput.setData(networkReply->readAll());
            qDebug() << "Part received.........";
            if (newInformation) {
                emit progressUpdate("Parsing document list...");
                xmlReader.parse(&xmlInput, true);
                newInformation = false;
                getDocument();
            }
        }
    } else {
        QString errorString(networkReply->readAll());
        errorString = errorString.right(errorString.length() - errorString.indexOf("Error=") - 6);
        emit userAuthenticated(false, errorString);
    }
}

void GoogleDocumentService::getDocument()
{
    QList<GoogleDocument *> gList = gHandler->documentList()->entries();
    if(gList.count() > 0) {
        emit showingDocumentList();
        documentList = new DocumentListWindow(this, gHandler->documentList());
    }
    else
        QMessageBox msgBox(QMessageBox::Information, i18n("Online Document Services"), i18n("No Documents Found !!!"));
}

void GoogleDocumentService::downloadDocument(const QString & _url, const QString & _type)
{
    authToken = docAuthToken;
    QString url = _url;
    QString type = _type;
    url.replace("docId", "docID", Qt::CaseInsensitive);
    QString exportFormat = "";

    if(QString::compare(type, "spreadsheet", Qt::CaseInsensitive) == 0) {
        exportFormat = "&exportFormat=ods&format=ods";
        authToken = spreadAuthToken;
    } else if(QString::compare(type, "presentation", Qt::CaseInsensitive) == 0) {
        exportFormat = "&exportFormat=ppt&format=ppt";
    }
    else if(QString::compare(type, "document", Qt::CaseInsensitive) == 0) {
        exportFormat = "&exportFormat=odt&format=odt";
    }

    qDebug() << "URL = " <<  url + exportFormat;
    QUrl documentUrl(url + exportFormat);

    QNetworkRequest requestHeader(documentUrl);
    requestHeader.setRawHeader("User-Agent", "Calligra");
    requestHeader.setRawHeader("GData-Version", "3.0");
    requestHeader.setRawHeader("Authorization", authToken.toUtf8());

    QList<QByteArray> headerlist = requestHeader.rawHeaderList();
    foreach (QByteArray element, headerlist)
        qDebug() << element << requestHeader.rawHeader(element);

    networkManager.get(requestHeader);

    waitingForDoc = true;

}

void GoogleDocumentService::hideDocumentListWindow()
{
    documentList->hide();
}
