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

#include <qwidget.h>
#include <qdockwindow.h>
#include <qvariant.h>
#include <qlabel.h>
#include <qtoolbutton.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qlayout.h>

#include <klocale.h>
#include <kglobalsettings.h>
#include <kaccelmanager.h>
#include <koView.h>

#include "kopalette.h"


static const unsigned char img_shadebutton [] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x13,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x90, 0x8c, 0x2d, 0xb5, 0x00, 0x00, 0x00,
    0xb2, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0xcd, 0x94, 0x51, 0x0a, 0xc4,
    0x20, 0x0c, 0x44, 0x93, 0x65, 0xbf, 0x05, 0x4b, 0xee, 0x99, 0x13, 0x94,
    0xd2, 0xde, 0xc0, 0x63, 0x16, 0xa5, 0x82, 0x1e, 0x20, 0xfb, 0xd5, 0xa5,
    0x54, 0xdb, 0x48, 0x71, 0x61, 0x03, 0x22, 0x04, 0xe6, 0x39, 0x43, 0x54,
    0xcc, 0x39, 0x0b, 0x74, 0xac, 0xb7, 0x48, 0x57, 0xde, 0x0f, 0x80, 0xe7,
    0x86, 0x31, 0x06, 0x5b, 0xc5, 0x29, 0xa5, 0xc2, 0x4d, 0xd5, 0x21, 0x33,
    0x37, 0xd9, 0x5e, 0x96, 0xa5, 0xe8, 0x5d, 0x46, 0x76, 0xce, 0xdd, 0x3a,
    0x65, 0x66, 0xa9, 0x69, 0x8b, 0xc8, 0x47, 0x81, 0x6a, 0xb1, 0x52, 0xb8,
    0x6d, 0x9b, 0x2a, 0xb4, 0xd6, 0x62, 0x8c, 0xb1, 0xe9, 0x00, 0x75, 0xca,
    0xc3, 0x30, 0xe0, 0xee, 0x76, 0x9a, 0x26, 0x15, 0xf8, 0x6a, 0x85, 0x01,
    0x00, 0xcc, 0xf3, 0xac, 0x03, 0x45, 0x04, 0x6a, 0xeb, 0x0c, 0x3b, 0x42,
    0xaf, 0x34, 0x22, 0x02, 0xe8, 0xbd, 0xbf, 0xcd, 0x4c, 0x44, 0x5f, 0xf0,
    0x38, 0x8e, 0xaa, 0xc3, 0x2a, 0x90, 0x88, 0x30, 0x84, 0x20, 0xda, 0xde,
    0x1c, 0x79, 0xbf, 0x63, 0xda, 0x5e, 0x8d, 0xbc, 0xae, 0x6b, 0xd7, 0xc7,
    0x7c, 0x3b, 0xe5, 0x27, 0xf5, 0xff, 0xdf, 0x57, 0xf7, 0xc8, 0x1f, 0x91,
    0xa7, 0xc0, 0x49, 0x0e, 0x5a, 0xc4, 0x98, 0x00, 0x00, 0x00, 0x00, 0x49,
    0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};


KoPalette::KoPalette(KoView * parent, const char * name)
    : QDockWindow(parent, name)
{

#if KDE_VERSION >= KDE_MAKE_VERSION(3,3,90)
    KAcceleratorManager::setNoAccel(this);
#endif    
    setCloseMode( QDockWindow::Never);
    setResizeEnabled(true);
    setOpaqueMoving(true);
    setFocusPolicy(QWidget::NoFocus);
      setVerticallyStretchable(false);
      setHorizontallyStretchable(false);
    
    setNewLine(true);
    layout() -> setSpacing(0);
    layout() -> setMargin(0);

    // Compute a font that's smaller that the general font
    m_font  = KGlobalSettings::generalFont();
    float ps = m_font.pointSize() * 0.7;
    m_font.setPointSize((int)ps);
    setFont(m_font);

#if 0 // Photoshop doesn't shade either, let's try for minimum size
    // XXX: Use a KDE icon for this?
    QImage img;
    img.loadFromData( img_shadebutton, sizeof( img_shadebutton), "PNG" );
    m_pixShadeButton = img;

    m_wdgDockerTabLayout = new QGridLayout( layout(), 1, 1, 0, "wdgDockerTabLayout");
     m_wdgDockerTabLayout->layout()-> setSpacing(0);
    
    m_lblCaption = new QLabel( this, "lblCaption" );
    m_lblCaption->setBackgroundMode( QLabel::PaletteMid );
    m_lblCaption->setFont(m_font);

    m_wdgDockerTabLayout->addWidget( m_lblCaption, 0, 0 );

    m_bnShade = new QToolButton( this, "bnShade" );
    m_bnShade->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, m_bnShade->sizePolicy().hasHeightForWidth() ) );
    m_bnShade->setMinimumSize( QSize( 18, 18 ) );
    m_bnShade->setMaximumSize( QSize( 18, 18 ) );
    m_bnShade->setBackgroundMode( QToolButton::PaletteMid );
    m_bnShade->setToggleButton( true );
    m_bnShade->setOn( false );
    m_bnShade->setIconSet( QIconSet( m_pixShadeButton ) );
    QToolTip::add( m_bnShade, i18n( "Shade or unshade the palette" ) );
    
    m_wdgDockerTabLayout->addWidget( m_bnShade, 0, 1 );

    QObject::connect(m_bnShade, SIGNAL(toggled(bool)), this, SLOT(slotShade(bool)));
    QObject::connect(this, SIGNAL(placeChanged(QDockWindow::Place)), this, SLOT(slotPlaceChanged(QDockWindow::Place)));
#endif
    
}

KoPalette::~KoPalette()
{
}

void KoPalette::setMainWidget(QWidget * widget)
{
#if 0
    m_wdgDockerTabLayout->addMultiCellWidget( widget, 1, 1, 0, 1 );
#endif
    setWidget(widget);
    resize( QSize(285, 233).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
    widget->setFont(m_font);
    m_page = widget;
}


void KoPalette::setCaption(const QString & /*caption*/)
{
#if 0
    QDockWindow::setCaption(caption);
     m_lblCaption -> setText(caption);
#endif
}

void KoPalette::slotShade(bool toggle)
{
    m_shaded = toggle;
    if (!toggle) {
         m_page -> show();
    }
    else {
         m_page -> hide();
        if (!m_docked) {
            resize(minimumSize());
        }

    }
}

void KoPalette::slotPlaceChanged(QDockWindow::Place p)
{

    if (p == QDockWindow::InDock) {
        m_docked = true;
#if 0
        m_lblCaption -> show();
        m_bnShade -> show();
        m_lblCaption -> setText(caption());
#endif
        resize(sizeHint());
    }
    else {
        if (m_docked) {
#if 0
            m_lblCaption -> hide();
            m_bnShade -> hide();
#endif 
            m_page-> show();
#if 0
            m_lblCaption -> setText("");
#endif
            resize(sizeHint());
        }
        m_docked = false;
    }
}

#include "kopalette.moc"
