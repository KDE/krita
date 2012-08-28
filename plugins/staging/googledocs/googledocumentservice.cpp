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
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QTimer>

GoogleDocumentService::GoogleDocumentService(OnlineDocument::DocumentType type)
          : newInformation(true)
          , waitingForDoc(false)
          , haveDocAuthToken(false)
          , documentList(0)
          , loggedin(false)
          , m_type(type)
{
    //QNetworkProxy::setApplicationProxy(QNetworkProxy(QNetworkProxy::HttpProxy, "proxy.jf.intel.com", 911));
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
    data.append(QString("accountType=HOSTED_OR_GOOGLE&Email=" + username + "&Passwd=" + password).toUtf8());

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
    QString url;
    switch (m_type) {
    case OnlineDocument::WORDS:
        url = "https://docs.google.com/feeds/default/private/full/-/document";
        break;
    case OnlineDocument::STAGE:
        url = "https://docs.google.com/feeds/default/private/full/-/presentation";
        break;
    case OnlineDocument::SHEETS:
        url = "https://docs.google.com/feeds/default/private/full/-/spreadsheet";
        break;
    }

    QNetworkRequest requestHeader(QUrl(url.toUtf8()));
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
    if (!networkReply->error()) {
        if (!loggedin) {
            QString text(networkReply->readAll());
            text = text.right(text.length() - text.indexOf("Auth=") - 5);
            authToken = QString("GoogleLogin auth=") + text.left(text.indexOf("\n"));
            if(authToken.length() > 20) {
                if(!haveDocAuthToken) {
                    docAuthToken = authToken;
                    haveDocAuthToken = true;
                    qDebug() << "Received Doc token = " << docAuthToken;
                    clientLogin(this->username, this->password);
                    return;
                }
                spreadAuthToken = authToken;
                authToken = "";
                loggedin = true;
                qDebug() << "Received Spreadsheet token = " << spreadAuthToken;
                listDocuments();
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
            showDocumentListWindow(false);
        }
        else {
            QByteArray bytAry = networkReply->readAll();
//            qDebug() << bytAry;

//            xmlInput.setData(networkReply->readAll());
            xmlInput.setData(bytAry);
            qDebug() << "Part received.........";
            if (newInformation) {
                emit progressUpdate("Parsing document list...");
                newInformation = xmlReader.parse(&xmlInput, true);
                qDebug() << "New Information = " << newInformation;
//                newInformation = false;
                getDocument();
            }
        }
    } else {
        QString errorString(networkReply->readAll());
        qDebug() << "Error occurred !!!!  " << errorString;
        errorString = errorString.right(errorString.length() - errorString.indexOf("Error=") - 6);
        if (!loggedin) {
            emit userAuthenticated(loggedin, errorString);
        } else {
            QMessageBox msgBox(QMessageBox::Information, i18n("Online Document Services"),
                               "Error occurred !!!!  " + errorString);
            msgBox.exec();
        }
    }
}

void GoogleDocumentService::getDocument()
{
    if(gHandler->documentList()->documentsCount() > 0) {
        emit showingDocumentList();
        documentList = new DocumentListWindow(this, gHandler->documentList());
    }
    else {
        QMessageBox msgBox(QMessageBox::Information, i18n("Online Document Services"), i18n("No Documents Found !!!"));
        msgBox.exec();
    }
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

void GoogleDocumentService::showDocumentListWindow(bool visible)
{
    if (visible) {
        if (documentList)
            documentList->show();
    } else {
        documentList->hideDialog();
    }
}
