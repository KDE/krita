// Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
// SPDX-License-Identifier: BSD-3-Clause

#ifndef QTSINGLEAPPLICATION_H
#define QTSINGLEAPPLICATION_H

#include <kritaui_export.h>
#include <QApplication>

class QtLocalPeer;

class KRITAUI_EXPORT QtSingleApplication : public QApplication
{
    Q_OBJECT

public:
    QtSingleApplication(int &argc, char **argv, bool GUIenabled = true);
    QtSingleApplication(const QString &id, int &argc, char **argv);
#if QT_VERSION < 0x050000
    QtSingleApplication(int &argc, char **argv, Type type);
#  if defined(Q_WS_X11)
    QtSingleApplication(Display* dpy, Qt::HANDLE visual = 0, Qt::HANDLE colormap = 0);
    QtSingleApplication(Display *dpy, int &argc, char **argv, Qt::HANDLE visual = 0, Qt::HANDLE cmap= 0);
    QtSingleApplication(Display* dpy, const QString &appId, int argc, char **argv, Qt::HANDLE visual = 0, Qt::HANDLE colormap = 0);
#  endif // Q_WS_X11
#endif // QT_VERSION < 0x050000

    bool isRunning();
    QString id() const;

    void setActivationWindow(QWidget* aw, bool activateOnMessage = true);
    QWidget* activationWindow() const;

    // Obsolete:
    void initialize(bool dummy = true)
        { isRunning(); Q_UNUSED(dummy) }

public Q_SLOTS:
    bool sendMessage(const QString &message, int timeout = 5000);
    void activateWindow();


Q_SIGNALS:
    void messageReceived(const QString &message);


private:
    void sysInit(const QString &appId = QString());
    QtLocalPeer *peer;
    QWidget *actWin;
};

#endif // QTSINGLEAPPLICATION_H
