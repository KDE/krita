/*
 *  SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _LOGDOCKER_DOCK_H_
#define _LOGDOCKER_DOCK_H_

#include <QDockWidget>

#include <kis_mainwindow_observer.h>

#include "ui_WdgLogDocker.h"
class MessageSender : public QObject
{
      Q_OBJECT
public:

    MessageSender() : QObject() {}
    ~MessageSender() override {}

    void sendMessage(QtMsgType type, const QString &msg);

Q_SIGNALS:

    void emitMessage(QtMsgType type, const QString &msg);

};

class LogDockerDock : public QDockWidget, public KisMainwindowObserver, public Ui_WdgLogDocker {
    Q_OBJECT
public:
    LogDockerDock( );
    QString observerName() override { return "LogDockerDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override {}
    void setViewManager(KisViewManager* kisview) override;

private Q_SLOTS:

    void toggleLogging(bool toggle);
    void clearLog();
    void saveLog();
    void settings();
    void insertMessage(QtMsgType type, const QString &msg);
    void changeTheme();

private:

    void applyCategories();

    static MessageSender *s_messageSender;
    static QTextCharFormat s_debug;
    static QTextCharFormat s_info;
    static QTextCharFormat s_warning;
    static QTextCharFormat s_critical;
    static QTextCharFormat s_fatal;
    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

};


#endif
