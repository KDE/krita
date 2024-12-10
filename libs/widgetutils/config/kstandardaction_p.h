/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 1999, 2000 Kurt Granroth <granroth@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KSTANDARDACTION_PRIVATE_H
#define KSTANDARDACTION_PRIVATE_H

#include <QAction>
#include <QApplication>
#include <QWidget>

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


// clang-format off

static constexpr KStandardActionInfo g_rgActionInfo[] = {
    { New,           KStandardShortcut::New, "file_new", QT_TRANSLATE_NOOP("KStandardActions", "&New"), QT_TRANSLATE_NOOP("KStandardActions", "Create new document"), "document-new" },
    { Open,          KStandardShortcut::Open, "file_open", QT_TRANSLATE_NOOP("KStandardActions", "&Open…"), QT_TRANSLATE_NOOP("KStandardActions", "Open an existing document"), "document-open" },
    { OpenRecent,    KStandardShortcut::AccelNone, "file_open_recent", QT_TRANSLATE_NOOP("KStandardActions", "Open &Recent"), QT_TRANSLATE_NOOP("KStandardActions", "Open a document which was recently opened"), "document-open-recent" },
    { Save,          KStandardShortcut::Save, "file_save", QT_TRANSLATE_NOOP("KStandardActions", "&Save"), QT_TRANSLATE_NOOP("KStandardActions", "Save document"), "document-save" },
    { SaveAs,        KStandardShortcut::SaveAs, "file_save_as", QT_TRANSLATE_NOOP("KStandardActions", "Save &As…"), QT_TRANSLATE_NOOP("KStandardActions", "Save document under a new name"), "document-save-as" },
    { Revert,        KStandardShortcut::Revert, "file_revert", QT_TRANSLATE_NOOP("KStandardActions", "Re&vert"), QT_TRANSLATE_NOOP("KStandardActions", "Revert unsaved changes made to document"), "document-revert" },
    { Close,         KStandardShortcut::Close, "file_close", QT_TRANSLATE_NOOP("KStandardActions", "&Close"), QT_TRANSLATE_NOOP("KStandardActions", "Close document"), "document-close" },
    { Print,         KStandardShortcut::Print, "file_print", QT_TRANSLATE_NOOP("KStandardActions", "&Print…"), QT_TRANSLATE_NOOP("KStandardActions", "Print document"), "document-print" },
    { PrintPreview,  KStandardShortcut::PrintPreview, "file_print_preview", QT_TRANSLATE_NOOP("KStandardActions", "Print Previe&w"), QT_TRANSLATE_NOOP("KStandardActions", "Show a print preview of document"), "document-print-preview" },
    { Mail,          KStandardShortcut::Mail, "file_mail", QT_TRANSLATE_NOOP("KStandardActions", "&Mail…"), QT_TRANSLATE_NOOP("KStandardActions", "Send document by mail"), "mail-send" },
    { Quit,          KStandardShortcut::Quit, "file_quit", QT_TRANSLATE_NOOP("KStandardActions", "&Quit"), QT_TRANSLATE_NOOP("KStandardActions", "Quit application"), "application-exit" },

    { Undo,          KStandardShortcut::Undo, "edit_undo", QT_TRANSLATE_NOOP("KStandardActions", "&Undo"), QT_TRANSLATE_NOOP("KStandardActions", "Undo last action"), "edit-undo" },
    { Redo,          KStandardShortcut::Redo, "edit_redo", QT_TRANSLATE_NOOP("KStandardActions", "Re&do"), QT_TRANSLATE_NOOP("KStandardActions", "Redo last undone action"), "edit-redo" },
    { Cut,           KStandardShortcut::Cut, "edit_cut", QT_TRANSLATE_NOOP("KStandardActions", "Cu&t"), QT_TRANSLATE_NOOP("KStandardActions", "Cut selection to clipboard"), "edit-cut" },
    { Copy,          KStandardShortcut::Copy, "edit_copy", QT_TRANSLATE_NOOP("KStandardActions", "&Copy"), QT_TRANSLATE_NOOP("KStandardActions", "Copy selection to clipboard"), "edit-copy" },
    { Paste,         KStandardShortcut::Paste, "edit_paste", QT_TRANSLATE_NOOP("KStandardActions", "&Paste"), QT_TRANSLATE_NOOP("KStandardActions", "Paste clipboard content"), "edit-paste" },
    { Clear,         KStandardShortcut::Clear, "edit_clear", QT_TRANSLATE_NOOP("KStandardActions", "C&lear"), {}, "edit-clear" },
    { SelectAll,     KStandardShortcut::SelectAll, "edit_select_all", QT_TRANSLATE_NOOP("KStandardActions", "Select &All"), {}, "edit-select-all" },
    { Deselect,      KStandardShortcut::Deselect, "edit_deselect", QT_TRANSLATE_NOOP("KStandardActions", "Dese&lect"), {}, "edit-select-none" },
    { Find,          KStandardShortcut::Find, "edit_find", QT_TRANSLATE_NOOP("KStandardActions", "&Find…"), {}, "edit-find" },
    { FindNext,      KStandardShortcut::FindNext, "edit_find_next", QT_TRANSLATE_NOOP("KStandardActions", "Find &Next"), {}, "go-down-search" },
    { FindPrev,      KStandardShortcut::FindPrev, "edit_find_prev", QT_TRANSLATE_NOOP("KStandardActions", "Find Pre&vious"), {}, "go-up-search" },
    { Replace,       KStandardShortcut::Replace, "edit_replace", QT_TRANSLATE_NOOP("KStandardActions", "&Replace…"), {}, "edit-find-replace" },

    { ActualSize,    KStandardShortcut::ActualSize, "view_actual_size", QT_TRANSLATE_NOOP("KStandardActions", "Zoom to &Actual Size"), QT_TRANSLATE_NOOP("KStandardActions", "View document at its actual size"), "zoom-original" },
    { FitToPage,     KStandardShortcut::FitToPage, "view_fit_to_page", QT_TRANSLATE_NOOP("KStandardActions", "&Fit to Page"), QT_TRANSLATE_NOOP("KStandardActions", "Zoom to fit page in window"), "zoom-fit-page" },
    { FitToWidth,    KStandardShortcut::FitToWidth, "view_fit_to_width", QT_TRANSLATE_NOOP("KStandardActions", "Fit to Page &Width"), QT_TRANSLATE_NOOP("KStandardActions", "Zoom to fit page width in window"), "zoom-fit-width" },
    { FitToHeight,   KStandardShortcut::FitToHeight, "view_fit_to_height", QT_TRANSLATE_NOOP("KStandardActions", "Fit to Page &Height"), QT_TRANSLATE_NOOP("KStandardActions", "Zoom to fit page height in window"), "zoom-fit-height" },
    { ZoomIn,        KStandardShortcut::ZoomIn, "view_zoom_in", QT_TRANSLATE_NOOP("KStandardActions", "Zoom &In"), {}, "zoom-in" },
    { ZoomOut,       KStandardShortcut::ZoomOut, "view_zoom_out", QT_TRANSLATE_NOOP("KStandardActions", "Zoom &Out"), {}, "zoom-out" },
    { Zoom,          KStandardShortcut::Zoom, "view_zoom", QT_TRANSLATE_NOOP("KStandardActions", "&Zoom…"), QT_TRANSLATE_NOOP("KStandardActions", "Select zoom level"), "zoom" },
    { Redisplay,     KStandardShortcut::Reload, "view_redisplay", QT_TRANSLATE_NOOP("KStandardActions", "&Refresh"), QT_TRANSLATE_NOOP("KStandardActions", "Refresh document"), "view-refresh" },

    { Up,            KStandardShortcut::Up, "go_up", QT_TRANSLATE_NOOP("KStandardActions", "&Up"), QT_TRANSLATE_NOOP("KStandardActions", "Go up"), "go-up" },
    // The following three have special i18n() needs for sLabel
    { Back,          KStandardShortcut::Back, "go_back", {}, {}, "go-previous" },
    { Forward,       KStandardShortcut::Forward, "go_forward", {}, {}, "go-next" },
    { Home,          KStandardShortcut::Home, "go_home", {}, {}, "go-home" },
    { Prior,         KStandardShortcut::Prior, "go_previous", QT_TRANSLATE_NOOP("KStandardActions", "&Previous Page"), QT_TRANSLATE_NOOP("KStandardActions", "Go to previous page"), "go-previous-view-page" },
    { Next,          KStandardShortcut::Next, "go_next", QT_TRANSLATE_NOOP("KStandardActions", "&Next Page"), QT_TRANSLATE_NOOP("KStandardActions", "Go to next page"), "go-next-view-page" },
    { Goto,          KStandardShortcut::Goto, "go_goto", QT_TRANSLATE_NOOP("KStandardActions", "&Go To…"), {}, {} },
    { GotoPage,      KStandardShortcut::GotoPage, "go_goto_page", QT_TRANSLATE_NOOP("KStandardActions", "&Go to Page…"), {}, "go-jump" },
    { GotoLine,      KStandardShortcut::GotoLine, "go_goto_line", QT_TRANSLATE_NOOP("KStandardActions", "&Go to Line…"), {}, {} },
    { FirstPage,     KStandardShortcut::Begin, "go_first", QT_TRANSLATE_NOOP("KStandardActions", "&First Page"), QT_TRANSLATE_NOOP("KStandardActions", "Go to first page"), "go-first-view-page" },
    { LastPage,      KStandardShortcut::End, "go_last", QT_TRANSLATE_NOOP("KStandardActions", "&Last Page"), QT_TRANSLATE_NOOP("KStandardActions", "Go to last page"), "go-last-view-page" },
    { DocumentBack,  KStandardShortcut::DocumentBack, "go_document_back", QT_TRANSLATE_NOOP("KStandardActions", "&Back"), QT_TRANSLATE_NOOP("KStandardActions", "Go back in document"), "go-previous" },
    { DocumentForward, KStandardShortcut::DocumentForward, "go_document_forward", QT_TRANSLATE_NOOP("KStandardActions", "&Forward"), QT_TRANSLATE_NOOP("KStandardActions", "Go forward in document"), "go-next" },

    { AddBookmark,   KStandardShortcut::AddBookmark, "bookmark_add", QT_TRANSLATE_NOOP("KStandardActions", "&Add Bookmark"), {}, "bookmark-new" },
    { EditBookmarks, KStandardShortcut::EditBookmarks, "bookmark_edit", QT_TRANSLATE_NOOP("KStandardActions", "&Edit Bookmarks…"), {}, "bookmarks-organize" },

    { Spelling,      KStandardShortcut::Spelling, "tools_spelling", QT_TRANSLATE_NOOP("KStandardActions", "&Spelling…"), QT_TRANSLATE_NOOP("KStandardActions", "Check spelling in document"), "tools-check-spelling" },

    { ShowMenubar,   KStandardShortcut::ShowMenubar, "options_show_menubar", QT_TRANSLATE_NOOP("KStandardActions", "Show &Menubar"), QT_TRANSLATE_NOOP("KStandardActions", "Show or hide menubar"), "show-men" },
    { ShowToolbar,   KStandardShortcut::ShowToolbar, "options_show_toolbar", QT_TRANSLATE_NOOP("KStandardActions", "Show &Toolbar"), QT_TRANSLATE_NOOP("KStandardActions", "Show or hide toolbar"), {} },
    { ShowStatusbar, KStandardShortcut::ShowStatusbar, "options_show_statusbar", QT_TRANSLATE_NOOP("KStandardActions", "Show St&atusbar"), QT_TRANSLATE_NOOP("KStandardActions", "Show or hide statusbar"), {} },
    { FullScreen,    KStandardShortcut::FullScreen, "fullscreen", QT_TRANSLATE_NOOP("KStandardActions", "F&ull Screen Mode"), {}, "view-fullscreen" },
    { KeyBindings,   KStandardShortcut::KeyBindings, "options_configure_keybinding", QT_TRANSLATE_NOOP("KStandardActions", "Configure Keyboard S&hortcuts…"), {}, "configure-shortcuts" },
    { Preferences,   KStandardShortcut::Preferences, "options_configure", QT_TRANSLATE_NOOP("KStandardActions", "&Configure %1…"), {}, "configure" },
    { ConfigureToolbars, KStandardShortcut::ConfigureToolbars, "options_configure_toolbars", QT_TRANSLATE_NOOP("KStandardActions", "Configure Tool&bars…"), {}, "configure-toolbars" },
    { ConfigureNotifications, KStandardShortcut::ConfigureNotifications, "options_configure_notifications", QT_TRANSLATE_NOOP("KStandardActions", "Configure &Notifications…"), {}, "preferences-desktop-notification" },

    // the idea here is that Contents is used in menus, and Help in dialogs, so both share the same
    // shortcut
    { HelpContents,  KStandardShortcut::Help, "help_contents", QT_TRANSLATE_NOOP("KStandardActions", "%1 &Handbook"), {}, "help-contents" },
    { WhatsThis,     KStandardShortcut::WhatsThis, "help_whats_this", QT_TRANSLATE_NOOP("KStandardActions", "What's &This?"), {}, "help-contextual" },
    { ReportBug,     KStandardShortcut::ReportBug, "help_report_bug", QT_TRANSLATE_NOOP("KStandardActions", "&Report Bug…"), {}, "tools-report-bug" },
    { SwitchApplicationLanguage, KStandardShortcut::SwitchApplicationLanguage, "switch_application_language", QT_TRANSLATE_NOOP("KStandardActions", "Configure &Language…"), {}, "preferences-desktop-locale" },
    { AboutApp,      KStandardShortcut::AccelNone, "help_about_app", QT_TRANSLATE_NOOP("KStandardActions", "&About %1"), {}, nullptr },
    { AboutKDE,      KStandardShortcut::AccelNone, "help_about_kde", QT_TRANSLATE_NOOP("KStandardActions", "About &KDE"), {}, "kde" },
    { ActionNone,    KStandardShortcut::AccelNone, {}, {}, {}, {} },
    };


// clang-format on

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
