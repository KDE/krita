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
     (C) 2000-2008 David Faure (faure@kde.org)

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

#ifndef KMAINWINDOW_H
#define KMAINWINDOW_H

#include <kritawidgetutils_export.h>

#include <QMainWindow>
#include <QMetaClassInfo>

class QMenu;
class KConfig;
class KConfigGroup;
class KMWSessionManager;
class KMainWindowPrivate;
class KToolBar;

// internal, not public API, may change any time
#define XMLGUI_DECLARE_PRIVATE(classname) \
    inline classname ## Private *k_func() { return reinterpret_cast<classname ## Private *>(k_ptr); } \
    inline const classname ## Private *k_func() const { return reinterpret_cast<classname ## Private *>(k_ptr); } \
    friend class classname ## Private;

// This is mostly from KDE3. TODO KDE5: remove the constructor parameter.
#define KDE_DEFAULT_WINDOWFLAGS 0

/**
 * @short %KDE top level main window
 *
 * Top level widget that provides toolbars, a status line and a frame.
 *
 * It should be used as a top level (parent-less) widget.
 * It manages the geometry for all its children, including your
 * main widget.
 *
 * Normally, you will inherit from KMainWindow,
 * then construct (or use some existing) widget as
 * your main view. You can set only one main view.
 *
 * You can add as many toolbars as you like. There can be only one menubar
 * and only one statusbar.
 *
 * The toolbars, menubar, and statusbar can be created by the
 * KMainWindow and - unlike the old KMainWindow - may, but do not
 * have to, be deleted by you. KMainWindow will handle that internally.
 *
 * Height and width can be operated independently from each other. Simply
 * define the minimum/maximum height/width of your main widget and
 * KMainWindow will take this into account. For fixed size windows set
 * your main widget to a fixed size.
 *
 * Fixed aspect ratios (heightForWidth()) and fixed width widgets are
 * not supported.
 *
 * KMainWindow will set icon, mini icon and caption, which it gets
 * from KApplication. It provides full session management, and
 * will save its position, geometry and positions of toolbars and
 * menubar on logout. If you want to save additional data, reimplement
 * saveProperties() and (to read them again on next login)
 * readProperties(). To save special data about your data, reimplement
 * saveGlobalProperties(). To warn user that application or
 * windows have unsaved data on close or logout, reimplement
 * queryClose().
 *
 * You have to implement session restoring also in your main() function.
 * There are also kRestoreMainWindows convenience functions which
 * can do this for you and restore all your windows on next login.
 *
 * Note that KMainWindow uses KGlobal::ref() and KGlobal::deref() so that closing
 * the last mainwindow will quit the application unless there is still something
 * that holds a ref in KGlobal - like a KIO job, or a systray icon.
 *
 * @see KApplication
 * @author Reginald Stadlbauer (reggie@kde.org) Stephan Kulow (coolo@kde.org), Matthias Ettrich (ettrich@kde.org), Chris Schlaeger (cs@kde.org), Sven Radej (radej@kde.org). Maintained by David Faure (faure@kde.org)
 */

class KRITAWIDGETUTILS_EXPORT KMainWindow : public QMainWindow
{
    friend class KMWSessionManager;
    friend class DockResizeListener;
    XMLGUI_DECLARE_PRIVATE(KMainWindow)
    Q_OBJECT
    Q_PROPERTY(bool hasMenuBar READ hasMenuBar)
    Q_PROPERTY(bool autoSaveSettings READ autoSaveSettings)
    Q_PROPERTY(QString autoSaveGroup READ autoSaveGroup)

public:
    /**
     * Construct a main window.
     *
     * @param parent The widget parent. This is usually 0 but it may also be the window
     * group leader. In that case, the KMainWindow becomes sort of a
     * secondary window.
     *
     * @param f Specify the window flags. The default is none.
     *
     * Note that a KMainWindow per-default is created with the
     * WA_DeleteOnClose attribute, i.e. it is automatically destroyed when the
     * window is closed. If you do not want this behavior, call
     * setAttribute(Qt::WA_DeleteOnClose, false);
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
    explicit KMainWindow(QWidget *parent = 0, Qt::WindowFlags f = KDE_DEFAULT_WINDOWFLAGS);

    /**
     * \brief Destructor.
     *
     * Will also destroy the toolbars, and menubar if
     * needed.
     */
    ~KMainWindow() override;

    /**
     * If the session did contain so high a number, @p true is returned,
     * else @p false.
     * @see restore()
     **/
    static bool canBeRestored(int number);

    /**
     * Returns the className() of the @p number of the toplevel window which
     * should be restored.
     *
     * This is only useful if your application uses
     * different kinds of toplevel windows.
     */
    static const QString classNameOfToplevel(int number);

