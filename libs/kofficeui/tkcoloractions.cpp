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


#include "tkcoloractions.h"
#include "tktoolbarbutton.h"

#include <QLayout>
//Added by qt3to4:
#include <QPixmap>
#include <QPaintEvent>
#include <Q3GridLayout>
#include <QEvent>
#include <Q3Frame>
#include <QShowEvent>
#include <QMouseEvent>
#include <kcolordialog.h>
#include <ktoolbar.h>
#include <QPainter>
#include <QToolTip>

#include <klocale.h>
#include <kdebug.h>
#include <QApplication>

TKColorPopupMenu::TKColorPopupMenu( QWidget* parent, const char* name )
: KMenu(parent)
{
	setObjectName(name);
}

TKColorPopupMenu::~TKColorPopupMenu()
{
}

void TKColorPopupMenu::updateItemSize()
{
  styleChange(*style());
}
/****************************************************************************************/
class TKSelectColorActionPrivate
{
public:
    TKSelectColorActionPrivate()
    {
    }
    bool defaultColorMenu;
    QColor defaultColor;
};


TKSelectColorAction::TKSelectColorAction( const QString& text, Type type, KActionCollection* parent, const char* name, bool menuDefaultColor )
: TKAction(parent,name)
{
    d=new TKSelectColorActionPrivate();
    d->defaultColorMenu=menuDefaultColor;
    d->defaultColor=QColor();
    setText(text);
    m_type = type;
    init();
}

TKSelectColorAction::TKSelectColorAction( const QString& text, Type type,
                                          QObject* receiver, const char* slot,
                                          KActionCollection* parent,
					  const char* name,
					  bool menuDefaultColor)
: TKAction(parent,name)
{
    d=new TKSelectColorActionPrivate();
    d->defaultColorMenu=menuDefaultColor;
    d->defaultColor=QColor();
    setText(text);
    m_type = type;
    connect( this, SIGNAL( activated() ), receiver, slot );
    init();
}

void TKSelectColorAction::init()
{
  m_pStandardColor = new TKColorPanel();
  m_pRecentColor = new TKColorPanel();
#if 0

  connect(m_pStandardColor,SIGNAL(colorSelected(const QColor&)),SLOT(panelColorSelected(const QColor&)));
  connect(m_pStandardColor,SIGNAL(reject()),SLOT(panelReject()));
  connect(m_pRecentColor,SIGNAL(colorSelected(const QColor&)),SLOT(panelColorSelected(const QColor&)));
  connect(m_pRecentColor,SIGNAL(reject()),SLOT(panelReject()));

  m_pRecentColor->clear();
#endif
  m_pMenu = new TKColorPopupMenu();
#if 0
  m_pMenu->insertItem(m_pStandardColor);
  m_pMenu->insertSeparator();
  m_pMenu->insertItem(m_pRecentColor);
  m_pMenu->insertSeparator();
#endif
  switch (m_type) {
    case TextColor:
      m_pMenu->addAction(i18n("More Text Colors..."),this,SLOT(selectColorDialog()));
      setCurrentColor(Qt::black);
      setIcon("textcolor");
      break;
    case LineColor:
      m_pMenu->addAction(i18n("More Line Colors..."),this,SLOT(selectColorDialog()));
      setCurrentColor(Qt::black);
      setIcon("color_line");
      break;
    case FillColor:
      m_pMenu->addAction(i18n("More Fill Colors..."),this,SLOT(selectColorDialog()));
      setCurrentColor(Qt::white);
      setIcon("color_fill");
      break;
    case Color:
      break;
  }
  if(d->defaultColorMenu)
  {
      m_pMenu->addSeparator();
      m_pMenu->addAction(i18n("Default Color"),this,SLOT(defaultColor()));
  }

  connect(m_pStandardColor,SIGNAL(sizeChanged()),m_pMenu,SLOT(updateItemSize()));
  connect(m_pRecentColor,SIGNAL(sizeChanged()),m_pMenu,SLOT(updateItemSize()));
}

