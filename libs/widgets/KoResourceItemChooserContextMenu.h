/*
 *    This file is part of the KDE project
 *    Copyright (c) 2013 Sascha Suelzer <s_suelzer@lavabit.com>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Library General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Library General Public License for more details.
 *
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to
 *    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  * Boston, MA 02110-1301, USA.
 * */

#ifndef KORESOURCEITEMCHOOSERCONTEXTMENU_H
#define KORESOURCEITEMCHOOSERCONTEXTMENU_H

#include <QMenu>
#include <QWidgetAction>
#include <KLineEdit>
class KoResource;

class ContextMenuExistingTagAction : public QAction
{
    Q_OBJECT
public:
    explicit ContextMenuExistingTagAction( KoResource * resource, QString tag, QObject* parent = 0);
    ~ContextMenuExistingTagAction();

signals:
    void triggered(KoResource * resource, QString tag);

protected slots:
    void onTriggered();

private:
    KoResource * m_resource;
    QString m_tag;
};

class ContextMenuNewTagAction : public QWidgetAction {
    Q_OBJECT
public:
    explicit ContextMenuNewTagAction (KoResource* resource, QObject* parent);
    ~ContextMenuNewTagAction();

    signals:
    void triggered(KoResource * resource, const QString &tag);

protected slots:
    void onTriggered(const QString& tagName);

private:
    KoResource * m_resource;
    QString m_tag;
    KLineEdit * m_editBox;
};

class KoResourceItemChooserContextMenu :  QMenu
{
    explicit KoResourceItemChooserContextMenu(QWidget* parent = 0);
};

#endif // KORESOURCEITEMCHOOSERCONTEXTMENU_H
