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
   the Free Software Foundation, Inc.,51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <qapplication.h>
#include <q3dict.h>
#include <qwidget.h>
#include <qobject.h>
#include <qevent.h>

#include <kparts/event.h>
#include <kapplication.h>
#include <kmenu.h>
#include <kaction.h>
#include <kactionclasses.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>

#include "KoView.h"
#include "KoMainWindow.h"

#include "kopalette.h"
#include "kotabpalette.h"
#include "kotoolboxpalette.h"
#include "KoIconTabPalette.h"

#include "kopalettemanager.h"

#include <QDesktopWidget>

KoPaletteManager::KoPaletteManager(KoView * view, KActionCollection *ac, const char * name)
    : QObject(view, name)
{

    m_view = view;
    m_view->installEventFilter(this);
    m_actionCollection = ac;

    m_actions = new Q3Dict<KToggleAction>();
    m_widgets = new Q3Dict<QWidget>();
    m_palettes = new Q3Dict<KoPalette>();
    m_palettes->setAutoDelete(true);
    m_defaultMapping = new QMap<QString, QString>();
    m_currentMapping = new QMap<QString, QString>();
    m_fixedWidth = 0;
    m_setFixedWidth = false;
    
    m_widgetNames = new QStringList();

    m_mapper = new QSignalMapper(this);
    connect(m_mapper, SIGNAL(mapped(int)), this, SLOT(slotTogglePalette(int)));
    m_viewActionMenu = new KActionMenu(i18n("Palettes"), m_actionCollection, "view_palette_action_menu");

    KConfig * cfg = KGlobal::config();
    cfg->setGroup("palettes");

    bool palettesShown = cfg->readEntry("palettesshown", true);
    KToggleAction * m_toggleShowHidePalettes;

    if ( palettesShown) {
        m_toggleShowHidePalettes = new KToggleAction(i18n("Hide All Palette Windows"),
                                    Qt::CTRL+Qt::SHIFT+Qt::Key_H, this,
                                    SLOT(slotToggleAllPalettes()),
                                    m_actionCollection, "toggleAllPaletteWindows");

        m_toggleShowHidePalettes->setCheckedState(i18n("Show Palette Windows Again"));
    }
    else {
        m_toggleShowHidePalettes = new KToggleAction(i18n("Show Palette Windows Again"),
                                    Qt::CTRL+Qt::SHIFT+Qt::Key_H, this,
                                    SLOT(slotToggleAllPalettes()),
                                    m_actionCollection, "toggleAllPaletteWindows");

        m_toggleShowHidePalettes->setCheckedState(i18n("Hide All Palette Windows"));
    }
    m_viewActionMenu->insert(m_toggleShowHidePalettes);

    // Recreate the palettes in the saved order
    QStringList paletteList = QStringList::split(",", cfg->readEntry("palettes"));
    QStringList::iterator it;
    for (it = paletteList.begin(); it != paletteList.end(); ++it) {
        if (cfg->hasGroup("palette-" + (*it))) {

            cfg->setGroup("palette-" + (*it));

            enumKoPaletteStyle style = (enumKoPaletteStyle)cfg->readEntry("palettestyle", 0);
            QString caption = cfg->readEntry("caption", "");

            createPalette((*it), caption, style);
        }
    }

/*
    KAction * a = new KAction(i18n("Restore Palettes"),
                  0, this,
                  SLOT(slotReset()),
                  this, "restorePalettes");
    m_viewActionMenu->insert(a);
*/
    m_viewActionMenu->popupMenu()->insertSeparator();
}


KoPaletteManager::~KoPaletteManager()
{
    save();

    delete m_viewActionMenu;
    delete m_widgetNames;
    delete m_widgets;
    delete m_palettes;
    delete m_actions;
    delete m_mapper;
    delete m_defaultMapping;
    delete m_currentMapping;
}

