/*
 *  SPDX-FileCopyrightText: 2019 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "MockNetworkAccessManager.h"

#include <QNetworkReply>
#include <QIODevice>
#include <QMetaObject>


FakeNetworkReply::FakeNetworkReply(const QNetworkRequest& originalRequest, FakeReplyData &replyData)
    : QNetworkReply()
{
    setRequest(originalRequest);

    setUrl(replyData.url);
    setOperation(replyData.requestMethod);
    setAttribute(QNetworkRequest::HttpStatusCodeAttribute, replyData.statusCode);
    setHeader(QNetworkRequest::ContentTypeHeader, replyData.contentType);
    setHeader(QNetworkRequest::ContentLengthHeader, replyData.responseBody.size());

    m_data.setData(replyData.responseBody);

    m_data.open(QIODevice::ReadOnly);

    open(QIODevice::ReadOnly);
    setFinished(true);
    QMetaObject::invokeMethod(this, "readyRead", Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
}

void FakeNetworkReply::abort()
{
    return;
}

bool FakeNetworkReply::atEnd() const
{
    return m_data.atEnd();
}

qint64 FakeNetworkReply::bytesAvailable() const
{
    return m_data.bytesAvailable();
}

bool FakeNetworkReply::canReadLine() const
{
    return m_data.canReadLine();
}

void FakeNetworkReply::close()
{
    return m_data.close();
}

qint64 FakeNetworkReply::size() const
{
    return m_data.size();
}

qint64 FakeNetworkReply::pos() const
{
    return m_data.pos();
}

qint64 FakeNetworkReply::readData(char *data, qint64 maxLen)
{
    return m_data.read(data, maxLen);
}

qint64 FakeNetworkReply::writeData(const char *data, qint64 maxLen)
{
    return m_data.write(data, maxLen);
}

MockNetworkAccessManager::MockNetworkAccessManager(QObject *parent)
    : KisNetworkAccessManager(parent)
{
}

void MockNetworkAccessManager::setReplyData(FakeReplyData &replyData)
{
    m_replyData = replyData;
}

QNetworkReply *MockNetworkAccessManager::createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    Q_UNUSED(op);
    Q_UNUSED(outgoingData);

    return new FakeNetworkReply(request, m_replyData);
}
