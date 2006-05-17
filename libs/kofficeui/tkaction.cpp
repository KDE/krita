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

#include <QLabel>
#include <QLayout>
#include <QToolBar>
#include <QPixmap>
#include <Q3HBoxLayout>

#include <ktoolbar.h>
#include <kiconloader.h>

#if 0
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
#endif

TKAction::TKAction(KActionCollection* parent, const char* name)
: KAction( "", 0, parent, name )
{
  m_imode = TK::IconOnly;
  
  setToolBarWidgetFactory(this);
}

TKAction::~TKAction()
{
}

QWidget* TKAction::createToolBarWidget(QToolBar* parent)
{
  TKToolBarButton* button = new TKToolBarButton(
		  icon().pixmap(parent->iconSize()), iconText(),
		  parent);
  button->setIconMode(m_imode);
  initToolBarButton(button);

  return button;
}

#if 0
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
#endif

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
#if 0
  SET_FOR_ALL_CONTAINER(TKToolBarButton,setIconMode,mode)
#endif
}

void TKAction::setText(const QString& text)
{
  KAction::setText(text);
  updateLayout();
}

void TKAction::setIcon(const QString& icon)
{
  KAction::setIcon(KIcon(icon));
  updateLayout();
}

void TKAction::updateLayout()
{
#if 0
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
#endif
}

QWidget* TKAction::createLayout(QWidget* parent, QWidget* children)
{
  QWidget* base = new QWidget(parent);
  QLabel* textLabel = new QLabel(base);
  textLabel->setMinimumHeight(1);
  QLabel* pixLabel = new QLabel(base);
  children->setParent(base);
  children->setObjectName("widget");
  Q3HBoxLayout* layout = new Q3HBoxLayout(base,0,3);
  layout->setSizeConstraint(QLayout::SetMinimumSize);
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

#if 0
  if (hasIcon())
    pix = iconSet(K3Icon::Small).pixmap();

  if (!icon().isEmpty())
    pix = BarIcon(icon());
#endif

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
TKBaseSelectAction::TKBaseSelectAction( KActionCollection* parent, const char* name )
: TKAction(parent,name)
{
  m_current = 0;
  m_editable = false;
}

TKBaseSelectAction::~TKBaseSelectAction()
{
}

QWidget* TKBaseSelectAction::createToolBarWidget(QToolBar* parent)
{
  TKComboBox* combo = new TKComboBox(m_editable, parent);
  initComboBox(combo);
  combo->setMinimumWidth(combo->sizeHint().width());
  QWidget* base = createLayout(parent, combo);

  setCurrentItem(currentItem());
  
  return base;
}

#if 0
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
#endif

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
#if 0
  SET_FOR_ALL_CONTAINER(TKComboBox,setEditable,editable)
#endif
}

bool TKBaseSelectAction::isEditable()
{
  return m_editable;
}

void TKBaseSelectAction::setCurrentItem(int index)
{
  m_current = index;
#if 0
  SET_FOR_ALL_CONTAINER(TKComboBox,setCurrentItem,index)
#endif
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
TKSelectAction::TKSelectAction( KActionCollection* parent, const char* name )
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

#if 0
  SET_FOR_ALL_CONTAINER(TKComboBox,clear, )
  SET_FOR_ALL_CONTAINER(TKComboBox,insertStringList,lst)
#endif

  // Disable if empty and not editable
  setEnabled ( lst.count() > 0 || m_editable );
}

QStringList TKSelectAction::items() const
{
  return m_list;
}

void TKSelectAction::clear()
{
#if 0
  SET_FOR_ALL_CONTAINER(TKComboBox,clear, )
#endif
}

void TKSelectAction::setEditText(const QString& /*text*/)
{
#if 0
  SET_FOR_ALL_CONTAINER(TKComboBox,setEditText,text)
#endif
}

#undef SET_FOR_ALL_CONTAINER
#include "tkaction.moc"
