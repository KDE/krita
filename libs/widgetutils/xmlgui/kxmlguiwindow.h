/*
    This file is part of the KDE libraries
     Copyright
     (C) 2000 Reginald Stadlbauer (reggie@kde.org)
     (C) 1997 Stephan Kulow (coolo@kde.org)
     (C) 1997-2000 Sven Radej (radej@kde.org)
     (C) 1997-2000 Matthias Ettrich (ettrich@kde.org)
     (C) 1999 Chris Schlaeger (cs@kde.org)
     (C) 2002 Joseph Wenninger (jowenn@kde.org)
     (C) 2005-2006 Hamish Rodda (rodda@kde.org)

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

#ifndef KXMLGUIWINDOW_H
#define KXMLGUIWINDOW_H

#include "kxmlguiclient.h"
#include "kxmlguibuilder.h"
#include "kmainwindow.h"
#include <QMetaClassInfo>

class KMenu;
class KXMLGUIFactory;
class KConfig;
class KConfigGroup;
class KToolBar;
class KXmlGuiWindowPrivate;

#define KDE_DEFAULT_WINDOWFLAGS 0

/**
 * @short %KDE top level main window with predefined action layout
 *
 * Instead of creating a KMainWindow manually and assigning menus, menu entries,
 * toolbar buttons and actions to it by hand, this class can be used to load an
 * rc file to manage the main window's actions.
 *
 * See http://techbase.kde.org/Development/Tutorials/Using_KActions#XMLGUI
 * for essential information on the XML file format and usage of this class.
 *
 * @see KMainWindow
 * @author Reginald Stadlbauer (reggie@kde.org) Stephan Kulow (coolo@kde.org), Matthias Ettrich (ettrich@kde.org), Chris Schlaeger (cs@kde.org), Sven Radej (radej@kde.org). Maintained by Sven Radej (radej@kde.org)

 */

class KRITAWIDGETUTILS_EXPORT KXmlGuiWindow : public KMainWindow, public KXMLGUIBuilder, virtual public KXMLGUIClient
{
    XMLGUI_DECLARE_PRIVATE(KXmlGuiWindow)
    Q_OBJECT
    Q_PROPERTY(bool hasMenuBar READ hasMenuBar)
    Q_PROPERTY(bool autoSaveSettings READ autoSaveSettings)
    Q_PROPERTY(QString autoSaveGroup READ autoSaveGroup)
    Q_PROPERTY(bool standardToolBarMenuEnabled READ isStandardToolBarMenuEnabled WRITE setStandardToolBarMenuEnabled)
    Q_FLAGS(StandardWindowOption)

public:
    /**
     * Construct a main window.
     *
     * @param parent The widget parent. This is usually 0 but it may also be the window
     * group leader. In that case, the KMainWindow becomes sort of a
     * secondary window.
     *
     * @param f Specify the widget flags. The default is
     * Qt::Window and Qt::WA_DeleteOnClose.  Qt::Window indicates that a
     * main window is a toplevel window, regardless of whether it has a
     * parent or not. Qt::WA_DeleteOnClose indicates that a main window is
     * automatically destroyed when its window is closed. Pass 0 if
     * you do not want this behavior.
     *
     * @see http://doc.trolltech.com/qt.html#WindowType-enum
     *
     * KMainWindows must be created on the heap with 'new', like:
     * \code
     * KMainWindow *kmw = new KMainWindow(...);
     * kmw->setObjectName(...);
     * \endcode
     *
     * IMPORTANT: For session management and window management to work
     * properly, all main windows in the application should have a
     * different name. If you don't do it, KMainWindow will create
     * a unique name, but it's recommended to explicitly pass a window name that will
     * also describe the type of the window. If there can be several windows of the same
     * type, append '#' (hash) to the name, and KMainWindow will replace it with numbers to make
     * the names unique. For example, for a mail client which has one main window showing
     * the mails and folders, and which can also have one or more windows for composing
     * mails, the name for the folders window should be e.g. "mainwindow" and
     * for the composer windows "composer#".
     *
     */
    explicit KXmlGuiWindow(QWidget *parent = 0, Qt::WindowFlags f = KDE_DEFAULT_WINDOWFLAGS);

