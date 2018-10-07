/*
 * This file is part of the KDE Libraries
 * Copyright (C) 1999-2000 Espen Sand (espen@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef KHELPMENU_H
#define KHELPMENU_H

#include <kritawidgetutils_export.h>

#include <QObject>
#include <QString>

class QMenu;
class QWidget;
class QAction;

class KAboutData;
class KHelpMenuPrivate;

/**
 * @short Standard %KDE help menu with dialog boxes.
 *
 * This class provides the standard %KDE help menu with the default "about"
 * dialog boxes and help entry.
 *
 * This class is used in KMainWindow so
 * normally you don't need to use this class yourself. However, if you
 * need the help menu or any of its dialog boxes in your code that is
 * not subclassed from KMainWindow you should use this class.
 *
 * The usage is simple:
 *
 * \code
 * mHelpMenu = new KHelpMenu( this, <someText> );
 * kmenubar->addMenu(mHelpMenu->menu() );
 * \endcode
 *
 * or if you just want to open a dialog box:
 *
 * \code
 * mHelpMenu = new KHelpMenu( this, <someText> );
 * connect( this, SIGNAL(someSignal()), mHelpMenu,SLOT(aboutKDE()));
 * \endcode
 *
 * IMPORTANT:
 * The first time you use KHelpMenu::menu(), a QMenu object is
 * allocated. Only one object is created by the class so if you call
 * KHelpMenu::menu() twice or more, the same pointer is returned. The class
 * will destroy the popupmenu in the destructor so do not delete this
 * pointer yourself.
 *
 * The KHelpMenu object will be deleted when its parent is destroyed but you
 * can delete it yourself if you want. The code below will always work.
 *
 * \code
 * MyClass::~MyClass()
 * {
 *   delete mHelpMenu;
 * }
 * \endcode
 *
 *
 * Using your own "about application" dialog box:
 *
 * The standard "about application" dialog box is quite simple. If you
 * need a dialog box with more functionality you must design that one
 * yourself. When you want to display the dialog, you simply need to
 * connect the help menu signal showAboutApplication() to your slot.
 *
 * \code
 * void MyClass::myFunc()
 * {
 *   ..
 *   KHelpMenu *helpMenu = new KHelpMenu( this );
 *   connect( helpMenu, SIGNAL(showAboutApplication()),
 *          this, SLOT(myDialogSlot()));
 *   ..
 * }
 *
 * void MyClass::myDialogSlot()
 * {
 *   <activate your custom dialog>
 * }
 * \endcode
 *
 * \image html khelpmenu.png "KDE Help Menu"
 *
 * KHelpMenu respects Kiosk settings (see the KAuthorized namespace in the
 * KConfig framework).  In particular, system administrators can disable items
 * on this menu using some subset of the following configuration:
 * @verbatim
   [KDE Action Restrictions][$i]
   actions/help_contents=false
   actions/help_whats_this=false
   actions/help_report_bug=false
   actions/switch_application_language=false
   actions/help_about_app=false
   actions/help_about_kde=false
   @endverbatim
 *
 * @author Espen Sand (espen@kde.org)
 */

class KRITAWIDGETUTILS_EXPORT KHelpMenu : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param parent The parent of the dialog boxes. The boxes are modeless
     *        and will be centered with respect to the parent.
     * @param aboutAppText User definable string that is used in the
     *        default application dialog box.
     * @param showWhatsThis Decides whether a "Whats this" entry will be
     *        added to the dialog.
     *
     */
    explicit KHelpMenu(QWidget *parent = 0, const QString &aboutAppText = QString(),
                       bool showWhatsThis = true);

    /**
     * Constructor.
     *
     * This alternative constructor is mainly useful if you want to
     * override the standard actions (aboutApplication(), aboutKDE(),
     * helpContents(), reportBug, and optionally whatsThis).
     *
     * @param parent The parent of the dialog boxes. The boxes are modeless
     *        and will be centered with respect to the parent.
     * @param aboutData User and app data used in the About app dialog
     * @param showWhatsThis Decides whether a "Whats this" entry will be
     *        added to the dialog.
     */
    KHelpMenu(QWidget *parent, const KAboutData &aboutData,
              bool showWhatsThis = true);

    /**
     * Destructor
     *
     * Destroys dialogs and the menu pointer retuned by menu
     */
    ~KHelpMenu() override;

    /**
     * Returns a popup menu you can use in the menu bar or where you
     * need it.
     *
     * The returned menu is configured with an icon, a title and
     * menu entries. Therefore adding the returned pointer to your menu
     * is enough to have access to the help menu.
     *
     * Note: This method will only create one instance of the menu. If
     * you call this method twice or more the same pointer is returned.
     */
    QMenu *menu();

    enum MenuId {
        menuHelpContents = 0,
        menuWhatsThis = 1,
        menuAboutApp = 2,
        menuAboutKDE = 3,
        menuReportBug = 4,
        menuSwitchLanguage = 5
    };

    /**
     * Returns the QAction * associated with the given parameter
     * Will return 0 pointers if menu() has not been called
     *
     * @param id The id of the action of which you want to get QAction *
     */
    QAction *action(MenuId id) const;

public Q_SLOTS:
    /**
     * Opens the help page for the application. The application name is
     * used as a key to determine what to display and the system will attempt
     * to open \<appName\>/index.html.
     */
    void appHelpActivated();

    /**
     * Activates What's This help for the application.
     */
    void contextHelpActivated();

    /**
     * Opens an application specific dialog box.
     *
     * The method will try to open the about box using the following steps:
     * - If the showAboutApplication() signal is connected, then it will be called.
     *   This means there is an application defined aboutBox.
     * - If the aboutData was set in the constructor a KAboutApplicationDialog will be created.
     * - Else a default about box using the aboutAppText from the constructor will be created.
     */
    void aboutApplication();

    /**
     * Opens the standard "About KDE" dialog box.
     */
    void aboutKDE();

    /**
     * Opens the standard "Report Bugs" dialog box.
     */
    void reportBug();

    /**
     * Opens the changing default application language dialog box.
     */
    void switchApplicationLanguage();

private Q_SLOTS:
    /**
     * Connected to the menu pointer (if created) to detect a delete
     * operation on the pointer. You should not delete the pointer in your
     * code yourself. Let the KHelpMenu destructor do the job.
     */
    void menuDestroyed();

    /**
     * Connected to the dialogs (about kde and bug report) to detect
     * when they are finished.
     */
    void dialogFinished();

    /**
     * This slot will delete a dialog (about kde or bug report) if the
     * dialog pointer is not zero and the dialog is not visible. This
     * slot is activated by a one shot timer started in dialogHidden
     */
    void timerExpired();

Q_SIGNALS:
    /**
     * This signal is emitted from aboutApplication() if no
     * "about application" string has been defined. The standard
     * application specific dialog box that is normally activated in
     * aboutApplication() will not be displayed when this signal
     * is emitted.
     */
    void showAboutApplication();

private:
    KHelpMenuPrivate *const d;
};

#endif
