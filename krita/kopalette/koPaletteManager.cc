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


#include <qdict.h>
#include <qwidget.h>
#include <qobject.h>

#include <kapplication.h>
#include <kdebug.h>

#include <koView.h>
#include <koMainWindow.h>

#include <koPalette.h>
#include <koTabPalette.h>
#include <koToolBoxPalette.h>

#include <koPaletteManager.h>


KoPaletteManager::KoPaletteManager(KoView * view, const char * name)
	: QObject(view, name)
{
	kdDebug() << "PaletteManager started: " << name << "\n";
	
	m_view = view;

	m_widgets = new QDict<QWidget>();
	
	m_palettes = new QDict<KoPalette>();
	m_palettes->setAutoDelete(true);
	
	m_defaultMapping = new QMap<QString, QString>();
	m_currentMapping = new QMap<QString, QString>();

}

KoPaletteManager::~KoPaletteManager()
{
	save();

	delete m_widgets;
	delete m_palettes;
	delete m_defaultMapping;
	delete m_currentMapping;
}

void KoPaletteManager::addWidget(QWidget * widget,
				 const QString & name, 
				 const QString & paletteName,
				 enumKoPaletteStyle style)
{
	kdDebug() << "Adding widget " << name << " (" << widget << ") to " << paletteName << "\n";
	
	Q_ASSERT(widget);

	if (!widget) return;
	
	QWidget * w = m_widgets->find(name);
	if (w != 0 )
	{
		removeWidget(name);
	}

	KoPalette * palette = m_palettes->find(paletteName);;
	
	if (palette == 0) {
		palette = createPalette(paletteName, widget->caption(), style);
	}

	if (palette == 0) {
		kdDebug() << "Tried to find or create palette " << paletteName << " for widget " << name << ", but failed\n";
	}


	palette->plug(widget, name);
	palette->showPage(widget);
	m_widgets->insert(name, widget);
	m_currentMapping->insert(name, paletteName);
}

QWidget * KoPaletteManager::widget(const QString & name)
{
	return m_widgets->find(name);
}
	 
void KoPaletteManager::showWidget(const QString & name)
{
	QWidget * w = m_widgets->find(name);
	if (!w) return;

	QString pname = *m_currentMapping->find(name);
	if (pname.isNull()) return;

	KoPalette * p = m_palettes->find(pname);
	p->showPage(w);
	
}

void KoPaletteManager::removeWidget(const QString & name)
{
	kdDebug() << "Removing widget " << name << "\n";
	QString palette = *(m_currentMapping->find(name));
	if (palette.isNull()) return;
	
	QWidget * w = m_widgets->find(name);
	if (!w) return;
	
	KoPalette * p = m_palettes->find(palette);
	if (!p) return;

	p->unplug(w);
	m_widgets->remove(name);
	m_currentMapping->remove(name);
	
}


void KoPaletteManager::save()
{
	// XXX: Save to the configuration
	KApplication *app = KApplication::kApplication();
	Q_ASSERT(app);

	KConfig * cfg = app -> config();
	Q_ASSERT(cfg);

	// For all palettes windows in the window
	// Save the name, caption, type, position and size of the palette
	// For all widgets in each palette
	// Save the palette name, name and caption of the widget

	kdDebug() << "Saving palette configuration\n";
}

void KoPaletteManager::reset()
{
	// Reset to the default settings
	kdDebug() << "Resetting palette configuration to application default\n";
}

KoPalette * KoPaletteManager::createPalette(const QString & name, const QString & caption, enumKoPaletteStyle style)
{
	kdDebug() << "Creating palette " << name << ", caption: " << caption << "\n";
	Q_ASSERT(m_view);

	KoPalette * palette = 0;
	switch (style) {
		case (PALETTE_DOCKER):
			palette = new KoTabPalette(m_view, name.latin1());
			break;
		case (PALETTE_TOOLBOX):
			palette = new KoToolBoxPalette(m_view, name.latin1());
			break;
	};

	if(!palette) return 0;
	
	palette->setCaption(caption);
	m_palettes->insert(name, palette);
	placePalette(name);
	
	return palette;
}

void KoPaletteManager::placePalette(const QString & name, Qt::Dock location)
{
	Q_ASSERT(!name.isNull());
	kdDebug() << "Placing palette " << name << "\n";
	
	KoPalette * palette = m_palettes->find(name);
	
	if (!palette) return;

	//XXX:  Check whether this name occurs in the config list, retrieve the location, set the location

	// For now, just add it to the right and compact the stack
	m_view->mainWindow()->addDockWindow(palette, location);
	m_view->mainWindow()->lineUpDockWindows();
	
}

void KoPaletteManager::addPalette(KoPalette * palette, const QString & name, Qt::Dock location)
{
	kdDebug() << "Adding palette " << name << "\n";
	Q_ASSERT(palette);
	Q_ASSERT(!name.isNull());
	
	m_palettes->insert(name, palette);
	placePalette(name, location);
}

#include "koPaletteManager.moc"