    /**
     * Try to restore the toplevel widget as defined by @p number (1..X).
     *
     * You should call canBeRestored() first.
     *
     * If the session did not contain so high a number, the configuration
     * is not changed and @p false returned.
     *
     * That means clients could simply do the following:
     * \code
     * if (qApp->isSessionRestored()){
     *   int n = 1;
     *   while (KMainWindow::canBeRestored(n)){
     *     (new childMW)->restore(n);
     *     n++;
     *   }
     * } else {
     *   // create default application as usual
     * }
     * \endcode
     * Note that if @p show is true (default), QWidget::show() is called
     * implicitly in restore.
     *
     * With this you can easily restore all toplevel windows of your
     * application.
     *
     * If your application uses different kinds of toplevel
     * windows, then you can use KMainWindow::classNameOfToplevel(n)
     * to determine the exact type before calling the childMW
     * constructor in the example from above.
     *
     * <i>Note that you don't need to deal with this function. Use the
     * kRestoreMainWindows() convenience template function instead!</i>
     * @see kRestoreMainWindows()
     * @see readProperties()
     * @see canBeRestored()
     */
    bool restore(int number, bool show = true);

    /**
     * Returns true, if there is a menubar
     */
    bool hasMenuBar();

    /**
     * List of members of KMainWindow class.
     */
    static QList<KMainWindow *> memberList();

    /**
     * Returns a pointer to the toolbar with the specified name.
     * This refers to toolbars created dynamically from the XML UI
     * framework.  If the toolbar does not exist one will be created.
     *
     * @param name The internal name of the toolbar. If no name is
     *             specified "mainToolBar" is assumed.
     *
     * @return A pointer to the toolbar
     **/
    KToolBar *toolBar(const QString &name = QString());

    /**
     * @return A list of all toolbars for this window
     */
    QList<KToolBar *> toolBars() const;

    /**
     * Call this to enable "auto-save" of toolbar/menubar/statusbar settings
     * (and optionally window size).
     * If the *bars were moved around/shown/hidden when the window is closed,
     * saveMainWindowSettings( KConfigGroup(KSharedConfig::openConfig(), groupName) ) will be called.
     *
     * @param groupName a name that identifies this "type of window".
     * You can have several types of window in the same application.
     *
     * @param saveWindowSize set it to true to include the window size
     * when saving.
     *
     * Typically, you will call setAutoSaveSettings() in your
     * KMainWindow-inherited class constructor, and it will take care
     * of restoring and saving automatically. Make sure you call this
     * _after all_ your *bars have been created.
     *
     * To make sure that KMainWindow properly obtains the default
     * size of the window you should do the following:
     * - Remove hard coded resize() calls in the constructor or main, they
     *   should be removed in favor of letting the automatic resizing
     *   determine the default window size.  Hard coded window sizes will
     *   be wrong for users that have big fonts, use different styles,
     *   long/small translations, large toolbars, and other factors.
     * - Put the setAutoSaveSettings ( or setupGUI() ) call after all widgets
     *   have been created and placed inside the main window (i.e. for 99% of
     *   apps setCentralWidget())
     * - Widgets that inherit from QWidget (like game boards) should overload
     *   "virtual QSize sizeHint() const;" to specify a default size rather
     *   than letting QWidget::adjust use the default size of 0x0.
     */
    void setAutoSaveSettings(const QString &groupName = QLatin1String("MainWindow"),
                             bool saveWindowSize = true);

    /**
     * Overload that lets you specify a KConfigGroup.
     * This allows the settings to be saved into another file than KSharedConfig::openConfig().
     * @since 4.1
     */
    void setAutoSaveSettings(const KConfigGroup &group,
                             bool saveWindowSize = true);

    /**
     * Disable the auto-save-settings feature.
     * You don't normally need to call this, ever.
     */
    void resetAutoSaveSettings();

    /**
     * @return the current autosave setting, i.e. true if setAutoSaveSettings() was called,
     * false by default or if resetAutoSaveSettings() was called.
     */
    bool autoSaveSettings() const;

    /**
     * @return the group used for setting-autosaving.
     * Only meaningful if setAutoSaveSettings(QString) was called.
     * This can be useful for forcing a save or an apply, e.g. before and after
     * using KEditToolbar.
     *
     * NOTE: you should rather use saveAutoSaveSettings() for saving or autoSaveConfigGroup() for loading.
     * This method doesn't make sense if setAutoSaveSettings(KConfigGroup) was called.
     */
    QString autoSaveGroup() const;

    /**
     * @return the group used for setting-autosaving.
     * Only meaningful if setAutoSaveSettings() was called.
     * This can be useful for forcing an apply, e.g. after using KEditToolbar.
     * @since 4.1
     */
    KConfigGroup autoSaveConfigGroup() const;

