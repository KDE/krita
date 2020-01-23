/*
 *    This file is part of the KDE project
 *    Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>
 *    Copyright (c) 2019 Boudewijn Rempt <boud@valdyas.org>
 *    Copyright (c) 2020 Agata Cacko <cacko.azh@gmail.com>
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

#ifndef KISRESOURCEITEMCHOOSERCONTEXTMENU_H
#define KISRESOURCEITEMCHOOSERCONTEXTMENU_H

#include <QMenu>
#include <QWidgetAction>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include <KoResource.h>

#include <KisTag.h>
#include <KisTagModel.h>

class ContextMenuExistingTagAction : public QAction
{
    Q_OBJECT
public:
    explicit ContextMenuExistingTagAction( KoResourceSP resource, KisTagSP tag, QObject* parent = 0);
    ~ContextMenuExistingTagAction() override;

Q_SIGNALS:
    void triggered(KoResourceSP resource, KisTagSP tag);

protected Q_SLOTS:
    void onTriggered();

private:
    KoResourceSP m_resource;
    KisTagSP m_tag;
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
    ~KoLineEditAction() override;
    void setIcon(const QIcon &icon);
    void closeParentOnTrigger(bool closeParent);
    bool closeParentOnTrigger();
    void setPlaceholderText(const QString& clickMessage);
    void setText(const QString& text);
    void setVisible(bool showAction);

    Q_SIGNALS:
    void triggered(const KisTagSP tag);

protected Q_SLOTS:
    void onTriggered();

private:
    bool m_closeParentOnTrigger;
    QLabel * m_label;
    QLineEdit * m_editBox;
    QPushButton * m_AddButton;
};

class NewTagAction : public KoLineEditAction
{
    Q_OBJECT
public:
    explicit NewTagAction (KoResourceSP resource, QMenu* parent);
    ~NewTagAction() override;

    Q_SIGNALS:
    void triggered(KoResourceSP resource, const KisTagSP tag);

protected Q_SLOTS:
    void onTriggered(const KisTagSP tagName);

private:
    KoResourceSP m_resource;
};

class KisResourceItemChooserContextMenu :  public QMenu
{
    Q_OBJECT
public:
    explicit KisResourceItemChooserContextMenu(KoResourceSP resource, const KisTagSP currentlySelectedTag);
    ~KisResourceItemChooserContextMenu() override;

Q_SIGNALS:
    /// Emitted when a resource should be added to an existing tag.
    void resourceTagAdditionRequested(KoResourceSP resource, const KisTagSP tag);
    /// Emitted when a resource should be removed from an existing tag.
    void resourceTagRemovalRequested(KoResourceSP resource, const KisTagSP tag);
    /// Emitted when a resource should be added to a new tag, which will need to be created.
    void resourceAssignmentToNewTagRequested(KoResourceSP resource, const KisTagSP tag);


public Q_SLOTS:
    void addResourceTag(KoResourceSP resource, const KisTagSP tag);
    void removeResourceExistingTag(KoResourceSP resource, const KisTagSP tag);
    void addResourceNewTag(KoResourceSP resource, const KisTagSP tag);

private:
    QString m_resourceType;
    KisTagModel* m_tagModel;

};

#endif // KORESOURCEITEMCHOOSERCONTEXTMENU_H
