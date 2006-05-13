/* This file is part of the KDE project
   Copyright (C) 2004 Peter Simonsson <psn@linux.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoSelectAction.h"

#include <QPixmap>
#include <QBitmap>

#include <qmenubar.h>
//Added by qt3to4:
#include <Q3PopupMenu>

#include <kmenu.h>
#include <kapplication.h>
#include <kdebug.h>
#include <ktoolbar.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kauthorized.h>

class KoSelectAction::KoSelectActionPrivate
{
  public:
    KoSelectActionPrivate()
    {
      m_popup = new KMenu(0L);
	  m_popup->setObjectName("KoLineStyleAction::popup");
      m_currentSelection = 0;
    }
    
    ~KoSelectActionPrivate()
    {
      delete m_popup;
      m_popup = 0;
    }
    
    KMenu* m_popup;
    int m_currentSelection;
};

KoSelectAction::KoSelectAction(const QString &text, const QString& icon,
  QObject* parent, const char* name) : KAction(text, icon, 0, parent, name)
{
  d = new KoSelectActionPrivate;
  setShowCurrentSelection(true);
  
  connect(popupMenu(), SIGNAL(activated(int)), this, SLOT(execute(int)));
}

KoSelectAction::KoSelectAction(const QString &text, const QString& icon, const QObject* receiver,
  const char* slot, QObject* parent, const char* name) : KAction(text, icon, 0, parent, name)
{
  d = new KoSelectActionPrivate;
  
  connect(this, SIGNAL(selectionChanged(int)), receiver, slot);
  connect(popupMenu(), SIGNAL(activated(int)), this, SLOT(execute(int)));
}

KoSelectAction::~KoSelectAction()
{
  delete d;
}

KMenu* KoSelectAction::popupMenu() const
{
  return d->m_popup;
}

void KoSelectAction::popup(const QPoint& global)
{
  popupMenu()->popup(global);
}

int KoSelectAction::plug(QWidget* widget, int index)
{
  // This function is copied from KActionMenu::plug
  if (kapp && !KAuthorized::authorizeKAction(name()))
    return -1;
  kDebug(129) << "KAction::plug( " << widget << ", " << index << " )" << endl; // remove -- ellis
  if ( widget->inherits("QPopupMenu") )
  {
    Q3PopupMenu* menu = static_cast<Q3PopupMenu*>( widget );
    int id;

    if ( hasIconSet() )
      id = menu->insertItem( iconSet(), text(), popupMenu(), -1, index );
    else
      id = menu->insertItem( kapp->iconLoader()->loadIcon(icon(), K3Icon::Small),
        text(), popupMenu(), -1, index );

    if ( !isEnabled() )
      menu->setItemEnabled( id, false );

    addContainer( menu, id );
    connect( menu, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }
  else if ( widget->inherits( "KToolBar" ) )
  {
    KToolBar *bar = static_cast<KToolBar *>( widget );

    int id_ = KAction::getToolButtonID();

    if ( icon().isEmpty() && !iconSet().isNull() ) {
      bar->insertButton( iconSet().pixmap(), id_, SIGNAL( clicked() ), this,
                          SLOT( slotActivated() ), isEnabled(), plainText(),
                          index );
    } else {
      KInstance *instance;

      if ( m_parentCollection ) {
        instance = m_parentCollection->instance();
      } else {
        instance = KGlobal::instance();
      }

      bar->insertButton( icon(), id_, SIGNAL( clicked() ), this,
                          SLOT( slotActivated() ), isEnabled(), plainText(),
                          index, instance );
    }

    addContainer( bar, id_ );

    if (!whatsThis().isEmpty())
      bar->getButton(id_)->setWhatsThis( whatsThis() );

    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    bar->getButton(id_)->setPopup(popupMenu(), true );

    return containerCount() - 1;
  }
  else if ( widget->inherits( "QMenuBar" ) )
  {
    QMenuBar *bar = static_cast<QMenuBar *>( widget );

    int id;

    id = bar->insertItem( text(), popupMenu(), -1, index );

    if ( !isEnabled() )
      bar->setItemEnabled( id, false );

    addContainer( bar, id );
    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }

  return -1;
}

void KoSelectAction::execute(int index)
{
  setCurrentSelection(index);
  emit selectionChanged(d->m_currentSelection);
}

int KoSelectAction::currentSelection()
{
  return d->m_currentSelection;
}

void KoSelectAction::setCurrentSelection(int index)
{
  if(popupMenu()->isCheckable()) {
    popupMenu()->setItemChecked(d->m_currentSelection, false);
    popupMenu()->setItemChecked(index, true);
  }

  d->m_currentSelection = index;
}

void KoSelectAction::setShowCurrentSelection(bool show)
{
  popupMenu()->setCheckable(show);
}

#include "KoSelectAction.moc"