TKSelectColorAction::~TKSelectColorAction()
{
  delete m_pMenu;
  delete d;
}

void TKSelectColorAction::initToolBarButton(TKToolBarButton* b)
{
  b->setWhatsThis( whatsThis() );
  TKAction::initToolBarButton(b);
  b->setDelayedPopup( popupMenu() );
  updatePixmap(b);
  updatePixmap();
}

void TKSelectColorAction::defaultColor()
{
   m_pCurrentColor = d->defaultColor;
   emit activated();
}

void TKSelectColorAction::setDefaultColor(const QColor &_col)
{
    d->defaultColor=_col;
}

void TKSelectColorAction::updatePixmap()
{
#if 0
  for( int id = 0; id < containerCount(); ++id ) {
    QWidget* w = container(id);
    if ( w->inherits("KToolBar") ) {
      QWidget* r = static_cast<KToolBar*>(w)->getWidget(itemId(id));
      if ( r->inherits("TKToolBarButton") ) {
        updatePixmap(static_cast<TKToolBarButton*>(r));
      }
    }
    else if(w->inherits("QPopupMenu") ) {
        QPixmap pix =iconSet(K3Icon::Small).pixmap(QIcon::Automatic,QIcon::Active);
  if ( pix.isNull() )
      return;
        QPainter p(&pix);
        switch (m_type) {
            case TextColor:
                p.fillRect(QRect(0,12,16,5), m_pCurrentColor);
                break;
            case LineColor:
                p.fillRect(QRect(0,13,16,5), m_pCurrentColor);
                p.fillRect(QRect(3,12,1,1), m_pCurrentColor);
                break;
            case FillColor:
                p.fillRect(QRect(0,13,16,5), m_pCurrentColor);
                p.fillRect(QRect(1,10,5,3), m_pCurrentColor);
                break;
            case Color:
                break;
        }
        p.end();
        setIconSet( pix );
    }
  }
#endif
}

void TKSelectColorAction::updatePixmap(TKToolBarButton* b)
{
  if (!b)
    return;
  // Not much point in painting with an invalid color
  if (!m_pCurrentColor.isValid())
    return;
  QPixmap pix =b->getActivePixmap();
  QPainter p(&pix);
  switch (m_type) {
    case TextColor:
      p.fillRect(QRect(0,12,16,5), m_pCurrentColor);
      break;
    case LineColor:
      p.fillRect(QRect(0,13,16,5), m_pCurrentColor);
      p.fillRect(QRect(3,12,1,1), m_pCurrentColor);
      break;
    case FillColor:
      p.fillRect(QRect(0,13,16,5), m_pCurrentColor);
      p.fillRect(QRect(1,10,5,3), m_pCurrentColor);
      break;
    case Color:
      break;
  }
  p.end();
  b->setPixmap(pix);
}

void TKSelectColorAction::setCurrentColor( const QColor& color )
{
    if ( color == m_pCurrentColor )
        return;
    m_pCurrentColor = color;
    setActiveColor( color );
    m_pRecentColor->setActiveColor(color );
  updatePixmap();
}

void TKSelectColorAction::setActiveColor( const QColor& color )
{
  m_pStandardColor->setActiveColor(color);
}

void TKSelectColorAction::selectColorDialog()
{
    QColor c = color();

    if ( d->defaultColorMenu )
    {
        if ( KColorDialog::getColor(c,d->defaultColor, qApp->activeWindow()) == QDialog::Accepted )
        {
            setCurrentColor(c);
            m_pRecentColor->insertColor(m_pCurrentColor);
            activate();
        }

    }
    else
    {
        if ( KColorDialog::getColor(c, qApp->activeWindow()) == QDialog::Accepted )
        {
            setCurrentColor(c);
            m_pRecentColor->insertColor(m_pCurrentColor);
            activate();
        }
    }
}

