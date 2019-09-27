/* This file is part of the KDE libraries
    Copyright (C) 2000 Reginald Stadlbauer (reggie@kde.org)
    (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
    (C) 1997, 1998 Sven Radej (radej@kde.org)
    (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
    (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)
    (C) 1999, 2000 Kurt Granroth (granroth@kde.org)
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

#ifndef KTOOLBAR_H
#define KTOOLBAR_H

#include <kritawidgetutils_export.h>

#include <QToolBar>

class QDomElement;

class KConfigGroup;
class KConfig;
class KMainWindow;
class KXMLGUIClient;

/**
 * @short Floatable toolbar with auto resize.
 *
 * A KDE-style toolbar.
 *
 * KToolBar can be used as a standalone widget, but KMainWindow
 * provides easy factories and management of one or more toolbars.
 *
 * KToolBar uses a global config group to load toolbar settings on
 * construction. It will reread this config group on a
 * KApplication::appearanceChanged() signal.
 *
 * KToolBar respects Kiosk settings (see the KAuthorized namespace in the
 * KConfig framework).  In particular, system administrators can prevent users
 * from moving toolbars with the "movable_toolbars" action, and from showing or
 * hiding toolbars with the "options_show_toolbar" action.  For example, to
 * disable both, add the following the application or global configuration:
 * @verbatim
   [KDE Action Restrictions][$i]
   movable_toolbars=false
   options_show_toolbar=false
   @endverbatim
 *
 * @note If you can't depend on KXmlGui but you want to integrate with KDE, you can use QToolBar with:
 *    Set ToolButtonStyle to Qt::ToolButtonFollowStyle, this will make QToolBar use the settings for "Main Toolbar"
 *    Additionally set QToolBar::setProperty("otherToolbar", true) to use settings for "Other toolbars"
 *    Settings from "Other toolbars" will only work on widget styles derived from KStyle
 * @author Reginald Stadlbauer <reggie@kde.org>, Stephan Kulow <coolo@kde.org>, Sven Radej <radej@kde.org>, Hamish Rodda <rodda@kde.org>.
 */
class KRITAWIDGETUTILS_EXPORT KToolBar : public QToolBar
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * This constructor takes care of adding the toolbar to the mainwindow,
     * if @p parent is a QMainWindow.
     *
     * Normally KDE applications do not call this directly, they either
     * call KMainWindow::toolBar(name), or they use XML-GUI and specify
     * toolbars using XML.
     *
     * @param objectName  The QObject name of this toolbar, required so that QMainWindow can save and load the toolbar position,
     *                    and so that KToolBar can find out if it's the main toolbar.
     * @param parent      The standard toolbar parent (usually a KMainWindow)
     * @param readConfig  whether to apply the configuration (global and application-specific)
     */
    explicit KToolBar(const QString &objectName, QWidget *parent, bool readConfig = true);

    /**
     * Destroys the toolbar.
     */
    ~KToolBar() override;

    /**
     * Returns the main window that this toolbar is docked with.
     */
    KMainWindow *mainWindow() const;

    /**
     * Convenience function to set icon size
     */
    void setIconDimensions(int size);

    /**
     * Returns the default size for this type of toolbar.
     *
     * @return the default size for this type of toolbar.
     */
    int iconSizeDefault() const; // KDE5: hide from public API. Doesn't make sense to export this, and it isn't used.

    /**
     * Save the toolbar settings to group @p configGroup in @p config.
     */
    void saveSettings(KConfigGroup &cg);

    /**
     * Read the toolbar settings from group @p configGroup in @p config
     * and apply them.
     */
    void applySettings(const KConfigGroup &cg);

    /**
     * Adds an XML gui client that uses this toolbar
     * @since 4.8.1
     */
    void addXMLGUIClient(KXMLGUIClient *client);

    /**
     * Removes an XML gui client that uses this toolbar
     * @since 4.8.5
     */
    void removeXMLGUIClient(KXMLGUIClient *client);

    /**
     * Load state from an XML @p element, called by KXMLGUIBuilder.
     */
    void loadState(const QDomElement &element);

    /**
     * Save state into an XML @p element, called by KXMLGUIBuilder.
     */
    void saveState(QDomElement &element) const;

    /**
     * Reimplemented to support context menu activation on disabled tool buttons.
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

    /**
     * Returns whether the toolbars are currently editable (drag & drop of actions).
     */
    static bool toolBarsEditable();

    /**
     * Enable or disable toolbar editing via drag & drop of actions.  This is
     * called by KEditToolbar and should generally be set to disabled whenever
     * KEditToolbar is not active.
     */
    static void setToolBarsEditable(bool editable);

    /**
     * Returns whether the toolbars are locked (i.e., moving of the toobars disallowed).
     */
    static bool toolBarsLocked();

    /**
     * Allows you to lock and unlock all toolbars (i.e., disallow/allow moving of the toobars).
     */
    static void setToolBarsLocked(bool locked);

    /**
     * Emits a dbus signal to tell all toolbars in all applications, that the user settings have
     * changed.
     * @since 5.0
     */
    static void emitToolbarStyleChanged();

protected Q_SLOTS:
    virtual void slotMovableChanged(bool movable);

protected:
    void contextMenuEvent(QContextMenuEvent *) override;
    void actionEvent(QActionEvent *) override;

    // Draggable toolbar configuration
    void dragEnterEvent(QDragEnterEvent *) override;
    void dragMoveEvent(QDragMoveEvent *) override;
    void dragLeaveEvent(QDragLeaveEvent *) override;
    void dropEvent(QDropEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;

private:
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT(d, void slotAppearanceChanged())
    Q_PRIVATE_SLOT(d, void slotContextAboutToShow())
    Q_PRIVATE_SLOT(d, void slotContextAboutToHide())
    Q_PRIVATE_SLOT(d, void slotContextLeft())
    Q_PRIVATE_SLOT(d, void slotContextRight())
    Q_PRIVATE_SLOT(d, void slotContextShowText())
    Q_PRIVATE_SLOT(d, void slotContextTop())
    Q_PRIVATE_SLOT(d, void slotContextBottom())
    Q_PRIVATE_SLOT(d, void slotContextIcons())
    Q_PRIVATE_SLOT(d, void slotContextText())
    Q_PRIVATE_SLOT(d, void slotContextTextRight())
    Q_PRIVATE_SLOT(d, void slotContextTextUnder())
    Q_PRIVATE_SLOT(d, void slotContextIconSize())
    Q_PRIVATE_SLOT(d, void slotLockToolBars(bool))
};

#endif
