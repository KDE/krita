// Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
// SPDX-License-Identifier: BSD-3-Clause

#ifndef QTLOCALPEER_H
#define QTLOCALPEER_H

#include <QLocalServer>
#include <QLocalSocket>
#include <QDir>
#include <QVector>

#include "qtlockedfile.h"

class QtLocalPeer : public QObject
{
    Q_OBJECT

public:
    QtLocalPeer(QObject *parent = 0, const QString &appId = QString());
    bool isClient();
    bool sendMessage(const QString &message, int timeout);
    QString applicationId() const
        { return id; }
    void setReady();

Q_SIGNALS:
    void sigMessageReceivedInternal(const QString &message);
    void messageReceived(const QString &message);

protected Q_SLOTS:
    void receiveConnection();

protected:
    QString id;
    QString socketName;
    QLocalServer* server;
    QtLP_Private::QtLockedFile lockFile;

private Q_SLOTS:
    void slotHandleMessageInternal(const QString &message);

private:
    static const char* ack;
    QVector<QString> queuedMessages;
    bool ready = false;
};

#endif // QTLOCALPEER_H