void KoPaletteManager::addWidget(QWidget * widget,
                                 const QString & name,
                                 const QString & paletteName,
                                 int position,
                                 enumKoPaletteStyle style,
                                 bool shown)
{

    if (!widget) return;

    QString pname = paletteName;

    QWidget * w = m_widgets->find(name);
    if (w != 0 )
    {
        removeWidget(name);
    }

    bool visible = true;

    KConfig * cfg = KGlobal::config();
    if (cfg->hasGroup("palettetab-" + name)) {
        cfg->setGroup("palettetab-" + name);

        pname = cfg->readEntry("docker");
        visible = cfg->readEntry("visible",true);
    }

    KoPalette * palette = m_palettes->find(pname);

    if (palette == 0) {
        palette = createPalette(pname, widget->caption(), style);
        m_defaultPaletteOrder.append(pname+ "," + QString::number(style));
    }

    KToggleAction * a;
    a = new KToggleAction(i18n("Show %1",widget->caption()), 0, m_mapper, SLOT(map()), m_actionCollection);
    a->setCheckedState(i18n("Hide %1",widget->caption()));

    m_mapper->setMapping(a, m_widgetNames->count()); // This is the position at which we'll insert the action
    m_actions->insert( name, a );
    m_viewActionMenu->insert(a);

    palette->plug(widget, name, position);

    m_widgets->insert(name, widget);

    // Default mappings ready for restoring
    m_defaultMapping->insert(name, pname);
    m_defaultWidgetOrder.append(name);

    // Find out the hidden state
    if(m_widgetNames->contains(name))
    {

        // The widget has already been added (and removed) during this session
        if(m_hiddenWidgets.contains(name))
            palette->hidePage( widget );
        else
        {
            a->setChecked(true);
            palette->showPage( widget );
        }
    }
    else
    {
        cfg->setGroup("palettes");
        if( cfg->readEntry("palettesshown", shown))
        {
            if (visible)
            {
                a->setChecked(true);
                palette->showPage( widget );
            }
            else
                palette->hidePage( widget );
        }
        else
        {
            if (visible)
                m_hiddenWidgets.push(name);
            palette->hidePage( widget );
        }
    }

    // Fill the current mappings
    m_widgetNames->append(name);
    m_currentMapping->insert(name, pname);
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
            createPalette(paletteName, "", PALETTE_DOCKER);
        }

        p->plug(w, widgetName, -1); // XXX: This is not good: we should add it in the same place as originally placed
        m_widgetNames->append(widgetName);
        m_currentMapping->insert(widgetName, paletteName);
    }
}

