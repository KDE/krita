/* This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Nicolas Hadacek <hadacek@kde.org>
    SPDX-FileCopyrightText: 2001 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>
    SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KISSHORTCUTSDIALOG_H
#define KISSHORTCUTSDIALOG_H

#include <QDialog>

#include <KisShortcutsEditor.h>

// Altering this class and some classes it uses was one major impetus behind
// forking XmlGui. The first major workaround was to allow
// KisPart::configureShortcuts() to pull up the dialog, and to remote the scheme
// editor support, since it's incompatible with Krita.
//
// The files were forked from KF5 XmlGui version 5.12.0
//     dialogs/KisShortcutsEditorItem.cpp       <- kshortcutseditoritem.cpp
//     dialogs/KisShortcutEditWidget.cpp        <- kshortcuteditwidget.cpp
//     dialogs/KisShortcutsEditorDelegate.cpp   <- kshortcutseditordelegate.cpp
//     dialogs/KisShortcutsDialog.cpp           <- kshortcutsdialog.cpp, , kshortcutsdialog_p.cpp
//     dialogs/KisShortcutsDialog.h             <- kshortcutsdialog.h
//     dialogs/KisShortcutsDialog_p.h           <- kshortcutsdialog_p.h, kshortcutseditor_p.h
//     forms/KisShortcutsDialog.ui              <- kshortcutsdialog.ui
//
//
// Changes that have been done to the files:
// * Adapt of includes
// * Removing unwanted parts related to schemes
// * Renamed KShortcutsDialog/Editor to KisShortcutsDialog/Editor
// * Add export macro
// * Split apart kshortcutseditor_p
// * Copied KShortcutsEditorPrivate::itemFromIndex() implementation from
//   KF5 XmlGui's kshortcutseditor.cpp to begin of KisShortcutsEditorItem.cpp

/**
 * @short Dialog for configuration of KActionCollection and KGlobalAccel.
 *
 * The KisShortcutsDialog class is used for configuring dictionaries of
 * key/action associations for KActionCollection and KGlobalAccel. It uses the
 * KShortcutsEditor widget and offers buttons to set all keys to defaults and
 * invoke on-line help.
 *
 * Several static methods are supplied which provide the most convenient
 * interface to the dialog. The most common and most encouraged use is with
 * KActionCollection.
 *
 * \code
 * KisShortcutsDialog::configure( actionCollection() );
 * \endcode
 *
 * @since 4.3 By default this dialog is modal. If you don't want that,
 * setModal(false) and then the non-static configure() will show the dialog. If
 * you want to do anything extra when the dialog is done, connect to okClicked()
 * and/or cancelClicked(). However, if your extra stuff depends on the changed
 * settings already being saved, connect to saved() instead to be safe; if you
 * connect to okClicked() your function might be called before the save happens.
 *
 * example:
 * \code
 * KisShortcutsDialog dlg;
 * dlg.addCollection(myActions);
 * dlg.setModal(false);
 * connect(&dlg, SIGNAL(saved()), this, SLOT(doExtraStuff()));
 * dlg.configure();
 * \endcode
 *
 * \image html kshortcutsdialog.png "KDE Shortcuts Dialog"
 *
 * @author Nicolas Hadacek <hadacek@via.ecp.fr>
 * @author Hamish Rodda <rodda@kde.org> (KDE 4 porting)
 * @author Michael Jansen <kde@michael-jansen.biz>
 */

const auto defaultActionTypes = KisShortcutsEditor::WidgetAction \
                                | KisShortcutsEditor::WindowAction  \
                                | KisShortcutsEditor::ApplicationAction;

class KRITAWIDGETUTILS_EXPORT KisShortcutsDialog : public QWidget
{
    Q_OBJECT

public:
    /**
     * Constructs a KisShortcutsDialog. Mostly UI boilerplate.
     *
     * @param types the action types 
     * @param allowLetterShortcuts set to KisShortcutsEditor::LetterShortcutsDisallowed if unmodified alphanumeric
     *  keys ('A', '1', etc.) are not permissible shortcuts.
     *
     * @param parent the parent widget to attach to
     *
     * There is some legacy support for global (i.e. desktop-wide) shortucts
     * that should probably be removed.
     */
    explicit KisShortcutsDialog(KisShortcutsEditor::ActionTypes types = defaultActionTypes,
                                KisShortcutsEditor::LetterShortcuts allowLetterShortcuts \
                                = KisShortcutsEditor::LetterShortcutsAllowed,
                                QWidget *parent = 0);

    ~KisShortcutsDialog() override;

    /**
     * Add all actions of the collection to the ones displayed and configured
     * by the dialog. This is where the business happens.
     *
     * @param collection the action collection.
     * @param title the title associated with the collection.
     */
    void addCollection(KActionCollection *, const QString &title = QString());

    /**
     * @return the list of action collections that are available for configuration in the dialog.
     */
    QList<KActionCollection *> actionCollections() const;

    /** @see QWidget::sizeHint() */
    QSize sizeHint() const override;


    /**
     *  Called when the "OK" button in the main configuration page is pressed.
     */
    void save();
    void allDefault();
    void undo();

    /**
     * Import shortcut scheme file from @p path
     */
    void importConfiguration(const QString &path);

    /**
     * Exports shortcut scheme file to @p path
     */
    void exportConfiguration(const QString &path) const;

    /**
     * Import custom shortcuts from @p path
     */
    void loadCustomShortcuts(const QString &path);

    /**
     * Exports custom shortcuts to @p path
     */
    void saveCustomShortcuts(const QString &path) const;

private:
    Q_PRIVATE_SLOT(d, void changeShortcutScheme(const QString &))
    Q_PRIVATE_SLOT(d, void undo())

    class KisShortcutsDialogPrivate;
    class KisShortcutsDialogPrivate *const d;

    Q_DISABLE_COPY(KisShortcutsDialog)
};

#endif // KISSHORTCUTSDIALOG_H

