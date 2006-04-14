/** @file
*   This file is part of the KDE/KOffice project.
*   Copyright (C) 2005, Gary Cramblitt <garycramblitt@comcast.net>
*
*   @author Gary Cramblitt <garycramblitt@comcast.net>
*   @since KOffice 1.5
*
*   This library is free software; you can redistribute it and/or
*   modify it under the terms of the GNU Library General Public
*   License as published by the Free Software Foundation; either
*   version 2 of the License, or (at your option) any later version.
*
*   This library is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*   Library General Public License for more details.
*
*   You should have received a copy of the GNU Library General Public License
*   along with this library; see the file COPYING.LIB.  If not, write to
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*   Boston, MA 02110-1301, USA.
*/

#ifndef __KKBDACCESSEXTENSIONS_H__
#define __KKBDACCESSEXTENSIONS_H__

// Qt includes.
#include <QObject>
#include <QLabel>
#include <QKeyEvent>
#include <QEvent>

// KOffice includes.
#include <koffice_export.h>

class KKbdAccessExtensionsPrivate;
class KMainWindow;

/** KKbdAccessExtensions is an object that improves accessibility for motor impaired users
*   who may not be able to easily use a mouse.  It adds two new capabilities using the keyboard:
*
*     - Resizing and positioning of panel widgets derived from QSplitter and QDockWindow.
*     - Setting focus to any widget that accepts focus.
*
*   @section sizing_mode Sizing Mode
*
*   Users may press F8 or Shift-F8 (defaults) to enter sizing mode.  A sizing icon appears on the first
*   QSplitter or QDockWindow handle found in the application (F8) or the last such handle (Shift+F8).
*   (A "handle" is the divider bar that appears to the left, right, above, or below each panel
*   of a QSplitter or QDockArea.)
*
*   Once in sizing mode, the following functions are available via the keyboard:
*
*     - F8          Moves to the next sizing handle.  After the last handle, exits sizing mode.
*     - Shift+F8    Moves to the previous sizing handle.  After the first handle, exits sizing mode.
*     - Esc         Exits sizing mode.
*     - LeftArrow   When on a vertical sizing handle, moves the handle to the left.
*                   When on a horizontal sizing handle, moves the handle up.
*     - RightArrow  When on a vertical sizing handle, moves the handle to the right.
*                   When on a horizontal sizing handle, moves the handle down.
*     - UpArrow     When on a vertical sizing handle, moves the handle to the left.
*                   When on a horizontal sizing handle, moves the handle up.
*     - DownArrow   When on a vertical sizing handle, moves the handle to the right.
*                   When on a horizontal sizing handle, moves the handle down.
*     - PgUp        Like LeftArrow or UpArrow, but moves the handle 5X farther to the left or up.
*     - PgDn        Like RightArrow or DownArrow, but moves the handle 5X farther to the right or down.
*     - Enter       (On numeric keypad).  When on the handle of a QDockWindow, undocks or docks
*                   the widget.  Ignored when on the handle of a QSplitter.
*
*   The default step size for each arrow key press is 10 pixels.
*
*   When a QDockWindow is undocked, the sizing icon appears in the center of the window.
*   The arrow keys and PgUp/PgDn move the undocked window on the screen.  Shifted arrow keys
*   and PgUp/PgDn decrease/increase the size of the undocked window.
*
*   When the sizing icon is on a sizing handle, the mouse may also be used to move the handle
*   without having to click and drag.  When moving the mouse while sizing icon is on an undocked
*   QDockWindow, the window moves with the mouse.  Holding Shift down while moving the mouse
*   sizes the QDockWindow.
*
*   @note Users can also move and size undocked windows using the Window Operations Menu (Alt+F3).
*
*   Clicking any mouse button exits sizing mode.
*
*   When entering sizing mode, the position of the mouse cursor is saved and restored when
*   exiting sizing mode.
*
*   For a QSplitter or QDockWindow to be found, it must be in the kapp::allWidgets() list.
*
*   @section focus_setting Focus Setting
*
*   Users can press Alt-F8.  A small box appears in the upperleft corner of each visible widget
*   on the screen that can accept focus.  Each box is assigned a single letter or digit.
*   User can press the corresponding key to set focus to the widget.
*
*   At most 36 such shortcuts are possible.  If any application shortcuts are single letters
*   or digits, those shortcuts are not in any of the boxes.
*
*   Clicking any mouse button exits Focus Setting mode.
*
*   @section notes Notes
*
*   The F8, Shift+F8, and Alt+F8 keys are KShortcuts and therefore user may choose different keys in
*   the application's Configure Shortcuts dialog.
*
*   @note At present, these shortcuts may not be multi-key.  If user sets multi-key
*   shortcuts, they will not work.
*
*   F8/Shift+F8 are the default shortcuts because these are the keys used for similar
*   functionality in GNOME and Java SWT.
*/
class KOFFICECORE_EXPORT KKbdAccessExtensions : public QObject
{
    // TODO: A .moc isn't really needed right now, but see TODO in eventFilter method.
    // Q_PROPERTY(int stepSize READ stepSize WRITE setStepSize)

    public:
        /** Constructor.
        *   @param parent       KMainWindow of the application.  Required.
        *   @param name         (optional) Name of this object.
        */
        KKbdAccessExtensions(KMainWindow* parent, const char* name = 0);

        /** Destructor. */
        virtual ~KKbdAccessExtensions();

        /** Returns number of pixels panel is sized for each arrow key pressed.  Default is 10. */
        int stepSize() const;
        /** Sets number of pixels panel is sized for each arrow key pressed. */
        void setStepSize(int s);

    protected:
        /** Event filter installed on kapp object. */
        bool eventFilter( QObject *o, QEvent *e );

        /** Retrieves a list of all Splitter and DockArea widgets in the application. */
        QWidgetList* getAllPanels();
        /** Advances to the next Panel handle.  If not currently in resizing mode,
            turns it on. */
        void nextHandle();
        /** Moves to the previous Panel handle.  If not currently in resizing mode,
            turns it on. */
        void prevHandle();
        /** Exits Sizing mode. */
        void exitSizing();
        /** Moves panel handle based on key pressed. */
        void resizePanelFromKey(int key, int state);
        /** Moves panel handle based on deltaX and deltaY and state of keyboard modifier keys. */
        void resizePanel(int dx, int dy, int state);
        /** Displays the sizer icon. */
        void showIcon();
        /** Hides the sizer icon. */
        void hideIcon();

        /** Displays the access keys. */
        void displayAccessKeys();
        /** Handles an access keypress. */
        bool handleAccessKey( const QKeyEvent* ev );

    private:
        KKbdAccessExtensionsPrivate* d;
};

/** Provides a way to sort QLabelss using a QValueList based on their screen position. */
class KSortedLabel
{
public:
    KSortedLabel(QLabel* l);
    KSortedLabel();    // default constructor
    bool operator<( const KSortedLabel& l) const;
    QLabel* label() const { return m_l; }

private:
    QLabel* m_l;
};

#endif              // __KKBDACCESSEXTENSIONS_H__