    /**
     * Read settings for statusbar, menubar and toolbar from their respective
     * groups in the config file and apply them.
     *
     * @param config Config group to read the settings from.
     * KF5 porting note: the unused bool argument was removed, make sure to remove it from your
     * reimplementations too! And add a override for good measure.
     */
    virtual void applyMainWindowSettings(const KConfigGroup &config);

    /**
     * Save settings for statusbar, menubar and toolbar to their respective
     * groups in the config group @p config.
     *
     * @param config Config group to save the settings to.
     */
    void saveMainWindowSettings(KConfigGroup &config);

    /**
     * Returns the path under which this window's D-Bus object is exported.
     * @since 4.0.1
     */
    QString dbusName() const;

public Q_SLOTS:
    /**
     * Makes a KDE compliant caption (window title).
     *
     * @param caption Your caption. @em Do @em not include the application name
     * in this string. It will be added automatically according to the KDE
     * standard.
     */
    virtual void setCaption(const QString &caption);
    /**
     * Makes a KDE compliant caption.
     *
     * @param caption Your caption. @em Do @em not include the application name
     * in this string. It will be added automatically according to the KDE
     * standard.
     * @param modified Specify whether the document is modified. This displays
     * an additional sign in the title bar, usually "**".
     */
    virtual void setCaption(const QString &caption, bool modified);

    /**
     * Make a plain caption without any modifications.
     *
     * @param caption Your caption. This is the string that will be
     * displayed in the window title.
     */
    virtual void setPlainCaption(const QString &caption);

    /**
     * Open the help page for the application.
     *
     *  The application name is
     * used as a key to determine what to display and the system will attempt
     * to open \<appName\>/index.html.
     *
     * This method is intended for use by a help button in the toolbar or
     * components outside the regular help menu. Use helpMenu() when you
     * want to provide access to the help system from the help menu.
     *
     * Example (adding a help button to the first toolbar):
     *
     * \code
     * toolBar(0)->addAction(KisIconUtils::loadIcon("help-contents"), i18n("Help"),
     *                       this, SLOT(appHelpActivated()));
     * \endcode
     *
     */
    void appHelpActivated(void);

    /**
     * Tell the main window that it should save its settings when being closed.
     * This is part of the auto-save-settings feature.
     * For everything related to toolbars this happens automatically,
     * but you have to call setSettingsDirty() in the slot that toggles
     * the visibility of the statusbar.
     */
    void setSettingsDirty();

protected:
    /**
     * Reimplemented to catch QEvent::Polish in order to adjust the object name
     * if needed, once all constructor code for the main window has run.
     * Also reimplemented to catch when a QDockWidget is added or removed.
     */
    bool event(QEvent *event) override;

    /**
     * Reimplemented to autosave settings and call queryClose().
     *
     * We recommend that you reimplement queryClose() rather than closeEvent().
     * If you do it anyway, ensure to call the base implementation to keep
     * the feature of auto-saving window settings working.
     */
    void closeEvent(QCloseEvent *) override;

    /**
       Called before the window is closed, either by the user or indirectly by
       the session manager.

       The purpose of this function is to prepare the window in a way that it is
       safe to close it, i.e. without the user losing some data.

       Default implementation returns true. Returning @p false will cancel
       the closing, and, if KApplication::sessionSaving() is true, it will also
       cancel KDE logout.

       Reimplement this function to prevent the user from losing data.
       Example:
       \code
       switch ( KMessageBox::warningYesNoCancel( this,
                i18n("Save changes to document foo?")) ) {
       case KMessageBox::Yes :
         // save document here. If saving fails, return false;
         return true;
       case KMessageBox::No :
         return true;
       default: // cancel
         return false;
       \endcode

       Note that you should probably @em not actually close the document from
       within this method, as it may be called by the session manager before the
       session is saved. If the document is closed before the session save occurs,
       its location might not be properly saved. In addition, the session shutdown
       may be canceled, in which case the document should remain open.

       @see KApplication::sessionSaving()
    */
    virtual bool queryClose();

    /**
     * Save your instance-specific properties. The function is
     * invoked when the session manager requests your application
     * to save its state.
     *
     * Please reimplement these function in childclasses.
     *
     * Note: No user interaction is allowed
     * in this function!
     *
     */
    virtual void saveProperties(KConfigGroup &) {}

    /**
     * Read your instance-specific properties.
     *
     * Is called indirectly by restore().
     */
    virtual void readProperties(const KConfigGroup &) {}

    /**
      * Save your application-wide properties. The function is
      * invoked when the session manager requests your application
      * to save its state.
      *
      * This function is similar to saveProperties() but is only called for
      * the very first main window, regardless how many main window are open.

      * Override it if you need to save other data about your documents on
      * session end. sessionConfig is a config to which that data should be
      * saved. Normally, you don't need this function. But if you want to save
      * data about your documents that are not in opened windows you might need
      * it.
      *
      * Default implementation does nothing.
      */
    virtual void saveGlobalProperties(KConfig *sessionConfig);

