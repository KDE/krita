/*
 *  Copyright (c) 2018 Boudewijn Rempt <boud@valdyas.org>
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
