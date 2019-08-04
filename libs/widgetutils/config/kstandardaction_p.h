/* This file is part of the KDE libraries
   Copyright (C) 1999,2000 Kurt Granroth <granroth@kde.org>

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

#ifndef KSTANDARDACTION_PRIVATE_H
#define KSTANDARDACTION_PRIVATE_H

#include <QAction>
#include <QApplication>

#include <klocalizedstring.h>
#include <kstandardshortcut.h>

namespace KStandardAction
{

struct KStandardActionInfo {
    StandardAction id;
    KStandardShortcut::StandardShortcut idAccel;
    const char *psName;
    const char *psLabel;
    const char *psToolTip;
    const char *psIconName;
};

static const KStandardActionInfo g_rgActionInfo[] = {
    { New,           KStandardShortcut::New, "file_new", I18N_NOOP("&New"), I18N_NOOP("Create new document"), "document-new" },
    { Open,          KStandardShortcut::Open, "file_open", I18N_NOOP("&Open..."), I18N_NOOP("Open an existing document"), "document-open" },
    { OpenRecent,    KStandardShortcut::AccelNone, "file_open_recent", I18N_NOOP("Open &Recent"), I18N_NOOP("Open a document which was recently opened"), "document-open-recent" },
    { Save,          KStandardShortcut::Save, "file_save", I18N_NOOP("&Save"), I18N_NOOP("Save document"), "document-save" },
    { SaveAs,        KStandardShortcut::SaveAs, "file_save_as", I18N_NOOP("Save &As..."), I18N_NOOP("Save document under a new name"), "document-save-as" },
    { Revert,        KStandardShortcut::Revert, "file_revert", I18N_NOOP("Re&vert"), I18N_NOOP("Revert unsaved changes made to document"), "document-revert" },
    { Close,         KStandardShortcut::Close, "file_close", I18N_NOOP("&Close"), I18N_NOOP("Close document"), "document-close" },
    { Print,         KStandardShortcut::Print, "file_print", I18N_NOOP("&Print..."), I18N_NOOP("Print document"), "document-print" },
    { PrintPreview,  KStandardShortcut::PrintPreview, "file_print_preview", I18N_NOOP("Print Previe&w"), I18N_NOOP("Show a print preview of document"), "document-print-preview" },
    { Mail,          KStandardShortcut::Mail, "file_mail", I18N_NOOP("&Mail..."), I18N_NOOP("Send document by mail"), "mail-send" },
    { Quit,          KStandardShortcut::Quit, "file_quit", I18N_NOOP("&Quit"), I18N_NOOP("Quit application"), "application-exit" },

    { Undo,          KStandardShortcut::Undo, "edit_undo", I18N_NOOP("&Undo"), I18N_NOOP("Undo last action"), "edit-undo" },
    { Redo,          KStandardShortcut::Redo, "edit_redo", I18N_NOOP("Re&do"), I18N_NOOP("Redo last undone action"), "edit-redo" },
    { Cut,           KStandardShortcut::Cut, "edit_cut", I18N_NOOP("Cu&t"), I18N_NOOP("Cut selection to clipboard"), "edit-cut" },
    { Copy,          KStandardShortcut::Copy, "edit_copy", I18N_NOOP("&Copy"), I18N_NOOP("Copy selection to clipboard"), "edit-copy" },
    { Paste,         KStandardShortcut::Paste, "edit_paste", I18N_NOOP("&Paste"), I18N_NOOP("Paste clipboard content"), "edit-paste" },
    { PasteText,     KStandardShortcut::Paste, "edit_paste", I18N_NOOP("&Paste"), I18N_NOOP("Paste clipboard content"), "edit-paste" },
    { Clear,         KStandardShortcut::Clear, "edit_clear", I18N_NOOP("C&lear"), 0, "edit-clear" },
    { SelectAll,     KStandardShortcut::SelectAll, "edit_select_all", I18N_NOOP("Select &All"), 0, "edit-select-all" },
    { Deselect,      KStandardShortcut::Deselect, "edit_deselect", I18N_NOOP("Dese&lect"), 0, 0 },
    { Find,          KStandardShortcut::Find, "edit_find", I18N_NOOP("&Find..."), 0, "edit-find" },
    { FindNext,      KStandardShortcut::FindNext, "edit_find_next", I18N_NOOP("Find &Next"), 0, "go-down-search" },
    { FindPrev,      KStandardShortcut::FindPrev, "edit_find_prev", I18N_NOOP("Find Pre&vious"), 0, "go-up-search" },
    { Replace,       KStandardShortcut::Replace, "edit_replace", I18N_NOOP("&Replace..."), 0, 0 },

    { ActualSize,    KStandardShortcut::ActualSize, "view_actual_size", I18N_NOOP("&Actual Size"), I18N_NOOP("View document at its actual size"), "zoom-original" },
    { FitToPage,     KStandardShortcut::FitToPage, "view_fit_to_page", I18N_NOOP("&Fit to Page"), I18N_NOOP("Zoom to fit page in window"), 0 },
    { FitToWidth,    KStandardShortcut::FitToWidth, "view_fit_to_width", I18N_NOOP("Fit to Page &Width"), I18N_NOOP("Zoom to fit page width in window"), 0 },
    { FitToHeight,   KStandardShortcut::FitToHeight, "view_fit_to_height", I18N_NOOP("Fit to Page &Height"), I18N_NOOP("Zoom to fit page height in window"), 0 },
    { ZoomIn,        KStandardShortcut::ZoomIn, "view_zoom_in", I18N_NOOP("Zoom &In"), 0, "zoom-in" },
    { ZoomOut,       KStandardShortcut::ZoomOut, "view_zoom_out", I18N_NOOP("Zoom &Out"), 0, "zoom-out" },
    { Zoom,          KStandardShortcut::Zoom, "view_zoom", I18N_NOOP("&Zoom..."), I18N_NOOP("Select zoom level"), 0 },
    { Redisplay,     KStandardShortcut::Reload, "view_redisplay", I18N_NOOP("&Redisplay"), I18N_NOOP("Redisplay document"), "view-refresh" },

    { Up,            KStandardShortcut::Up, "go_up", I18N_NOOP("&Up"), I18N_NOOP("Go up"), "go-up" },
    // The following three have special i18n() needs for sLabel
    { Back,          KStandardShortcut::Back, "go_back", 0, 0, "go-previous" },
    { Forward,       KStandardShortcut::Forward, "go_forward", 0, 0, "go-next" },
    { Home,          KStandardShortcut::Home, "go_home", 0, 0, "go-home" },
    { Prior,         KStandardShortcut::Prior, "go_previous", I18N_NOOP("&Previous Page"), I18N_NOOP("Go to previous page"), "go-previous-view-page" },
    { Next,          KStandardShortcut::Next, "go_next", I18N_NOOP("&Next Page"), I18N_NOOP("Go to next page"), "go-next-view-page" },
    { Goto,          KStandardShortcut::Goto, "go_goto", I18N_NOOP("&Go To..."), 0, 0 },
    { GotoPage,      KStandardShortcut::GotoPage, "go_goto_page", I18N_NOOP("&Go to Page..."), 0, "go-jump" },
    { GotoLine,      KStandardShortcut::GotoLine, "go_goto_line", I18N_NOOP("&Go to Line..."), 0, 0 },
    { FirstPage,     KStandardShortcut::Begin, "go_first", I18N_NOOP("&First Page"), I18N_NOOP("Go to first page"), "go-first-view-page" },
    { LastPage,      KStandardShortcut::End, "go_last", I18N_NOOP("&Last Page"), I18N_NOOP("Go to last page"), "go-last-view-page" },
    { DocumentBack,  KStandardShortcut::DocumentBack, "go_document_back", I18N_NOOP("&Back"), I18N_NOOP("Go back in document"), "go-previous" },
    { DocumentForward, KStandardShortcut::DocumentForward, "go_document_forward", I18N_NOOP("&Forward"), I18N_NOOP("Go forward in document"), "go-next" },

    { AddBookmark,   KStandardShortcut::AddBookmark, "bookmark_add", I18N_NOOP("&Add Bookmark"), 0, "bookmark-new" },
    { EditBookmarks, KStandardShortcut::EditBookmarks, "bookmark_edit", I18N_NOOP("&Edit Bookmarks..."), 0, "bookmarks-organize" },

    { Spelling,      KStandardShortcut::Spelling, "tools_spelling", I18N_NOOP("&Spelling..."), I18N_NOOP("Check spelling in document"), "tools-check-spelling" },

    { ShowMenubar,   KStandardShortcut::ShowMenubar, "options_show_menubar", I18N_NOOP("Show &Menubar"), I18N_NOOP("Show or hide menubar"), "show-menu" },
    { ShowToolbar,   KStandardShortcut::ShowToolbar, "options_show_toolbar", I18N_NOOP("Show &Toolbar"), I18N_NOOP("Show or hide toolbar"), 0 },
    { ShowStatusbar, KStandardShortcut::ShowStatusbar, "options_show_statusbar", I18N_NOOP("Show St&atusbar"), I18N_NOOP("Show or hide statusbar"), 0 },
    { FullScreen,    KStandardShortcut::FullScreen, "fullscreen", I18N_NOOP("F&ull Screen Mode"), 0, "view-fullscreen" },
    { SaveOptions,   KStandardShortcut::SaveOptions, "options_save_options", I18N_NOOP("&Save Settings"), 0, 0 },
    { KeyBindings,   KStandardShortcut::KeyBindings, "options_configure_keybinding", I18N_NOOP("Configure S&hortcuts..."), 0, "configure-shortcuts" },
    { Preferences,   KStandardShortcut::Preferences, "options_configure", I18N_NOOP("&Configure %1..."), 0, "configure" },
    { ConfigureToolbars, KStandardShortcut::ConfigureToolbars, "options_configure_toolbars", I18N_NOOP("Configure Tool&bars..."), 0, "configure-toolbars" },
    { ConfigureNotifications, KStandardShortcut::ConfigureNotifications, "options_configure_notifications", I18N_NOOP("Configure &Notifications..."), 0, "preferences-desktop-notification" },

    // the idea here is that Contents is used in menus, and Help in dialogs, so both share the same
    // shortcut
    { Help,          KStandardShortcut::Help, "help", 0, 0, "help-contents" },
    { HelpContents,  KStandardShortcut::Help, "help_contents", I18N_NOOP("%1 &Handbook"), 0, "help-contents" },
    { WhatsThis,     KStandardShortcut::WhatsThis, "help_whats_this", I18N_NOOP("What's &This?"), 0, "help-contextual" },
    { TipofDay,      KStandardShortcut::TipofDay, "help_show_tip", I18N_NOOP("Tip of the &Day"), 0, "help-hint" },
    { ReportBug,     KStandardShortcut::ReportBug, "help_report_bug", I18N_NOOP("&Report Bug..."), 0, "tools-report-bug" },
    { SwitchApplicationLanguage, KStandardShortcut::SwitchApplicationLanguage, "switch_application_language", I18N_NOOP("Switch Application &Language..."), 0, "preferences-desktop-locale" },
    { AboutApp,      KStandardShortcut::AccelNone, "help_about_app", I18N_NOOP("&About %1"), 0, 0 },
    { AboutKDE,      KStandardShortcut::AccelNone, "help_about_kde", I18N_NOOP("About &KDE"), 0, "kde" },
    { ActionNone,    KStandardShortcut::AccelNone, 0, 0, 0, 0 }
};

inline const KStandardActionInfo *infoPtr(StandardAction id)
{
    for (uint i = 0; g_rgActionInfo[i].id != ActionNone; i++) {
        if (g_rgActionInfo[i].id == id) {
            return &g_rgActionInfo[i];
        }
    }

    return 0;
}

static inline QStringList internal_stdNames()
{
    QStringList result;

    for (uint i = 0; g_rgActionInfo[i].id != ActionNone; i++)
        if (g_rgActionInfo[i].psLabel) {
            if (QByteArray(g_rgActionInfo[i].psLabel).contains("%1"))
                // Prevents i18n from complaining about unsubstituted placeholder.
            {
                result.append(i18n(g_rgActionInfo[i].psLabel, QString()));
            } else {
                result.append(i18n(g_rgActionInfo[i].psLabel));
            }
        }

    return result;
}

class AutomaticAction : public QAction
{
    Q_OBJECT

public:
    AutomaticAction(const QIcon &icon, const QString &text, const QList<QKeySequence> &shortcut, const char *slot,
                    QObject *parent);
public Q_SLOTS:
    inline void cut()
    {
        invokeEditSlot("cut");
    }
    inline void copy()
    {
        invokeEditSlot("copy");
    }
    inline void paste()
    {
        invokeEditSlot("paste");
    }
    inline void clear()
    {
        invokeEditSlot("clear");
    }
    inline void selectAll()
    {
        invokeEditSlot("selectAll");
    }

    void invokeEditSlot(const char *slot)
    {
        if (qApp->focusWidget()) {
            QMetaObject::invokeMethod(qApp->focusWidget(), slot);
        }
    }

};

}

#endif
