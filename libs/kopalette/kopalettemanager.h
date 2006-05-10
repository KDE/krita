/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2, as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KO_PALETTE_MANAGER_
#define _KO_PALETTE_MANAGER_

#include <qobject.h>
#include <q3dockwindow.h>
#include <QString>
#include <QMap>
#include <q3dict.h>
#include <q3valuestack.h>
#include <QWidget>
#include <qsignalmapper.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <QEvent>

#include <koffice_export.h>

#include <KoView.h>

class KoPalette;
class KActionMenu;
class KAction;
class KActionCollection;
class KToggleAction;

enum enumKoDockability {
    DOCK_ENABLED = 0, // It's possible to dock the dockers
    DOCK_DISABLED = 1, // The dockers cannot be docked
    DOCK_SMART = 2 // On small screens, don't dock, else dock, initially
};

enum enumKoPaletteStyle {
    PALETTE_DOCKER,  // QDockWindow based docker with tabs
    PALETTE_TOOLBOX, // QDockWindow based docker with a QToolBox
    PALETTE_ICONTABS // QDockWidget based docker with icon tabs
};


namespace {
    struct DockerRecord {
        int position;
        int x;
        int y;
        int w;
        int h;
    };
}

/**
 * Manages the set of dockwindow palettes and their widgets.
 *
 * You create one palette manager per view; then you can add widgets
 * to your hearts content. Widgets are grouped into dock windows by
 * palette names. To see the menu entries, add a the following line
 * to your .rc file:
 *
 * &lt;Action name="view_palette_action_menu"/&gt;
 *
 * There are two styles: one that uses tabs and one that uses the vertical
 * QToolBox style to separate and show individual widgets. By implementing
 * the kopalette interface and extending the above enum, you can add
 * more types.
 *
 * TODO:
 *        - Drag & Drop
 *        - Restore order of tabs in a docker
 *        - Set initial position of floating dockers on first startup
 *        - Restoration of the application default state
 *        - Make it impossible to close a floating palette window with alt-f4
 */
class KOPALETTE_EXPORT KoPaletteManager : public QObject {

    Q_OBJECT


public:

    KoPaletteManager(KoView * view, KActionCollection * ac, const char * name);
    virtual ~KoPaletteManager();

public:
    
    /**
     * Add a new tab with the given name an description to the specified palette.
     * The widget's caption is used, where necessary. If there is no
     * palette with this name, a new palette is created with the given palette name
     * and the widget's caption.
     *
     * If there is already a widget with the given name, that widget will be
     * unplugged (but not destroyed) and the given widget will be plugged in place.
     *
     * If the widget occurs in the saved configuration, it is not added to the
     * specified palette, but in the place where it was left.
     *
     * @param widget the widget that will be inserted as a tab or entry in the palette
     * @param name the name under which the palette will be stored. Not the caption -- do not i18n this.
     * @param paletteName the unique name of the palette this widget will be a child of. If the palette already exists, the current widget is added to it.
     * @param position the position of the widget in the palettes
     * @param style docker, toolbox or slider
     */
    virtual void addWidget(QWidget * widget, const QString & name, const QString & paletteName, int position = -1, 
			   enumKoPaletteStyle style = PALETTE_DOCKER, bool shown = true);


    /**
     * Get a certain widget by name
     */
    virtual QWidget * widget(const QString & name);

    /**
     * Show a the specified palette wherever it currently is.
     */
    virtual void showWidget(const QString & name);

    /**
     * hide the specified widget
     */
    virtual void hideWidget(const QString & name);
    
    /**
     * Remove the widget with the specified name from whichever
     * palette it is currently in. If it is the last widget in
     * the palette, the palette is destroyed. If the name does
     * not occur, nothing is done.
     */
    virtual void removeWidget(const QString & name);


    /**
     * Create an empty palette in the given style. with the given name and caption. If
     * the palette already exists, nothing is done.
     */
    virtual KoPalette * createPalette(const QString & name, const QString & caption, enumKoPaletteStyle style = PALETTE_DOCKER);

    /**
     * Move the specified palette to the specified location. If there is already
     * a location for the palette in the saved settings, then move it there
     * instead of the specified location.
     */
    virtual void placePalette(const QString & name, Qt::DockWidgetArea location = Qt::RightDockWidgetArea);

    /**
     * Add a palette; this can be used to add palettes other than those in the two
     * default styles.
     */
     virtual void addPalette(KoPalette * palette, const QString & name, Qt::DockWidgetArea location = Qt::RightDockWidgetArea);

     /**
      * Sets all palettes to the specified fixed width
      */
     virtual void setFixedWidth(int w);

public slots:

    void slotTogglePalette(int paletteIndex);
    void slotToggleAllPalettes();
    void showAllPalettes(bool shown);

    /**
     * Restores the palette configuration to the default layout, i.e, the layout
     * preferred by each docker.
     */
    virtual void slotReset();

    /**
     * Reset the font for all palettes
     */
    virtual void slotResetFont();
     

protected:

    bool eventFilter( QObject *o, QEvent *e );

private:


    /**
     * Saves the current palette configuration to the application config object.
     */
    virtual void save();


private:

    KoView                  * m_view;
    KActionCollection       * m_actionCollection;
    KActionMenu             * m_viewActionMenu;
    KToggleAction           * m_toggleShowHidePalettes;
    enumKoDockability         m_dockability;
    
    QStringList             * m_widgetNames;

    Q3Dict<QWidget>          * m_widgets;
    Q3Dict<KoPalette>        * m_palettes;
    Q3ValueStack<QString>   m_hiddenWidgets; // names of widgets actively hidden by hide all
    Q3Dict<KToggleAction>    * m_actions;
    QSignalMapper           * m_mapper;

    QMap<QString, QString>  * m_defaultMapping; // widget to docker
    QStringList               m_defaultPaletteOrder; // Order of palette creation
    QStringList               m_defaultWidgetOrder; // Order of widget addition
    QMap<QString, QString>  * m_currentMapping; // widget to docker

    bool m_setFixedWidth;
    int m_fixedWidth;
};

#endif
