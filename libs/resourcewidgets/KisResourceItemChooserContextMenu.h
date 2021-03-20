/*
 *    This file is part of the KDE project
 *    SPDX-FileCopyrightText: 2013 Sascha Suelzer <s.suelzer@gmail.com>
 *    SPDX-FileCopyrightText: 2019 Boudewijn Rempt <boud@valdyas.org>
 *    SPDX-FileCopyrightText: 2020 Agata Cacko <cacko.azh@gmail.com>
 *
 *    SPDX-License-Identifier: LGPL-2.0-or-later
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

#include "TagActions.h"
#include "KisTagChooserWidget.h"

///
/// \brief The KisResourceItemChooserContextMenu class is responsible for the context menu in ResourceItemChooser
///
/// The context menu for the resource item in the resource item chooser (see: main area in the Brush Presets docker)
/// contains actions to tag and untag the selected resource.
/// In case of tagging the user can choose to create a new tag or select one of the existing ones.
/// In case of untagging the user can untag from the current selected tag (in the combobox) or from some other tags.
/// This class needs to provide correct lists of tags and take into account that "All" and "All Untagged" (and possibly other
/// generated tags) are special and the user cannot untage the resource from it.
///
class KisResourceItemChooserContextMenu :  public QMenu
{
    Q_OBJECT
public:
    ///
    /// \brief KisResourceItemChooserContextMenu the constructor for the KisResourceItemChooserContextMenu class
    /// \param resource the resource that the context menu is called for
    /// \param currentlySelectedTag the currently selected tag in the combobox over the resource item chooser
    ///
    explicit KisResourceItemChooserContextMenu(KoResourceSP resource, const KisTagSP currentlySelectedTag, KisTagChooserWidget *tagChooser);
    /// \brief the destructor
    ~KisResourceItemChooserContextMenu() override;

Q_SIGNALS:

    /// Emitted when a resource should be added to an existing tag.
    void resourceTagAdditionRequested(const KisTagSP tag, KoResourceSP resource);

    /// Emitted when a resource should be removed from an existing tag.
    void resourceTagRemovalRequested(KoResourceSP resource, const KisTagSP tag);

    /// Emitted when a resource should be added to a new tag, which will need to be created.
    void resourceAssignmentToNewTagRequested(const QString &tag, KoResourceSP resource);


public Q_SLOTS:

    ///
    /// \brief removeResourceExistingTag slot for a signal from the action to remove the tag from the resource
    /// \param resource resource that the tag needs to be removed from
    /// \param tag tag that needs to be removed from the resource
    ///
    void removeResourceExistingTag(const KisTagSP tag, KoResourceSP resource);

private:
    ///
    /// \brief m_tagModel data model for tags (for tagging and untagging resources and create lists of tags)
    ///
    KisTagModel *m_tagModel {0};
    KisTagChooserWidget *m_tagChooserWidget {0};

};

#endif // KORESOURCEITEMCHOOSERCONTEXTMENU_H
