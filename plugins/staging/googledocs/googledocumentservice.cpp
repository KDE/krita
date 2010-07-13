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

#include <QtGui>
#include <QtNetwork>
#include <QMessageBox>

#include "googledocumentservice.h"
#include "googledocumentlist.h"
#include "googledocument.h"
#include "googlecontenthandler.h"

const QString GoogleDocumentService::GOOGLE_DOCUMENT_URL = "docs.google.com";
const QString GoogleDocumentService::GOOGLE_SPREADSHEET_URL = "spreadsheets.google.com";

GoogleDocumentService::GoogleDocumentService()
        : newInformation(true)
          , waitingForDoc(false)
          , loggedin(false)
{
    connect(&networkManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(handleNetworkData(QNetworkReply*)));

    gHandler = new GoogleContentHandler();
    xmlReader.setContentHandler(gHandler);
}

void GoogleDocumentService::clientLogin(const QString & username, const QString & password)
{
    QByteArray data;
    data.append(QString("Email="+username+"&Passwd="+password).toUtf8());
    data.append(QString("&service=writely&source=KOfficev2").toUtf8());

    QNetworkRequest req(QUrl("https://www.google.com/accounts/ClientLogin"));
    req.setRawHeader("Host", "www.google.com");
    req.setRawHeader("GData-Version", "3.0");
    req.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    req.setHeader(QNetworkRequest::ContentLengthHeader, data.length());

    networkManager.post(req, data);
}

void GoogleDocumentService::listDocuments()
{
    QNetworkRequest requestHeader(QUrl("https://docs.google.com/feeds/default/private/full"));
    requestHeader.setRawHeader("Host", "docs.google.com");
    requestHeader.setRawHeader("User-Agent", "KOffice");
    requestHeader.setRawHeader("GData-Version", "3.0");
    requestHeader.setRawHeader("Content-Type", "application/atom+xml");
    requestHeader.setRawHeader("Authorization", authToken.toUtf8());

    networkManager.get(requestHeader);
}

void GoogleDocumentService::handleNetworkData(QNetworkReply *networkReply)
{
    QUrl url = networkReply->url();
    bool ok = false;
    if (!networkReply->error()) {
        if (!loggedin) {
            QByteArray m_data = networkReply->readAll();
            qDebug() << m_data;
            QString text(m_data.data());
            text = text.right(text.length()-text.indexOf("Auth=")-5);
            authToken = QString("GoogleLogin auth=")+text.left(text.indexOf("\n"));
            if (authToken.length() > 20) {
                loggedin = true;
                emit userAuthenticated(loggedin);
                listDocuments();
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
            /*QTemporaryFile *file = new QTemporaryFile(documentList->currentDocument());
            file->open();
            file->write(data);
            file->close();
            qDebug() << "Received Document!!!!! " << QDir::tempPath() + "/" + file->fileName();
            emit receivedDocument(QDir::tempPath() + "/" + file->fileName());*/
            waitingForDoc = false;
            hideDocumentListWindow();
        }
        else {
            xmlInput.setData(networkReply->readAll());
            qDebug() << "Part received.........";
             if (newInformation) {
                 ok = xmlReader.parse(&xmlInput, true);
                 newInformation = false;
                 getDocument();
             }
             //else
             //    ok = xmlReader.parseContinue();

        }
    }
    else
        qDebug() << networkReply->readAll();

    networkReply->deleteLater();
}

void GoogleDocumentService::getDocument()
{
    QList<GoogleDocument *> gList = gHandler->documentList()->entries();
    if(gList.count() > 0)
        documentList = new DocumentListWindow(this, gList);
    else
        QMessageBox msgBox(QMessageBox::NoIcon, "Office Viewer", "No Documents Found !!!");
}

void GoogleDocumentService::downloadDocument(GoogleDocument *doc)
{
    QString url = doc->documentUrl();
    QString type = doc->documentType();
    QString hostName = GOOGLE_DOCUMENT_URL;
    url.replace("docId", "docID", Qt::CaseInsensitive);
    QString exportFormat = "&exportFormat=odt";

    if (QString::compare(type, "spreadsheet") == 0 ) {
        hostName = GOOGLE_SPREADSHEET_URL;
        exportFormat = "&exportFormat=xls";
        url.replace("https://", "http://", Qt::CaseInsensitive);
    }
    else if (QString::compare(type, "presentation") == 0 )
        exportFormat = "&exportFormat=ppt";

    qDebug() << "URL = " <<  url + exportFormat;
    QUrl documentUrl(url);

    QNetworkRequest requestHeader(documentUrl);
    requestHeader.setRawHeader("Host", hostName.toUtf8());
    requestHeader.setRawHeader("User-Agent", "KOffice");
    requestHeader.setRawHeader("GData-Version", "3.0");
    requestHeader.setRawHeader("Content-Type", "application/atom+xml");
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
    delete documentList;
    documentList = 0;
}
