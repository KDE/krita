/*
 *    This file is part of the KDE project
 *    Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>
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
#include <QLabel>
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

/*!
 *  A line edit QWidgetAction.
 *  Default behavior: Closes its parent upon triggering.
 */
class KoLineEditAction : public QWidgetAction
{
    Q_OBJECT
public:
    explicit KoLineEditAction(QObject* parent);
    virtual ~KoLineEditAction();
    void setIcon(QIcon icon);
    void closeParentOnTrigger(bool closeParent);
    bool closeParentOnTrigger();
    void setClickMessage(const QString& clickMessage);
    void setText(const QString& text);

    signals:
    void triggered(const QString &tag);

protected slots:
    void onTriggered(const QString& text);

private:
    bool m_closeParentOnTrigger;
    QLabel * m_label;
    KLineEdit * m_editBox;
};

class NewTagAction : public KoLineEditAction
{
    Q_OBJECT
public:
    explicit NewTagAction (KoResource* resource, QMenu* parent);
    ~NewTagAction();

    signals:
    void triggered(KoResource * resource, const QString &tag);

protected slots:
    void onTriggered(const QString& tagName);

private:
    KoResource * m_resource;
};

class KoResourceItemChooserContextMenu :  public QMenu
{
    Q_OBJECT
public:
    explicit KoResourceItemChooserContextMenu
    (
        KoResource* resource,
        const QStringList& resourceTags,
        const QString& currentlySelectedTag,
        const QStringList& allTags
    );
    virtual ~KoResourceItemChooserContextMenu();

signals:
    /// Emitted when a resource should be added to an existing tag.
    void resourceTagAdditionRequested(KoResource* resource, const QString& tag);
    /// Emitted when a resource should be removed from an existing tag.
    void resourceTagRemovalRequested(KoResource* resource, const QString& tag);
    /// Emitted when a resource should be added to a new tag, which will need to be created.
    void resourceAssignmentToNewTagRequested(KoResource* resource, const QString& tag);

};

#endif // KORESOURCEITEMCHOOSERCONTEXTMENU_H