    /**
     * \brief Destructor.
     *
     * Will also destroy the toolbars, and menubar if
     * needed.
     */
    ~KXmlGuiWindow() override;

    /**
     * Enables the build of a standard help menu when calling createGUI/setupGUI().
     *
     * The default behavior is to build one, you must call this function
     * to disable it
     */
    void setHelpMenuEnabled(bool showHelpMenu = true);

    /**
     * Return @p true when the help menu is enabled
     */
    bool isHelpMenuEnabled() const;

    virtual KXMLGUIFactory *guiFactory();

    /**
     * Create a GUI given a local XML file. In a regular app you usually want to use
     * setupGUI() instead of this one since it does more things for free
     * like setting up the toolbar/shortcut edit actions, etc.
     *
     * If @p xmlfile is 0,
     * then it will try to construct a local XML filename like
     * appnameui.xmlgui where 'appname' is your app's name.  If that file
     * does not exist, then the XML UI code will only use the global
     * (standard) XML file for the layout purposes.
     *
     * @param xmlfile The local xmlfile (relative or absolute)
     */
    void createGUI(const QString &xmlfile = QString());

    /**
     * Sets whether KMainWindow should provide a menu that allows showing/hiding
     * the available toolbars ( using KToggleToolBarAction ) . In case there
     * is only one toolbar configured a simple 'Show \<toolbar name here\>' menu item
     * is shown.
     *
     * The menu / menu item is implemented using xmlgui. It will be inserted in your
     * menu structure in the 'Settings' menu.
     *
     * If your application uses a non-standard xmlgui resource file then you can
     * specify the exact position of the menu / menu item by adding a
     * &lt;Merge name="StandardToolBarMenuHandler" /&gt;
     * line to the settings menu section of your resource file ( usually appname.xmlgui ).
     *
     * Note that you should enable this feature before calling createGUI() ( or similar ) .
     */
    void setStandardToolBarMenuEnabled(bool enable);
    bool isStandardToolBarMenuEnabled() const;

    /**
     * Sets whether KMainWindow should provide a menu that allows showing/hiding
     * of the statusbar ( using KToggleStatusBarAction ).
     *
     * The menu / menu item is implemented using xmlgui. It will be inserted
     * in your menu structure in the 'Settings' menu.
     *
     * Note that you should enable this feature before calling createGUI()
     * ( or similar ).
     *
     * If an application maintains the action on its own (i.e. never calls
     * this function) a connection needs to be made to let KMainWindow
     * know when that status (hidden/shown) of the statusbar has changed.
     * For example:
     * connect(action, SIGNAL(activated()),
     *         kmainwindow, SLOT(setSettingsDirty()));
     * Otherwise the status (hidden/show) of the statusbar might not be saved
     * by KMainWindow.
     */
    void createStandardStatusBarAction();

    /**
     * @see setupGUI()
     */
    enum StandardWindowOption {
        /**
         * adds action to show/hide the toolbar(s) and adds
         * action to configure the toolbar(s).
         * @see setStandardToolBarMenuEnabled
         */
        ToolBar = 1,

        /**
         * adds action to show the key configure action.
         */
        Keys = 2,

        /**
         * adds action to show/hide the statusbar if the
         * statusbar exists.  @see createStandardStatusBarAction
         */
        StatusBar = 4,

        /**
         * auto-saves (and loads) the toolbar/menubar/statusbar settings and
         * window size using the default name.  @see setAutoSaveSettings
         *
         * Typically you want to let the default window size be determined by
         * the widgets size hints. Make sure that setupGUI() is called after
         * all the widgets are created ( including setCentralWidget ) so the
         * default size's will be correct. @see setAutoSaveSettings for
         * more information on this topic.
         */
        Save = 8,

