/*
 *  Copyright (c) 2018 Jouni Pentikäinen <joupent@gmail.com>
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

#include "KisWindowLayoutManager.h"

#include <QWidget>

#include <KisApplication.h>
#include <KisMainWindow.h>

Q_GLOBAL_STATIC(KisWindowLayoutManager, s_instance)

struct KisWindowLayoutManager::Private {
    bool showImageInAllWindows{false};
    bool primaryWorkspaceFollowsFocus{false};
    QUuid primaryWindow;

    QString lastLayoutName;
};

KisWindowLayoutManager * KisWindowLayoutManager::instance()
{
    return s_instance;
}

KisWindowLayoutManager::KisWindowLayoutManager()
{
    connect(qobject_cast<KisApplication*>(KisApplication::instance()), SIGNAL(focusChanged(QWidget*, QWidget*)),
            this, SLOT(focusChanged(QWidget*, QWidget*)));
}

KisWindowLayoutManager::~KisWindowLayoutManager() {}

void KisWindowLayoutManager::setShowImageInAllWindowsEnabled(bool showInAll)
{
    d->showImageInAllWindows = showInAll;
}

bool KisWindowLayoutManager::isShowImageInAllWindowsEnabled() const
{
    return d->showImageInAllWindows;
}

bool KisWindowLayoutManager::primaryWorkspaceFollowsFocus() const
{
    return d->primaryWorkspaceFollowsFocus;
}

void KisWindowLayoutManager::setPrimaryWorkspaceFollowsFocus(bool enabled, QUuid primaryWindow)
{
    d->primaryWorkspaceFollowsFocus = enabled;
    d->primaryWindow = primaryWindow;
}

QUuid KisWindowLayoutManager::primaryWindowId() const
{
    return d->primaryWindow;
}
void KisWindowLayoutManager::focusChanged(QWidget *old, QWidget *now)
{
    Q_UNUSED(old);

    if (!now) return;
    KisMainWindow *newMainWindow = qobject_cast<KisMainWindow*>(now->window());
    if (!newMainWindow) return;

    newMainWindow->windowFocused();
}

QString KisWindowLayoutManager::lastLayoutName()
{
    return d->lastLayoutName;
}

void KisWindowLayoutManager::setLastLayoutName(const QString &name)
{
    d->lastLayoutName = name;
}
