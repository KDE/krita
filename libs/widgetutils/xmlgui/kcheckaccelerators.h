/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
    Copyright (C) 1998, 1999, 2000 KDE Team
    Copyright (C) 2008 Nick Shaforostoff <shaforostoff@kde.ru>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
        */

#ifndef KCHECKACCELERATORS_H_
#define KCHECKACCELERATORS_H_

#include <QObject>
#include <QPointer>

#include <QTimer>

class QDialog;
class QTextBrowser;

/**
 @internal
 This class allows translators (and application developers) to check for accelerator
 conflicts in menu and widgets. Put the following in your kdeglobals (or the config
 file for the application you're testing):

 \code
 [Development]
 CheckAccelerators=F12
 AutoCheckAccelerators=false
 AlwaysShowCheckAccelerators=false
 \endcode

 The checking can be either manual or automatic. To perform manual check, press
 the keyboard shortcut set to 'CheckAccelerators' (here F12). If automatic checking
 is enabled by setting 'AutoCheckAccelerators' to true, check will be performed every
 time the GUI changes. It's possible that in certain cases the check will be
 done also when no visible changes in the GUI happen or the check won't be done
 even if the GUI changed (in the latter case, use manual check ). Automatic
 checks can be anytime disabled by the checkbox in the dialog presenting
 the results of the check. If you set 'AlwaysShowCheckAccelerators' to true,
 the dialog will be shown even if the automatic check didn't find any conflicts,
 and all submenus will be shown, even those without conflicts.

 The dialog first lists the name of the window, then all results for all menus
 (if the window has a menubar) and then result for all controls in the active
 window (if there are any checkboxes etc.). For every submenu and all controls
 there are shown all conflicts grouped by accelerator, and a list of all used
 accelerators.

 COPY WIDGET TEXT:

 You can copy widgets' texts to find them in translation files faster by middle-clicking them.
 Put the following lines in ~/.config/kdeglobals (or in rc file for specific app):

 \code
 [Development]
 CopyWidgetText=true
 CopyWidgetTextCommand=find_in_po_script "%1" "%2"
 \endcode

 Where %1 gets replaced with program name and %2 - with text.
 CopyWidgetTextCommand may be empty, in which case the text gets copied to clipboard.
 Press Ctrl+MMB to get widget text w/o accelerator mark (&)

*/

class KCheckAccelerators: public QObject
{
    Q_OBJECT
public:
    KCheckAccelerators(QObject *parent, int key, bool autoCheck, bool copyWidgetText);
    /**
     * Re-implemented to filter the parent's events.
     */
    bool eventFilter(QObject *, QEvent *e) Q_DECL_OVERRIDE;

private:
    void checkAccelerators(bool automatic);
    int key;
    bool block;
    bool alwaysShow;
    bool autoCheck;

    bool copyWidgetText;
    QString copyWidgetTextCommand;

    QTimer autoCheckTimer;
    void createDialog(QWidget *parent, bool automatic);
    QPointer<QDialog> drklash;
    QTextBrowser *drklash_view;

private Q_SLOTS:
    void autoCheckSlot();
    void slotDisableCheck(bool);
};

#endif
