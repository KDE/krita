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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include <qdict.h>
#include <qwidget.h>
#include <qobject.h>

#include <kapplication.h>
#include <kpopupmenu.h>
#include <kaction.h>
#include <kactionclasses.h>
#include <kdebug.h>
#include <klocale.h>

#include <koView.h>
#include <koMainWindow.h>

#include <kopalette.h>
#include <kotabpalette.h>
#include <kotoolboxpalette.h>

#include <kopalettemanager.h>


KoPaletteManager::KoPaletteManager(KoView * view, KActionCollection *ac, const char * name)
    : QObject(view, name)
{
    
    m_view = view;
    m_actionCollection = ac;

    m_actions = new QDict<KToggleAction>();
    m_widgets = new QDict<QWidget>();
    m_palettes = new QDict<KoPalette>();
    m_palettes->setAutoDelete(true);
    m_defaultMapping = new QMap<QString, QString>();
    m_currentMapping = new QMap<QString, QString>();

    m_widgetNames = new QStringList();
    
    m_mapper = new QSignalMapper(this);
    connect(m_mapper, SIGNAL(mapped(int)), this, SLOT(slotTogglePalette(int)));
    m_viewActionMenu = new KActionMenu(i18n("Palettes"), m_actionCollection, "view_palette_action_menu");

    KToggleAction * m_toggleShowHidePalettes = new KToggleAction(i18n("Hide all Palette Windows"),
                                    "f9", this,
                                    SLOT(slotToggleAllPalettes()),
                                    this, "toggleAllPaletteWindows");

    m_allPalettesShown = true;
    m_toggleShowHidePalettes->setCheckedState(i18n("Show all Palette Windows"));
    m_viewActionMenu->insert(m_toggleShowHidePalettes);

#if 0
    KAction * a = new KAction(i18n("Restore Palettes"),
                  0, this, 
                  SLOT(slotReset()),
                  this, "restorePalettes");
    m_viewActionMenu->insert(a);
#endif 
    m_viewActionMenu->popupMenu()->insertSeparator();
    
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
                 int position,
                 enumKoPaletteStyle style)
{
    
    if (!widget) return;
    
    QWidget * w = m_widgets->find(name);
    if (w != 0 )
    {
        removeWidget(name);
    }

    KoPalette * palette = m_palettes->find(paletteName);
    
    if (palette == 0) {
        palette = createPalette(paletteName, widget->caption(), style);
        m_defaultPaletteOrder.append( paletteName + "," + QString::number(style));
    }


    KToggleAction * a = new KToggleAction(i18n("Hide") + " " + widget->caption(), 0, m_mapper, SLOT(map()), m_actionCollection);
    a->setCheckedState(i18n("Show") + " " + widget->caption());
    m_mapper->setMapping(a, m_actions->count()); // This is the position at which we'll insert the action
    m_actions->insert( name, a );
    m_viewActionMenu->insert(a);
    
    palette->plug(widget, name, position);
    palette->showPage(widget);
    m_widgets->insert(name, widget);

    // Default mappings ready for restoring
    m_defaultMapping->insert(name, paletteName);
    m_defaultWidgetOrder.append(name);
    
    // Fill the current mappings
    m_widgetNames->append(name);
    m_currentMapping->insert(name, paletteName);
}


