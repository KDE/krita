/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2001 Ian Reinhart Geiser <geiseri@yahoo.com>
   SPDX-FileCopyrightText: 2006 Thiago Macieira <thiago@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kmainwindowiface_p.h"

#include "kactioncollection.h"
#include "kxmlguiwindow.h"

#include <QApplication>
#include <QAction>
#include <QClipboard>

KisKMainWindowInterface::KisKMainWindowInterface(KXmlGuiWindow *mainWindow)
    : QDBusAbstractAdaptor(mainWindow)
{
    m_MainWindow = mainWindow;
}

KisKMainWindowInterface::~KisKMainWindowInterface()
{
}

QStringList KisKMainWindowInterface::actions()
{
    QStringList tmp_actions;
    QList<QAction *> lst = m_MainWindow->actionCollection()->actions();
    foreach (QAction *it, lst) {
        if (it->associatedWidgets().count() > 0) {
            tmp_actions.append(it->objectName());
        }
    }
    return tmp_actions;
}

bool KisKMainWindowInterface::activateAction(const QString &action)
{
    QAction *tmp_Action = m_MainWindow->actionCollection()->action(action);
    if (tmp_Action) {
        tmp_Action->trigger();
        return true;
    } else {
        return false;
    }
}

bool KisKMainWindowInterface::disableAction(const QString &action)
{
    QAction *tmp_Action = m_MainWindow->actionCollection()->action(action);
    if (tmp_Action) {
        tmp_Action->setEnabled(false);
        return true;
    } else {
        return false;
    }
}

bool KisKMainWindowInterface::enableAction(const QString &action)
{
    QAction *tmp_Action = m_MainWindow->actionCollection()->action(action);
    if (tmp_Action) {
        tmp_Action->setEnabled(true);
        return true;
    } else {
        return false;
    }
}

bool KisKMainWindowInterface::actionIsEnabled(const QString &action)
{
    QAction *tmp_Action = m_MainWindow->actionCollection()->action(action);
    if (tmp_Action) {
        return tmp_Action->isEnabled();
    } else {
        return false;
    }
}

QString KisKMainWindowInterface::actionToolTip(const QString &action)
{
    QAction *tmp_Action = m_MainWindow->actionCollection()->action(action);
    if (tmp_Action) {
        return tmp_Action->toolTip();
    } else {
        return QStringLiteral("Error no such object!");
    }
}

qlonglong KisKMainWindowInterface::winId()
{
    return qlonglong(m_MainWindow->winId());
}

void KisKMainWindowInterface::grabWindowToClipBoard()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setPixmap(m_MainWindow->grab());
}

#include "moc_kmainwindowiface_p.cpp"