    /**
     * The counterpart of saveGlobalProperties().
     *
     * Read the application-specific properties in again.
     */
    virtual void readGlobalProperties(KConfig *sessionConfig);
    void savePropertiesInternal(KConfig *, int);
    bool readPropertiesInternal(KConfig *, int);

    /**
     * For inherited classes
     */
    bool settingsDirty() const;

protected Q_SLOTS:

    /**
     * This slot should only be called in case you reimplement closeEvent() and
     * if you are using the "auto-save" feature. In all other cases,
     * setSettingsDirty() should be called instead to benefit from the delayed
     * saving.
     *
     * @see setAutoSaveSettings
     * @see setSettingsDirty
     *
     * Example:
     * \code
     *
     * void MyMainWindow::closeEvent( QCloseEvent *e )
     * {
     *   // Save settings if auto-save is enabled, and settings have changed
     *   if ( settingsDirty() && autoSaveSettings() )
     *     saveAutoSaveSettings();
     *   ..
     * }
     * \endcode
     */
    void saveAutoSaveSettings();

protected:
    KMainWindow(KMainWindowPrivate &dd, QWidget *parent, Qt::WindowFlags f);

    KMainWindowPrivate *const k_ptr;
private:
    Q_PRIVATE_SLOT(k_func(), void _k_slotSettingsChanged(int))
    Q_PRIVATE_SLOT(k_func(), void _k_slotSaveAutoSaveSize())
};

/**
 * @def KDE_RESTORE_MAIN_WINDOWS_NUM_TEMPLATE_ARGS
 * @ingroup XMLGUIMacros
 * Returns the maximal number of arguments that are actually
 * supported by kRestoreMainWindows().
 **/
#define KDE_RESTORE_MAIN_WINDOWS_NUM_TEMPLATE_ARGS 3

/**
 * Restores the last session. (To be used in your main function).
 *
 * These functions work also if you have more than one kind of toplevel
 * widget (each derived from KMainWindow, of course).
 *
 * Imagine you have three kinds of toplevel widgets: the classes childMW1,
 * childMW2 and childMW3. Than you can just do:
 *
 * \code
 * if (qApp->isSessionRestored())
 *   kRestoreMainWindows< childMW1, childMW2, childMW3 >();
 * else {
 *   // create default application as usual
 * }
 * \endcode
 *
 * kRestoreMainWindows<>() will create (on the heap) as many instances
 * of your main windows as have existed in the last session and
 * call KMainWindow::restore() with the correct arguments. Note that
 * also QWidget::show() is called implicitly.
 *
 * Currently, these functions are provided for up to three
 * template arguments. If you need more, tell us. To help you in
 * deciding whether or not you can use kRestoreMainWindows, a
 * define #KDE_RESTORE_MAIN_WINDOWS_NUM_TEMPLATE_ARGS is provided.
 *
 * @see KMainWindow::restore()
 * @see KMainWindow::classNameOfToplevel()
 **/
template <typename T>
inline void kRestoreMainWindows()
{
    for (int n = 1; KMainWindow::canBeRestored(n); ++n) {
        const QString className = KMainWindow::classNameOfToplevel(n);
        if (className == QLatin1String(T::staticMetaObject.className())) {
            (new T)->restore(n);
        }
    }
}

template <typename T0, typename T1>
inline void kRestoreMainWindows()
{
    const char *classNames[2];
    classNames[0] = T0::staticMetaObject.className();
    classNames[1] = T1::staticMetaObject.className();
    for (int n = 1; KMainWindow::canBeRestored(n); ++n) {
        const QString className = KMainWindow::classNameOfToplevel(n);
        if (className == QLatin1String(classNames[0])) {
            (new T0)->restore(n);
        } else if (className == QLatin1String(classNames[1])) {
            (new T1)->restore(n);
        }
    }
}

template <typename T0, typename T1, typename T2>
inline void kRestoreMainWindows()
{
    const char *classNames[3];
    classNames[0] = T0::staticMetaObject.className();
    classNames[1] = T1::staticMetaObject.className();
    classNames[2] = T2::staticMetaObject.className();
    for (int n = 1; KMainWindow::canBeRestored(n); ++n) {
        const QString className = KMainWindow::classNameOfToplevel(n);
        if (className == QLatin1String(classNames[0])) {
            (new T0)->restore(n);
        } else if (className == QLatin1String(classNames[1])) {
            (new T1)->restore(n);
        } else if (className == QLatin1String(classNames[2])) {
            (new T2)->restore(n);
        }
    }
}

#endif