void KoPaletteManager::slotReset()
{
    // Clear all old palettes
    m_palettes->setAutoDelete( true );
    m_palettes->clear();

    m_widgetNames->clear();

    // Recreate the palettewindows in the saved order
    QStringList::iterator it;
    for (it = m_defaultPaletteOrder.begin(); it != m_defaultPaletteOrder.end(); ++it) {
        QString s = *it;
        QString pname = s.section( ",", 0, 0 );;
        enumKoPaletteStyle style = (enumKoPaletteStyle)s.section(",", 1,1).toInt();
        createPalette(pname, "", style);
    }

    // Place all existing (that we didn't throw away!) tabs in the right palette and in the right order
    for (it = m_defaultWidgetOrder.begin(); it != m_defaultWidgetOrder.end(); ++it) {
    
        QString widgetName = *it;
        QWidget * w = m_widgets->find(widgetName);

        if (!w) {
            continue;
        }
        QString paletteName = *m_defaultMapping->find(widgetName);
        KoPalette * p = m_palettes->find(paletteName);
        
        if (p == 0) {
            // Funny -- we should have a consistent set of palettes without holes!
            p = createPalette(paletteName, "", PALETTE_DOCKER);
        }
        
        p->plug(w, widgetName, -1); // XXX: This is not good: we should add it in the same place as originally placed
        m_widgetNames->append(widgetName);
        m_currentMapping->insert(widgetName, paletteName);
    }
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
    QString palette = *(m_currentMapping->find(name));
    if (palette.isNull()) return;
    
    QWidget * w = m_widgets->find(name);
    if (!w) return;
    
    KoPalette * p = m_palettes->find(palette);
    if (!p) return;

    p->unplug(w);
    m_widgets->remove(name);
    m_currentMapping->remove(name);
    
    KAction * a = m_actions->take(name);
    m_viewActionMenu->remove(a);
    m_actionCollection->remove(a);
    
}


void KoPaletteManager::save()
{
    // XXX: Save to the configuration
    // We save:
    //   * which tab at which place in which palette 
    //   * which tab is hidden
    //   * whether a palette is floating or docked
    //   * dock location of a docked palette
    //   * float location of a floated palette
    //   * order in which the palettes are docked.
    KApplication *app = KApplication::kApplication();
    Q_ASSERT(app);

    KConfig * cfg = app -> config();
    Q_ASSERT(cfg);

    // For all palettes windows in the window

    QString palettes;
    
    for (QMap<QString,QString>::Iterator it = m_currentMapping->begin(); it != m_currentMapping->end(); ++it) {
        QString widgetName = it.key();
        QString paletteName = it.data();
        QWidget * w = m_widgets->find(widgetName);
        KoPalette * p = m_palettes->find(paletteName);
        bool hidden = p->isHidden(w);
        

        int i = p->indexOf(w);
#if 0
        kdDebug() << "Saving: " << paletteName
            << " pos: " << p->place() << "," << p->x() << "," << p->y() << "," << p->width() << "," << p->height()
            << ", widgetName " << widgetName
            << ", index " << i
            << ", hidden: " << hidden << "\n";
#endif            
        palettes.append(widgetName + ","
            + paletteName + ","
            + p->place() + ","
            + p->x() + ","
            + p->y() + ","
            + p->width() + ","
            + p->height() + ","
            + QString::number(i) + "," + QString::number(hidden ? 0:1) + ";");
    }
    
    cfg->writeEntry("palettes", palettes);

}

KoPalette * KoPaletteManager::createPalette(const QString & name, const QString & caption, enumKoPaletteStyle style)
{
    Q_ASSERT(m_view);

    KoPalette * palette = 0;
    switch (style) {
        case (PALETTE_DOCKER):
            palette = new KoTabPalette(m_view, name.latin1());
            break;
        case (PALETTE_TOOLBOX):
        default:
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
    
    KoPalette * palette = m_palettes->find(name);
    
    if (!palette) return;

    //XXX:  Check whether this name occurs in the config list, retrieve the location, set the location

    // For now, just add it to the right and compact the stack
    m_view->mainWindow()->addDockWindow(palette, location);
    m_view->mainWindow()->lineUpDockWindows();
    
}

void KoPaletteManager::addPalette(KoPalette * palette, const QString & name, Qt::Dock location)
{
    Q_ASSERT(palette);
    Q_ASSERT(!name.isNull());
    
    m_palettes->insert(name, palette);
    placePalette(name, location);
}

void KoPaletteManager::slotTogglePalette(int paletteIndex)
{
    // Toggle the right palette
    QString name = *m_widgetNames->at(paletteIndex);
    QWidget * w = m_widgets->find(name);
    QString pname = *m_currentMapping->find(name);
    KoPalette * p = m_palettes->find(pname);
    p->togglePageHidden( w );
}

void KoPaletteManager::slotToggleAllPalettes()
{
    m_allPalettesShown= !m_allPalettesShown;
    QDictIterator<KoPalette> it(*m_palettes);
    for (; it.current(); ++it) {
        it.current()->makeVisible(m_allPalettesShown);
    }
}

#include "kopalettemanager.moc"
