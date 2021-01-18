/*
 *    This file is part of the KDE project
 *    SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *    SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *    SPDX-FileCopyrightText: 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *    SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *    SPDX-FileCopyrightText: 2011 José Luis Vergara <pentalis@gmail.com>
 *    SPDX-FileCopyrightText: 2013 Sascha Suelzer <s.suelzer@gmail.com>
 *    SPDX-FileCopyrightText: 2020 Agata Cacko <cacko.azh@gmail.com>
 *
 *    SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISTAGTOOLBUTTON_H
#define KISTAGTOOLBUTTON_H

#include <QWidget>
#include <KisTag.h>

class KisTagModel;

#include <KoResource.h>

/**
 * \brief The KisTagToolButton class manages the logic of the tag management popup.
 *
 * This class is responsible for the GUI for creating, renaming and removing tags.
 * Since both renaming and removing is context-dependent (it depends on which tag
 * is currently selected in the combobox), all actions emit signals to the TagChooserWidget
 * for it to handle actual creationg, renaming and removal of tags in the KisTagModel.
 */
class KisTagToolButton : public QWidget
{
    Q_OBJECT

private:
    explicit KisTagToolButton(QWidget* parent = 0);
    ~KisTagToolButton() override;

    ///
    /// \brief readOnlyMode sets the mode of the popup
    ///
    /// If the mode is read-only, then renaming and removal of the tag
    /// is not accessible (the textbox and the buttons are hidden).
    /// \param activate if true, then the popup is in the read-only mode.
    ///
    void readOnlyMode(bool activate);

    ///
    /// \brief setUndeletionCandidate sets a new item in the deleted tags list
    ///
    /// Tags are never deleted fully, they are only marked inactive.
    /// Undeletion means marking them as active again. This function
    /// adds new tags for the user to be able to undelete them (mark active in the database).
    /// \param deletedTag tag that can be undeleted (activated again)
    ///
    void setUndeletionCandidate(const KisTagSP deletedTag);

    void setCurrentTag(const KisTagSP tag);

    ///
    /// \brief updates icon file when loading and changing themes
    ///
    void loadIcon();

Q_SIGNALS:
    ///
    /// \brief newTagRequested signals to the KisTagChooserWidget to create a new tag
    /// \param tag tag name written by the user (other fields are not used)
    ///
    /// Since KisTagToolButton doesn't know which KisTagModel it should be using (because it doesn't
    /// know the resourceType) and for the consistency, it signals KisTagChooserWidget to create
    /// a new tag with the name written by the user.
    void newTagRequested(const QString &tagName);

    ///
    /// \brief renamingOfCurrentTagRequested signals to KisTagChooserWidget to rename the current tag
    /// \param tag tag name written by the user (other fields are not used)
    ///
    /// Since KisTagToolButton doesn't know which tag is current or which KisTagModel it should be using,
    /// it signals KisTagChooserWidget to do rename the current tag to the name written by the user.
    void renamingOfCurrentTagRequested(const QString &tagName);

    ///
    /// \brief deletionOfCurrentTagRequested signals to KisTagChooserWidget to delete the current tag
    ///
    /// Since KisTagToolButton doesn't know which tag is current or which KisTagModel it should be using,
    /// it signals KisTagChooserWidget to do remove the current tag.
    void deletionOfCurrentTagRequested();

    ///
    /// \brief undeletionOfTagRequested signals to KisTagChooserWidget to undelete the mentioned tag
    /// \param tag tag to be undeleted (marked active again)
    ///
    /// Tags are never deleted fully, they are only marked inactive.
    /// Undeletion means marking them as active again. This function signals to KisTagChooserWidget
    /// that a tag mentioned in the argument should be activated.
    void undeletionOfTagRequested(const KisTagSP tag);
    ///
    /// \brief popupMenuAboutToShow signals that the tags popup will be shown soon.
    ///
    /// It is used by \see KisTagChooserWidget
    ///
    void popupMenuAboutToShow();

private Q_SLOTS:
    ///
    /// \brief onTagUndeleteClicked is called when the user
    ///
    void onTagUndeleteClicked();

private:
    class Private;
    Private* const d;
    friend class KisTagChooserWidget;
};

#endif // KOTAGTOOLBUTTON_H
