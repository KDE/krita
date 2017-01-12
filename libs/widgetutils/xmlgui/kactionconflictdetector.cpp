/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>

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

#include <QAction>
#include <QCoreApplication>
#include <QShortcutEvent>

#include <klocalizedstring.h>
#include <kmessagebox.h>

class KActionConflictDetector : public QObject
{
public:
    explicit KActionConflictDetector(QObject *parent = 0)
        : QObject(parent)
    {
    }

    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (qobject_cast<QAction *>(watched) && (event->type() == QEvent::Shortcut)) {
            QShortcutEvent *se = static_cast<QShortcutEvent *>(event);
            if (se->isAmbiguous()) {
                KMessageBox::information(
                    0,  // No widget to be seen around here
                    i18n("The key sequence '%1' is ambiguous. Use the 'Keyboard Shortcuts'\n"
                         "tab in 'Settings->Configure Krita...' dialog to solve the ambiguity.\n"
                         "No action will be triggered.",
                         se->key().toString(QKeySequence::NativeText)),
                    i18n("Ambiguous shortcut detected"));
                return true;
            }
        }

        return QObject::eventFilter(watched, event);
    }
};

void _k_installConflictDetector()
{
    QCoreApplication *app = QCoreApplication::instance();
    app->installEventFilter(new KActionConflictDetector(app));
}

Q_COREAPP_STARTUP_FUNCTION(_k_installConflictDetector)