// Called when activating the menu item
void TKSelectColorAction::slotActivated()
{
  //kDebug() << "TKSelectColorAction::slotActivated" << endl;
  selectColorDialog();
}

void TKSelectColorAction::activate()
{
  emit colorSelected(m_pCurrentColor);
  emit activated();
}

void TKSelectColorAction::panelColorSelected( const QColor& color )
{
  m_pMenu->hide();
  setCurrentColor(color);

  activate();
}

void TKSelectColorAction::panelReject()
{
  m_pMenu->hide();
}

class TKColorPanel::TKColorPanelPrivate
{
public:
  TKColorPanelPrivate()
  {
    panelCreated = false;
  }

  bool panelCreated;
};

/****************************************************************************************/
TKColorPanel::TKColorPanel( QWidget* parent, const char* name )
: QWidget(parent,name)
{
  d = new TKColorPanel::TKColorPanelPrivate();
  m_activeColor = Qt::black;

  //m_iX = 0;  // happens in setNumCols() -> resetGrid()
  //m_iY = 0;  // anyway, so...

  m_pLayout = 0L;
  setNumCols(15);
}

void TKColorPanel::setNumCols( int col )
{
  m_iWidth = col;
  resetGrid();

  Q3DictIterator<TKColorPanelButton> it(m_pColorDict);
  while ( it.current() ) {
    addToGrid(it.current());
    ++it;
  }
}

TKColorPanel::~TKColorPanel()
{
  delete d;
}

void TKColorPanel::resetGrid()
{
  m_iX = 0;
  m_iY = 0;

  delete m_pLayout;
  m_pLayout = new Q3GridLayout(this,0,m_iWidth+1,0,0);

  emit sizeChanged();
}

void TKColorPanel::clear()
{
  m_pColorDict.setAutoDelete(true);
  m_pColorDict.clear();
  m_pColorDict.setAutoDelete(false);
  d->panelCreated = true;  // we don't want to create the default
                           // panel anymore now (b/c of recent colors)
  resetGrid();
}

void TKColorPanel::insertColor( const QColor& color, const QString& text )
{
  if (m_pColorDict[color.name()])
    return;

  insertColor(color);
  m_pColorDict[color.name()]->setToolTip(text);
}

void TKColorPanel::insertColor( const QColor& color )
{
  if (m_pColorDict[color.name()])
    return;

  m_pLayout->setMargin(3);
  TKColorPanelButton* f = new TKColorPanelButton(color,this);
  m_pColorDict.insert(color.name(),f);
  if ( m_activeColor == color )
      f->setActive(true);

  connect(f,SIGNAL(selected(const QColor&)),SLOT(selected(const QColor&)));

  addToGrid(f);
}

void TKColorPanel::addToGrid( TKColorPanelButton* f )
{
  m_pLayout->addWidget(f,m_iY,m_iX);
  f->show();  // yeehaaaw! ugly hack (Werner)
  m_iX++;
  if ( m_iX == m_iWidth ) {
    m_iX = 0;
    m_iY++;
  }
  emit sizeChanged();
}

void TKColorPanel::setActiveColor( const QColor& color )
{
    TKColorPanelButton* b = m_pColorDict[m_activeColor.name()];
  if (b)
    b->setActive(false);

  m_activeColor = color;

  b = m_pColorDict[m_activeColor.name()];
  if (b)
    b->setActive(true);
}

void TKColorPanel::mouseReleaseEvent( QMouseEvent* )
{
  reject();
}

void TKColorPanel::showEvent( QShowEvent *e )
{
  if ( !d->panelCreated )
    fillPanel();
  QWidget::showEvent(e);
}

void TKColorPanel::selected( const QColor& color )
{
  emit colorSelected(color);
}

