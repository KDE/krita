/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000 theKompany.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "tkaction.h"
#include "tktoolbarbutton.h"
#include "tkcombobox.h"

#include <qlabel.h>
#include <qlayout.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3HBoxLayout>

#include <ktoolbar.h>
#include <kiconloader.h>

#define SET_FOR_ALL_CONTAINER(WIDGET_TYPE,METHOD_NAME,VALUE)             \
  for( int id = 0; id < containerCount(); ++id ) {                       \
    QWidget* w = container(id);                                          \
    if ( w->inherits("KToolBar") ) {                                     \
      QWidget* r = static_cast<KToolBar*>(w)->getWidget(itemId(id));     \
      if (qstrcmp(r->name(),"KTToolBarLayout")==0)                       \
        r = (QWidget*)r->child("widget");                                \
      if ( r && r->inherits(#WIDGET_TYPE) ) {                            \
        WIDGET_TYPE* b = static_cast<WIDGET_TYPE*>(r);                   \
        b->METHOD_NAME(VALUE);                                         \
      }                                                                  \
    }                                                                    \
  }

TKAction::TKAction(QObject* parent, const char* name)
: KAction( "", 0, parent, name )
{
  m_imode = TK::IconOnly;
}

TKAction::~TKAction()
{
}

int TKAction::plug(QWidget* widget, int index)
{
  if ( widget->inherits("KToolBar") ) {
    KToolBar* bar = static_cast<KToolBar*>(widget);
    int id_ = KAction::getToolButtonID();
    KInstance *instance;

    if ( parentCollection() )
      instance = parentCollection()->instance();
    else
      instance = KGlobal::instance();

    TKToolBarButton* b = new TKToolBarButton(icon(),plainText(),bar,name(),instance);
    // we don't need clicked() and buttonClicked(), do we?
    // connect(b,SIGNAL(clicked()),SLOT(slotActivated()));
    b->setIconMode(m_imode);
    initToolBarButton(b);

    bar->insertWidget( id_, 100, b, index );
    addContainer(bar,id_);
    connect( bar, SIGNAL(destroyed()), this, SLOT(slotDestroyed()) );

    return containerCount() - 1;
  }
  return KAction::plug(widget,index);
}

void TKAction::initToolBarButton(TKToolBarButton* button)
{
  connect(button,SIGNAL(buttonClicked()),SLOT(slotActivated()));
}

TK::IconMode TKAction::iconMode()
{
  return m_imode;
}

void TKAction::setIconMode(TK::IconMode mode)
{
  m_imode = mode;
  SET_FOR_ALL_CONTAINER(TKToolBarButton,setIconMode,mode)
}

void TKAction::setText(const QString& text)
{
  KAction::setText(text);
  updateLayout();
}

void TKAction::setIcon(const QString& icon)
{
  KAction::setIcon(icon);
  updateLayout();
}

void TKAction::updateLayout()
{
  int len = containerCount();
  for( int id = 0; id < len; ++id ) {
    QWidget* w = container( id );
    if (w->inherits("KToolBar")) {
      QWidget* r = static_cast<KToolBar*>(w)->getWidget(itemId(id));
      if (qstrcmp(r->name(),"KTToolBarLayout")==0) {
        updateLayout(r);
      }
    }
  }
}

QWidget* TKAction::createLayout(QWidget* parent, QWidget* children)
{
  QWidget* base = new QWidget(parent,"KTToolBarLayout");
  QLabel* textLabel = new QLabel(base,"text");
  textLabel->setMinimumHeight(1);
  QLabel* pixLabel = new QLabel(base,"pixmap");
  children->reparent(base,QPoint(0,0));
  children->setName("widget");
  Q3HBoxLayout* layout = new Q3HBoxLayout(base,0,3);
  layout->setResizeMode(QLayout::SetMinimumSize);
  layout->addWidget(textLabel);
  layout->addWidget(pixLabel);
  layout->addWidget(children,1);

  updateLayout(base);
  return base;
}

void TKAction::updateLayout(QWidget* base)
{
  QLabel* textLabel = (QLabel*)base->child("text");
  QLabel* pixLabel = (QLabel*)base->child("pixmap");
  QWidget* w = (QWidget*)base->child("widget");

  if (!textLabel || !pixLabel || !w)
    return;

  if (!text().isEmpty() && m_imode != TK::IconOnly ) {
    textLabel->setText(text());
    textLabel->show();
  } else
    textLabel->hide();

  QPixmap pix;
  if (hasIcon())
    pix = iconSet(K3Icon::Small).pixmap();

  if (!icon().isEmpty())
    pix = BarIcon(icon());

  if (!pix.isNull() && m_imode != TK::TextOnly) {
    pixLabel->setPixmap(pix);
    pixLabel->show();
  } else
    pixLabel->hide();

  base->setFixedWidth( w->sizeHint().width() +
                       (textLabel->isVisible() ? textLabel->sizeHint().width():0) +
                       (pixLabel->isVisible() ? pixLabel->sizeHint().width():0) );
}
/******************************************************************************/
TKBaseSelectAction::TKBaseSelectAction( QObject* parent, const char* name )
: TKAction(parent,name)
{
  m_current = 0;
  m_editable = false;
}

TKBaseSelectAction::~TKBaseSelectAction()
{
}

int TKBaseSelectAction::plug(QWidget* widget, int index)
{
  if ( widget->inherits("KToolBar") )
  {
    KToolBar* bar = static_cast<KToolBar*>( widget );
    int id_ = KAction::getToolButtonID();

    TKComboBox* cb = new TKComboBox(m_editable,bar);
    initComboBox(cb);
    cb->setMinimumWidth( cb->sizeHint().width() );
    QWidget* base = createLayout(bar,cb);

    bar->insertWidget( id_, 100, base, index );
    addContainer( bar, id_ );

    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    setCurrentItem(currentItem());

    return containerCount() - 1;
  }
  return -1;
}

int TKBaseSelectAction::currentItem()
{
  return m_current;
}

void TKBaseSelectAction::initComboBox(TKComboBox* cb)
{
  connect(cb,SIGNAL(activated(int)),SLOT(slotActivated(int)));
}

void TKBaseSelectAction::setEditable(bool editable)
{
  m_editable = editable;
  SET_FOR_ALL_CONTAINER(TKComboBox,setEditable,editable)
}

bool TKBaseSelectAction::isEditable()
{
  return m_editable;
}

void TKBaseSelectAction::setCurrentItem(int index)
{
  m_current = index;
  SET_FOR_ALL_CONTAINER(TKComboBox,setCurrentItem,index)
}

void TKBaseSelectAction::slotActivated(int id)
{
  if ( m_current == id )
    return;

  m_current = id;
  setCurrentItem(id);
  activate(id);
}

void TKBaseSelectAction::activate(int id)
{
  emit activated(id);
}
/******************************************************************************/
TKSelectAction::TKSelectAction( QObject* parent, const char* name )
: TKBaseSelectAction(parent,name)
{
}

TKSelectAction::~TKSelectAction()
{
}

void TKSelectAction::initComboBox(TKComboBox* cb)
{
  TKBaseSelectAction::initComboBox(cb);
  connect(cb,SIGNAL(activated(const QString&)),SLOT(slotActivated(const QString&)));
  cb->insertStringList(items());
}

void TKSelectAction::slotActivated(const QString& text)
{
  emit activated(text);
}

void TKSelectAction::setItems(const QStringList& lst )
{
  m_list = lst;
  m_current = -1;

  SET_FOR_ALL_CONTAINER(TKComboBox,clear, )
  SET_FOR_ALL_CONTAINER(TKComboBox,insertStringList,lst)

  // Disable if empty and not editable
  setEnabled ( lst.count() > 0 || m_editable );
}

QStringList TKSelectAction::items() const
{
  return m_list;
}

void TKSelectAction::clear()
{
  SET_FOR_ALL_CONTAINER(TKComboBox,clear, )
}

void TKSelectAction::setEditText(const QString& text)
{
  SET_FOR_ALL_CONTAINER(TKComboBox,setEditText,text)
}

#undef SET_FOR_ALL_CONTAINER
#include "tkaction.moc"