void KoPaletteManager::slotResetFont()
{
    Q3DictIterator<KoPalette> it(*m_palettes);
    for (; it.current(); ++it) {
        it.current()->resetFont();
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

    KToggleAction * a = m_actions->find(name);
    a->setChecked(true);
}

void KoPaletteManager::hideWidget(const QString & name)
{
    QWidget * w = m_widgets->find(name);
    if (!w) return;

    QString pname = *m_currentMapping->find(name);
    if (pname.isNull()) return;

    KoPalette * p = m_palettes->find(pname);
    p->hidePage(w);

    KToggleAction * a = m_actions->find(name);
    a->setChecked(false);
}

void KoPaletteManager::removeWidget(const QString & name)
{
    QString palette = *(m_currentMapping->find(name));
    if (palette.isNull()) return;

    QWidget * w = m_widgets->find(name);
    if (!w) return;

    KoPalette * p = m_palettes->find(palette);
    if (!p) return;

    p->showPage(w);
    p->unplug(w);
    m_widgets->remove(name);
    m_currentMapping->remove(name);

    KAction * a = m_actions->take(name);
    m_viewActionMenu->remove(a);
    m_actionCollection->remove(a);
}

KoPalette * KoPaletteManager::createPalette(const QString & name, const QString & caption, enumKoPaletteStyle style)
{
    Q_ASSERT(m_view);
    KoPalette * palette = 0;

    
    palette = m_palettes->find(name);
    if (palette) return palette;


    switch (style) {
        case (PALETTE_DOCKER):
            palette = new KoTabPalette(m_view, name.latin1());
            break;
        case (PALETTE_TOOLBOX):
            palette = new KoToolBoxPalette(m_view, name.latin1());
            break;
        case (PALETTE_ICONTABS):
            palette = new KoIconTabPalette(m_view, name.latin1());
            break;
        default:
            // This is a custom palette that we cannot create
            return 0;
    };

    if(!palette) return 0;

    if (m_setFixedWidth)
        palette->setFixedWidth(m_fixedWidth);
    
    palette->setCaption(caption);
    m_palettes->insert(name, palette);
    placePalette(name);

    return palette;
}

void KoPaletteManager::placePalette(const QString & name, Qt::DockWidgetArea location)
{
    Q_ASSERT(!name.isNull());
    KoPalette * palette = m_palettes->find(name);

    if (!palette) return;

    //XXX:  Check whether this name occurs in the config list, retrieve the location, set the location
    KConfig * cfg = KGlobal::config();

    if (cfg->hasGroup("palette-" + name)) {
        cfg->setGroup("palette-" + name);
        QString dockarea = cfg->readEntry("dockarea", "right");
        QString caption = cfg->readEntry("caption", "");
        int height = cfg->readEntry("height", 120);
        int place = cfg->readEntry("place", 0);
        int width = cfg->readEntry("width", 200);
        int x = cfg->readEntry("x", 0);
        int y = cfg->readEntry("y", 0);
        int offset = cfg->readEntry("offset", 0);
        palette->setGeometry(x, y, width, height);
//        palette->setOffset(offset); TODO Port this somehow
        if (dockarea == "left" && place == 0) {
            location = Qt::LeftDockWidgetArea;
        }
        else if (dockarea == "right" && place == 0) {
            location = Qt::RightDockWidgetArea;
        }
#if 0
        else {
            location = Qt::DockTornOff;
        }
#endif
    }

    cfg->setGroup("");
    m_dockability = (enumKoDockability) cfg->readEntry("palettesdockability",0);

    // Left and right may accept docks. The height of the screen is important
    int h = qApp->desktop()->height();
    switch (m_dockability) {
        case (DOCK_ENABLED):
	    palette->setAllowedAreas(Qt::LeftDockWidgetArea |
                                     Qt::RightDockWidgetArea);
            break;
        case (DOCK_DISABLED):
	    palette->setAllowedAreas(0);
	    palette->setFloating(true);
            break;
        case (DOCK_SMART):
            if (h > 768) {
		palette->setAllowedAreas(Qt::LeftDockWidgetArea |
                                         Qt::RightDockWidgetArea);
            }
            else {
		palette->setAllowedAreas(0);
		palette->setFloating(true);
           }
            break;
    }

    m_view->mainWindow()->addDockWidget(location, palette);
}

void KoPaletteManager::addPalette(KoPalette * palette, const QString & name, Qt::DockWidgetArea location)
{
    Q_ASSERT(palette);
    Q_ASSERT(!name.isNull());

    m_palettes->insert(name, palette);
    placePalette(name, location);
}

void KoPaletteManager::slotTogglePalette(int paletteIndex)
{
    // Toggle the right palette
    QString name = m_widgetNames->at(paletteIndex);
    QWidget * w = m_widgets->find(name);
    QString pname = *m_currentMapping->find(name);
    KoPalette * p = m_palettes->find(pname);
    p->togglePageHidden( w );

    m_hiddenWidgets.clear();
}

void KoPaletteManager::slotToggleAllPalettes()
{
    if( ! m_hiddenWidgets.isEmpty())
    {
        // Restore previous visibility state
        while(!m_hiddenWidgets.isEmpty())
        {
            QString name = m_hiddenWidgets.pop();
            QWidget *w = m_widgets->find(name);
            KToggleAction * a = m_actions->find(name);
            a->setChecked(true);

            QString pname = *m_currentMapping->find(name);
            KoPalette * p = m_palettes->find(pname);
            p->showPage(w);
        }
    }
    else
    {
        // Save hidden state and hide all palettes
        m_hiddenWidgets.clear();
        Q3DictIterator<QWidget> it(*m_widgets);
        for (; it.current(); ++it)
        {
            KToggleAction * a = m_actions->find(it.currentKey());
            if(a->isChecked())
            {
                a->setChecked(false);
                m_hiddenWidgets.push(it.currentKey());

                QString pname = *m_currentMapping->find(it.currentKey());
                KoPalette * p = m_palettes->find(pname);
                p->hidePage(it.current());
            }
        }
    }
}

void KoPaletteManager::showAllPalettes(bool shown)
{
    Q3DictIterator<KoPalette> it(*m_palettes);
    for (; it.current(); ++it) {
        it.current()->makeVisible(shown);
    }
}

bool KoPaletteManager::eventFilter( QObject *o, QEvent *e )
{
    if (o != m_view) return false;

    if(e && e->type() == (QEvent::User + 42)) {
         KParts::PartActivateEvent * pae = dynamic_cast<KParts::PartActivateEvent *>(e);
         if(pae && pae->widget() && pae->widget() == m_view) {
            if (pae->activated()) {
                showAllPalettes( true );
            }
            else {
                showAllPalettes( false );
            }
         }
    }
    return false;
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

    if (!m_view) return;
    if (!m_view->mainWindow()) return;

    KConfig * cfg = KGlobal::config();
    Q_ASSERT(cfg);
    cfg->setGroup("");

    QString widgets;

    // Save the list of palettes
    Q3DictIterator<KoPalette> itP(*m_palettes);

    QStringList paletteList;

    for (; itP.current(); ++itP) {

        KoPalette * p = itP.current();

        cfg->setGroup("palette-" + itP.currentKey());

        if ( m_view->mainWindow()->dockWidgetArea (p) == Qt::LeftDockWidgetArea) {
            cfg->writeEntry("dockarea", "left");
        }
        else {
            cfg->writeEntry("dockarea", "right");
        }
//        cfg->writeEntry("place", (int)p->place()); TODO Port this somehow
        cfg->writeEntry("x", p->x());
        cfg->writeEntry("y", p->y());
        cfg->writeEntry("height", p->height());
        cfg->writeEntry("width", p->width());
        cfg->writeEntry("palettestyle", (int)p->style());
        cfg->writeEntry("caption", p->caption());
//        cfg->writeEntry("offset", p->offset()); TODO Port this somehow

        // XXX: I dare say that it is immediately visible that I never have had
        //      any formal training in algorithms. BSAR.
        if (paletteList.isEmpty()) {
            paletteList.append(itP.currentKey());
        }
        else {
            QStringList::iterator it;
            bool inserted = false;
            for (it = paletteList.begin(); it != paletteList.end(); ++it) {
                KoPalette * p2 = m_palettes->find((*it));
                if (p2->y() > p->y()) {
                    paletteList.insert(it, itP.currentKey());
                    inserted = true;
                    break;
                }
            }
            if (!inserted) {
                paletteList.append(itP.currentKey());
            }
        }
    }

    cfg->setGroup("palettes");

    cfg->writeEntry("palettes", paletteList.join(","));
    bool palettesShown = m_hiddenWidgets.isEmpty();
    cfg->writeEntry("palettesshown", palettesShown);

    Q3DictIterator<QWidget> itW(*m_widgets);
    for (; itW.current(); ++itW) {
        cfg->setGroup("palettetab-" + itW.currentKey());
        QString pname = *m_currentMapping->find(itW.currentKey());
        KoPalette * p = m_palettes->find(pname);
        QWidget * w = itW.current();
        cfg->writeEntry("docker", pname);

        if(palettesShown)
            cfg->writeEntry("visible", !p->isHidden(w));
        else
            if(m_hiddenWidgets.contains(itW.currentKey()))
               cfg->writeEntry("visible", true);
            else
                cfg->writeEntry("visible", false);
    }
}

void KoPaletteManager::setFixedWidth(int w)
{
    m_fixedWidth = w;
    m_setFixedWidth = true;
}

#include "kopalettemanager.moc"