void TKColorPanel::fillPanel()
{
  d->panelCreated = true;
  blockSignals(true); // don't emit sizeChanged() all the time

  // ### TODO: names without space (e.g. red) are lower case in rgb.txt
  insertColor(QColor( 255,   0,   0 ), i18nc( "color", "Red"));
  insertColor(QColor( 255, 165,   0 ), i18nc( "color", "Orange"));
  insertColor(QColor( 255,   0, 255 ), i18nc( "color", "Magenta"));
  insertColor(QColor(   0,   0, 255 ), i18nc( "color", "Blue"));
  insertColor(QColor(   0, 255, 255 ), i18nc( "color", "Cyan"));
  insertColor(QColor(   0, 255,   0 ), i18nc( "color", "Green"));
  insertColor(QColor( 255, 255,   0 ), i18nc( "color", "Yellow"));
  insertColor(QColor( 165,  42,  42 ), i18nc( "color", "Brown"));
  insertColor(QColor( 139,   0,   0 ), i18nc( "color", "DarkRed"));
  insertColor(QColor( 255, 140,   0 ), i18nc( "color", "DarkOrange"));
  insertColor(QColor( 139,   0, 139 ), i18nc( "color", "DarkMagenta"));
  insertColor(QColor(   0,   0, 139 ), i18nc( "color", "DarkBlue"));
  insertColor(QColor(   0, 139, 139 ), i18nc( "color", "DarkCyan"));
  insertColor(QColor(   0, 100,   0 ), i18nc( "color", "DarkGreen"));
  insertColor(QColor( 130, 127,   0 ), i18nc( "color", "DarkYellow")); // ### not in rgb.txt

  insertColor(QColor( 255, 255, 255 ), i18nc( "color", "White"));
  insertColor(QColor( 229, 229, 229 ), i18nc( "color", "Gray 90%")); // ### not in rgb.txt
  insertColor(QColor( 204, 204, 204 ), i18nc( "color", "Gray 80%")); // ### not in rgb.txt
  insertColor(QColor( 178, 178, 178 ), i18nc( "color", "Gray 70%")); // ### not in rgb.txt
  insertColor(QColor( 153, 153, 153 ), i18nc( "color", "Gray 60%")); // ### not in rgb.txt
  insertColor(QColor( 127, 127, 127 ), i18nc( "color", "Gray 50%")); // ### not in rgb.txt
  insertColor(QColor( 102, 102, 102 ), i18nc( "color", "Gray 40%")); // ### not in rgb.txt
  insertColor(QColor(  76,  76,  76 ), i18nc( "color", "Gray 30%")); // ### not in rgb.txt
  insertColor(QColor(  51,  51,  51 ), i18nc( "color", "Gray 20%")); // ### not in rgb.txt
  insertColor(QColor(  25,  25,  25 ), i18nc( "color", "Gray 10%")); // ### not in rgb.txt
  insertColor(QColor(   0,   0,   0 ), i18nc( "color", "Black"));

  insertColor(QColor( 255, 255, 240 ), i18nc( "color", "Ivory"));
  insertColor(QColor( 255, 250, 250 ), i18nc( "color", "Snow"));
  insertColor(QColor( 245, 255, 250 ), i18nc( "color", "MintCream"));
  insertColor(QColor( 255, 250, 240 ), i18nc( "color", "FloralWhite"));
  insertColor(QColor( 255, 255, 224 ), i18nc( "color", "LightYellow"));
  insertColor(QColor( 240, 255, 255 ), i18nc( "color", "Azure"));
  insertColor(QColor( 248, 248, 255 ), i18nc( "color", "GhostWhite"));
  insertColor(QColor( 240, 255, 240 ), i18nc( "color", "Honeydew"));
  insertColor(QColor( 255, 245, 238 ), i18nc( "color", "Seashell"));
  insertColor(QColor( 240, 248, 255 ), i18nc( "color", "AliceBlue"));
  insertColor(QColor( 255, 248, 220 ), i18nc( "color", "Cornsilk"));
  insertColor(QColor( 255, 240, 245 ), i18nc( "color", "LavenderBlush"));
  insertColor(QColor( 253, 245, 230 ), i18nc( "color", "OldLace"));
  insertColor(QColor( 245, 245, 245 ), i18nc( "color", "WhiteSmoke"));
  insertColor(QColor( 255, 250, 205 ), i18nc( "color", "LemonChiffon"));
  insertColor(QColor( 224, 255, 255 ), i18nc( "color", "LightCyan"));
  insertColor(QColor( 250, 250, 210 ), i18nc( "color", "LightGoldenrodYellow"));
  insertColor(QColor( 250, 240, 230 ), i18nc( "color", "Linen"));
  insertColor(QColor( 245, 245, 220 ), i18nc( "color", "Beige"));
  insertColor(QColor( 255, 239, 213 ), i18nc( "color", "PapayaWhip"));
  insertColor(QColor( 255, 235, 205 ), i18nc( "color", "BlanchedAlmond"));
  insertColor(QColor( 250, 235, 215 ), i18nc( "color", "AntiqueWhite"));
  insertColor(QColor( 255, 228, 225 ), i18nc( "color", "MistyRose"));
  insertColor(QColor( 230, 230, 250 ), i18nc( "color", "Lavender"));
  insertColor(QColor( 255, 228, 196 ), i18nc( "color", "Bisque"));
  insertColor(QColor( 255, 228, 181 ), i18nc( "color", "Moccasin"));
  insertColor(QColor( 255, 222, 173 ), i18nc( "color", "NavajoWhite"));
  insertColor(QColor( 255, 218, 185 ), i18nc( "color", "PeachPuff"));
  insertColor(QColor( 238, 232, 170 ), i18nc( "color", "PaleGoldenrod"));
  insertColor(QColor( 245, 222, 179 ), i18nc( "color", "Wheat"));
  insertColor(QColor( 220, 220, 220 ), i18nc( "color", "Gainsboro"));
  insertColor(QColor( 240, 230, 140 ), i18nc( "color", "Khaki"));
  insertColor(QColor( 175, 238, 238 ), i18nc( "color", "PaleTurquoise"));
  insertColor(QColor( 255, 192, 203 ), i18nc( "color", "Pink"));
  insertColor(QColor( 238, 221, 130 ), i18nc( "color", "LightGoldenrod"));
  insertColor(QColor( 211, 211, 211 ), i18nc( "color", "LightGray"));
  insertColor(QColor( 255, 182, 193 ), i18nc( "color", "LightPink"));
  insertColor(QColor( 176, 224, 230 ), i18nc( "color", "PowderBlue"));
  insertColor(QColor( 127, 255, 212 ), i18nc( "color", "Aquamarine"));
  insertColor(QColor( 216, 191, 216 ), i18nc( "color", "Thistle"));
  insertColor(QColor( 173, 216, 230 ), i18nc( "color", "LightBlue"));
  insertColor(QColor( 152, 251, 152 ), i18nc( "color", "PaleGreen"));
  insertColor(QColor( 255, 215,   0 ), i18nc( "color", "Gold"));
  insertColor(QColor( 173, 255,  47 ), i18nc( "color", "GreenYellow"));
  insertColor(QColor( 176, 196, 222 ), i18nc( "color", "LightSteelBlue"));
  insertColor(QColor( 144, 238, 144 ), i18nc( "color", "LightGreen"));
  insertColor(QColor( 221, 160, 221 ), i18nc( "color", "Plum"));
  insertColor(QColor( 190, 190, 190 ), i18nc( "color", "Gray"));
  insertColor(QColor( 222, 184, 135 ), i18nc( "color", "BurlyWood"));
  insertColor(QColor( 135, 206, 250 ), i18nc( "color", "LightSkyblue"));
  insertColor(QColor( 255, 160, 122 ), i18nc( "color", "LightSalmon"));
  insertColor(QColor( 135, 206, 235 ), i18nc( "color", "SkyBlue"));
  insertColor(QColor( 210, 180, 140 ), i18nc( "color", "Tan"));
  insertColor(QColor( 238, 130, 238 ), i18nc( "color", "Violet"));
  insertColor(QColor( 244, 164,  96 ), i18nc( "color", "SandyBrown"));
  insertColor(QColor( 233, 150, 122 ), i18nc( "color", "DarkSalmon"));
  insertColor(QColor( 189, 183, 107 ), i18nc( "color", "DarkKhaki"));
  insertColor(QColor( 127, 255,   0 ), i18nc( "color", "Chartreuse"));
  insertColor(QColor( 169, 169, 169 ), i18nc( "color", "DarkGray"));
  insertColor(QColor( 124, 252,   0 ), i18nc( "color", "LawnGreen"));
  insertColor(QColor( 255, 105, 180 ), i18nc( "color", "HotPink"));
  insertColor(QColor( 250, 128, 114 ), i18nc( "color", "Salmon"));
  insertColor(QColor( 240, 128, 128 ), i18nc( "color", "LightCoral"));
  insertColor(QColor(  64, 224, 208 ), i18nc( "color", "Turquoise"));
  insertColor(QColor( 143, 188, 143 ), i18nc( "color", "DarkSeagreen"));
  insertColor(QColor( 218, 112, 214 ), i18nc( "color", "Orchid"));
  insertColor(QColor( 102, 205, 170 ), i18nc( "color", "MediumAquamarine"));
  insertColor(QColor( 255, 127,  80 ), i18nc( "color", "Coral"));
  insertColor(QColor( 154, 205,  50 ), i18nc( "color", "YellowGreen"));
  insertColor(QColor( 218, 165,  32 ), i18nc( "color", "Goldenrod"));
  insertColor(QColor(  72, 209, 204 ), i18nc( "color", "MediumTurquoise"));
  insertColor(QColor( 188, 143, 143 ), i18nc( "color", "RosyBrown"));
  insertColor(QColor( 219, 112, 147 ), i18nc( "color", "PaleVioletRed"));
  insertColor(QColor(   0, 250, 154 ), i18nc( "color", "MediumSpringGreen"));
  insertColor(QColor( 255,  99,  71 ), i18nc( "color", "Tomato"));
  insertColor(QColor(   0, 255, 127 ), i18nc( "color", "SpringGreen"));
  insertColor(QColor( 205, 133,  63 ), i18nc( "color", "Peru"));
  insertColor(QColor( 100, 149, 237 ), i18nc( "color", "CornflowerBlue"));
  insertColor(QColor( 132, 112, 255 ), i18nc( "color", "LightSlateBlue"));
  insertColor(QColor( 147, 112, 219 ), i18nc( "color", "MediumPurple"));
  insertColor(QColor( 186,  85, 211 ), i18nc( "color", "MediumOrchid"));
  insertColor(QColor(  95, 158, 160 ), i18nc( "color", "CadetBlue"));
  insertColor(QColor(   0, 206, 209 ), i18nc( "color", "DarkTurquoise"));
  insertColor(QColor(   0, 191, 255 ), i18nc( "color", "DeepSkyblue"));
  insertColor(QColor( 119, 136, 153 ), i18nc( "color", "LightSlateGray"));
  insertColor(QColor( 184, 134,  11 ), i18nc( "color", "DarkGoldenrod"));
  insertColor(QColor( 123, 104, 238 ), i18nc( "color", "MediumSlateBlue"));
  insertColor(QColor( 205,  92,  92 ), i18nc( "color", "IndianRed"));
  insertColor(QColor( 210, 105,  30 ), i18nc( "color", "Chocolate"));
  insertColor(QColor(  60, 179, 113 ), i18nc( "color", "MediumSeaGreen"));
  insertColor(QColor(  50, 205,  50 ), i18nc( "color", "LimeGreen"));
  insertColor(QColor(  32, 178, 170 ), i18nc( "color", "LightSeaGreen"));
  insertColor(QColor( 112, 128, 144 ), i18nc( "color", "SlateGray"));
  insertColor(QColor(  30, 144, 255 ), i18nc( "color", "DodgerBlue"));
  insertColor(QColor( 255,  69,   0 ), i18nc( "color", "OrangeRed"));
  insertColor(QColor( 255,  20, 147 ), i18nc( "color", "DeepPink"));
  insertColor(QColor(  70, 130, 180 ), i18nc( "color", "SteelBlue"));
  insertColor(QColor( 106,  90, 205 ), i18nc( "color", "SlateBlue"));
  insertColor(QColor( 107, 142,  35 ), i18nc( "color", "OliveDrab"));
  insertColor(QColor(  65, 105, 225 ), i18nc( "color", "RoyalBlue"));
  insertColor(QColor( 208,  32, 144 ), i18nc( "color", "VioletRed"));
  insertColor(QColor( 153,  50, 204 ), i18nc( "color", "DarkOrchid"));
  insertColor(QColor( 160,  32, 240 ), i18nc( "color", "Purple"));
  insertColor(QColor( 105, 105, 105 ), i18nc( "color", "DimGray"));
  insertColor(QColor( 138,  43, 226 ), i18nc( "color", "BlueViolet"));
  insertColor(QColor( 160,  82,  45 ), i18nc( "color", "Sienna"));
  insertColor(QColor( 199,  21, 133 ), i18nc( "color", "MediumVioletRed"));
  insertColor(QColor( 176,  48,  96 ), i18nc( "color", "Maroon"));
  insertColor(QColor(  46, 139,  87 ), i18nc( "color", "SeaGreen"));
  insertColor(QColor(  85, 107,  47 ), i18nc( "color", "DarkOliveGreen"));
  insertColor(QColor(  34, 139,  34 ), i18nc( "color", "ForestGreen"));
  insertColor(QColor( 139,  69,  19 ), i18nc( "color", "SaddleBrown"));
  insertColor(QColor( 148,   0, 211 ), i18nc( "color", "DarkViolet"));
  insertColor(QColor( 178,  34,  34 ), i18nc( "color", "FireBrick"));
  insertColor(QColor(  72,  61, 139 ), i18nc( "color", "DarkSlateBlue"));
  insertColor(QColor(  47,  79,  79 ), i18nc( "color", "DarkSlateGray"));
  insertColor(QColor(  25,  25, 112 ), i18nc( "color", "MidnightBlue"));
  insertColor(QColor(   0,   0, 205 ), i18nc( "color", "MediumBlue"));
  insertColor(QColor(   0,   0, 128 ), i18nc( "color", "Navy"));

  blockSignals(false);  // Signals allowed again
  emit sizeChanged();   // one call should be enough ;)
}

/****************************************************************************************/
TKColorPanelButton::TKColorPanelButton( const QColor& color, QWidget* parent, const char* name )
: Q3Frame(parent,name), m_Color(color), m_bActive(false)
{
  setFixedSize(16,16);
  setFrameStyle( NoFrame );
}

TKColorPanelButton::~TKColorPanelButton()
{
}

void TKColorPanelButton::enterEvent( QEvent* )
{
  if (!m_bActive)
    setFrameStyle( Panel | Sunken );
}

void TKColorPanelButton::leaveEvent( QEvent* )
{
  if (!m_bActive)
    setFrameStyle( NoFrame );
}

void TKColorPanelButton::paintEvent( QPaintEvent* ev )
{
  Q3Frame::paintEvent(ev);

  QPainter p(this);
  p.fillRect(2,2,12,12,m_Color);
  p.setPen(Qt::gray);
  p.drawRect(2,2,12,12);
  p.end();
}

void TKColorPanelButton::setActive( bool f )
{
  m_bActive = f;
  setFrameStyle( m_bActive ? Panel | Sunken : NoFrame );
}

void TKColorPanelButton::mouseReleaseEvent( QMouseEvent* )
{
  emit selected(m_Color);
}
#include "tkcoloractions.moc"
