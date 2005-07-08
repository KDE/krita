/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _KO_PALETTE_MANAGER_
#define _KO_PALETTE_MANAGER_

#include <qobject.h>
#include <qdockwindow.h>
#include <qstring.h>
#include <qmap.h>
#include <qdict.h>
#include <qwidget.h>
#include <qsignalmapper.h>
#include <qstringlist.h>


#include <koView.h>

class KoPalette;
class KActionMenu;
class KAction;
class KActionCollection;
class KToggleAction;

enum enumKoPaletteStyle {
#if 0   // XXX
 	PALETTE_SLIDER, // Sliding docker as in Kivio
#endif
	PALETTE_DOCKER, // QDockWindow based docker with tabs
	PALETTE_TOOLBOX, // QDockWindow based docker with a QToolBox
	PALETTE_SLIDER, // Kivio-style sliders
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
 * XXX: loading, saving and resetting of configuration
 * XXX: show/hide dockwindows menu entries
 * XXX: reset menu entry.
 */
class KoPaletteManager : public QObject {

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
	 */
	virtual void addWidget(QWidget * widget, const QString & name, const QString & paletteName, int position = -1, enumKoPaletteStyle style = PALETTE_DOCKER);

	/**
	 * Get a certain widget by name
	 */
	virtual QWidget * widget(const QString & name);

	/**
	 * Show a the specified palette wherever it currently is.
	 */
	virtual void showWidget(const QString & name);
	 
	/**
	 * Remove the widget with the specified name from whichever
	 * palette it is currently in. If it is the last widget in
	 * the palette, the palette is destroyed. If the name does
	 * not occur, nothing is done.
	 */
	virtual void removeWidget(const QString & name);

	/**
	 * Saves the current palette configuration to the application config object.
	 */
	virtual void save();


	/**
	 * Create a palette in the given style. with the given name and caption. If
	 * the palette already exists, nothing is done. 
	 */
	virtual KoPalette * createPalette(const QString & name, const QString & caption, enumKoPaletteStyle style = PALETTE_DOCKER);

	/**
	 * Move the specified palette to the specified location. If there is already
	 * a location for the palette in the saved settings, then move it there
	 * instead of the specified location.
	 */
	virtual void placePalette(const QString & name, Dock location = DockRight);

	/**
	 * Add a palette; this can be used to add palettes other than those in the two
	 * default styles.
	 */
	 virtual void addPalette(KoPalette * palette, const QString & name, Dock location = DockRight);

public slots:

	void slotTogglePalette(int paletteIndex);
	void slotToggleAllPalettes();
	/**
	 * Restores the palette configuration to the default layout, i.e, the layout 
	 * preferred by each docker.
	 */
	virtual void slotReset();



private:

	KoView                  * m_view;
	KActionCollection       * m_actionCollection;
	KActionMenu             * m_viewActionMenu;
	KToggleAction           * m_toggleShowHidePalettes;
	bool                    m_allPalettesShown;
	
	QStringList             * m_widgetNames;
	
	QDict<QWidget>          * m_widgets;
	QDict<KoPalette>        * m_palettes;
	QDict<KToggleAction>    * m_actions;
	QSignalMapper           * m_mapper;

	QMap<QString, QString>  * m_defaultMapping; // widget to docker
	QStringList             m_defaultPaletteOrder; // Order of palette creation
	QStringList             m_defaultWidgetOrder; // Order of widget addition
	QMap<QString, QString>  * m_currentMapping; // widget to docker
};

#endif