        /**
         * calls createGUI() once ToolBar, Keys and Statusbar have been
         * taken care of.  @see createGUI
         *
         * NOTE: when using KParts::MainWindow, remove this flag from the
         * setupGUI call, since you'll be using createGUI(part) instead.
         * @code
         *     setupGUI(ToolBar | Keys | StatusBar | Save);
         * @endcode
         */
        Create = 16,

        /**
         * All the above option
         * (this is the default)
         */
        Default = ToolBar | Keys | StatusBar | Save | Create
    };
    Q_DECLARE_FLAGS(StandardWindowOptions, StandardWindowOption)

    /**
     * Configures the current windows and its actions in the typical KDE
     * fashion.  The options are all enabled by default but can be turned
     * off if desired through the params or if the prereqs don't exists.
     *
     * Typically this function replaces createGUI().
     *
     * @see StandardWindowOptions
     * @note Since this method will restore the state of the application (toolbar, dockwindows
     *       positions...), you need to have added all your actions to your toolbars etc before
     *       calling to this method. (This note is only applicable if you are using the Default or
     *       Save flag).
     * @warning If you are calling createGUI yourself, remember to remove the Create flag from
     *          the @p options parameter.
     *
     */
    void setupGUI(StandardWindowOptions options = Default, const QString &xmlfile = QString());

    /**
     * Configures the current windows and its actions in the typical KDE
     * fashion.  The options are all enabled by default but can be turned
     * off if desired through the params or if the prereqs don't exists.
     *
     * @p defaultSize The default size of the window
     *
     * Typically this function replaces createGUI().
     *
     * @see StandardWindowOptions
     * @note Since this method will restore the state of the application (toolbar, dockwindows
     *       positions...), you need to have added all your actions to your toolbars etc before
     *       calling to this method. (This note is only applicable if you are using the Default or
     *       Save flag).
     * @warning If you are calling createGUI yourself, remember to remove the Create flag from
     *          the @p options parameter. Also, call setupGUI always after you call createGUI.
     */
    void setupGUI(const QSize &defaultSize, StandardWindowOptions options = Default, const QString &xmlfile = QString());

    /**
     * Returns a pointer to the mainwindows action responsible for the toolbars menu
     */
    QAction *toolBarMenuAction();

    /**
     * @internal for KToolBar
     */
    void setupToolbarMenuActions();

    // KDE5 TODO: change it to "using KXMLGUIBuilder::finalizeGUI;"
    void finalizeGUI(KXMLGUIClient *client) override;

    /**
     * @internal
     */
    void finalizeGUI(bool force);

    // reimplemented for internal reasons
    void applyMainWindowSettings(const KConfigGroup &config) override;

public Q_SLOTS:
    /**
     * Show a standard configure toolbar dialog.
     *
     * This slot can be connected directly to the action to configure toolbar.
     * This is very simple to do that by adding a single line
     * \code
     * KStandardAction::configureToolbars( this, SLOT( configureToolbars() ),
     *                                actionCollection() );
     * \endcode
     */
    virtual void configureToolbars();

    /**
     * Apply a state change
     *
     * Enable and disable actions as defined in the XML rc file
     */
    virtual void slotStateChanged(const QString &newstate);

    /**
     * Apply a state change
     *
     * Enable and disable actions as defined in the XML rc file,
     * can "reverse" the state (disable the actions which should be
     * enabled, and vice-versa) if specified.
     */
    void slotStateChanged(const QString &newstate,
                          bool reverse);

protected:
    /**
     * Reimplemented to catch QEvent::Polish in order to adjust the object name
     * if needed, once all constructor code for the main window has run.
     * Also reimplemented to catch when a QDockWidget is added or removed.
     */
    bool event(QEvent *event) override;

protected Q_SLOTS:
    /**
     * Rebuilds the GUI after KEditToolbar changed the toolbar layout.
     * @see configureToolbars()
     */
    virtual void saveNewToolbarConfig();

private:
    Q_PRIVATE_SLOT(k_func(), void _k_slotFactoryMakingChanges(bool))
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KXmlGuiWindow::StandardWindowOptions)

#endif
