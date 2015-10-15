/*
    This file is part of the KDE libraries
     Copyright
     (C) 2000 Reginald Stadlbauer (reggie@kde.org)
     (C) 1997 Stephan Kulow (coolo@kde.org)
     (C) 1997-2000 Sven Radej (radej@kde.org)
     (C) 1997-2000 Matthias Ettrich (ettrich@kde.org)
     (C) 1999 Chris Schlaeger (cs@kde.org)
     (C) 2002 Joseph Wenninger (jowenn@kde.org)
     (C) 2005-2006 Hamish Rodda (rodda@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef KMAINWINDOW_P_H
#define KMAINWINDOW_P_H

#include <kconfiggroup.h>
#include <QPointer>
#include <QEventLoopLocker>

#define K_D(Class) Class##Private * const d = k_func()

class QObject;
class QSessionManager;
class QTimer;
class KHelpMenu;
class KMainWindow;

class KMainWindowPrivate
{
public:
    bool autoSaveSettings: 1;
    bool settingsDirty: 1;
    bool autoSaveWindowSize: 1;
    bool sizeApplied: 1;
    KConfigGroup autoSaveGroup;
    QTimer *settingsTimer;
    QTimer *sizeTimer;
    QRect defaultWindowSize;
    KHelpMenu *helpMenu;
    KMainWindow *q;
    QPointer<QObject> dockResizeListener;
    QString dbusName;
    bool letDirtySettings;
    QEventLoopLocker locker;

    // This slot will be called when the style KCM changes settings that need
    // to be set on the already running applications.
    void _k_slotSettingsChanged(int category);
    void _k_slotSaveAutoSaveSize();

    void init(KMainWindow *_q);
    void polish(KMainWindow *q);
    enum CallCompression {
        NoCompressCalls = 0,
        CompressCalls
    };
    void setSettingsDirty(CallCompression callCompression = NoCompressCalls);
    void setSizeDirty();
};

class KMWSessionManager : public QObject
{
    Q_OBJECT
public:
    KMWSessionManager();
    ~KMWSessionManager();

private Q_SLOTS:
    bool saveState(QSessionManager &);
};

#endif

