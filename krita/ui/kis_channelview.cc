/*
 *  channelview.cc - part of Krayon
 *
 *  Copyright (c) 1999 Andrew Richards <A.Richards@phys.canterbury.ac.nz>
 *                1999 Michael Koch    <koch@kde.org>
 *                2001 John Califf     <jcaliff@compuzone.net>
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

#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qstring.h>
#include <qslider.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qstyle.h>

#include <kstandarddirs.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kdialog.h>

#include <koFrameButton.h>

#include "kis_doc.h"
#include "kis_view.h"
#include "kis_channelview.h"
#include "kis_factory.h"
#include "integerwidget.h"

KisChannelView::KisChannelView( KisDoc *_doc, QWidget *_parent, const char *_name )
	: QWidget( _parent, _name )
{
	buttons = new QHBox( this );
	buttons -> setMargin(4);
	buttons -> setSpacing(4);
	buttons -> setMaximumHeight(30);

	pbAddChannel = new QToolButton( buttons );
	pbAddChannel -> setFixedSize(24, 24);
	pbAddChannel->setPixmap( BarIcon( "newlayer" ) );

	pbRemoveChannel = new QToolButton( buttons );
	pbRemoveChannel -> setFixedSize(24, 24);
	pbRemoveChannel->setPixmap( BarIcon( "deletelayer" ) );

	pbUp = new QToolButton( buttons );
	pbUp -> setFixedSize(24, 24);
	pbUp->setPixmap( BarIcon( "raiselayer" ) );

	pbDown = new QToolButton( buttons );
	pbDown->setPixmap( BarIcon( "lowerlayer" ) );
	pbDown -> setFixedSize(24, 24);

	QWidget * spacer = new QWidget(buttons);
	buttons -> setStretchFactor(spacer, 10); 


	frame = new QHBox( this );
	frame->setFrameStyle( QFrame::Panel | QFrame::Sunken );

	channeltable = new ChannelTable( _doc, frame, this, "channellist" );

	connect( pbAddChannel, SIGNAL( clicked() ),
		 channeltable, SLOT( slotAddChannel() ) );
	connect( pbRemoveChannel, SIGNAL( clicked() ),
		 channeltable, SLOT( slotRemoveChannel() ) );
	//connect( pbUp, SIGNAL( clicked() ),
	//    channeltable, SLOT( slotRaiseChannel() ) );
	//connect( pbDown, SIGNAL( clicked() ),
	//    channeltable, SLOT( slotLowerChannel() ) );

	initGUI();
}


void KisChannelView::initGUI()
{
	QVBoxLayout *mainLayout = new QVBoxLayout( this, 2);
	QHBoxLayout *buttonsLayout = new QHBoxLayout( buttons, 4 );

	buttonsLayout->addWidget(pbAddChannel);
	buttonsLayout->addWidget(pbRemoveChannel);
	buttonsLayout->addWidget(pbUp);
	buttonsLayout->addWidget(pbDown);

	mainLayout->addWidget( frame);
	mainLayout->addWidget( buttons);
}

KisChannelView::~KisChannelView()
{
	delete pbAddChannel;
	delete pbRemoveChannel;
	delete pbUp;
	delete pbDown;
	delete buttons;
	delete channeltable;
	delete frame;
}

void KisChannelView::showScrollBars( )
{
	resizeEvent(0L);
}


ChannelTable::ChannelTable( QWidget* _parent, const char* _name )
	: super( _parent, _name )
{
	pChannelView = 0L;
	init( 0 );
}

ChannelTable::ChannelTable( KisDoc* doc, QWidget* _parent, const char* _name )
	: super( _parent, _name )
{
	pChannelView = 0L;
	init( doc );
}

ChannelTable::ChannelTable( KisDoc* doc, QWidget* _parent,
			    KisChannelView *_channelview, const char* _name )
	: super( _parent, _name )
{
	pChannelView = _channelview;
	init( doc );
}


void ChannelTable::init( KisDoc* doc )
{
	m_doc = doc;

	setBackgroundColor( white );

	// load icon pixmaps
	QString _icon = locate( "kis_pics", "visible.png", KisFactory::global() );
	mVisibleIcon = new QPixmap;
	if( !mVisibleIcon->load( _icon ) )
		KMessageBox::error( this, i18n("Can't find 'visible.png'."), i18n("Canvas") );
	mVisibleRect = QRect( QPoint( 2,( CELLHEIGHT - mVisibleIcon->height() ) / 2 ),
			      mVisibleIcon->size() );

	_icon = locate( "kis_pics", "novisible.png",
			KisFactory::global() );
	mNovisibleIcon = new QPixmap;
	if( !mNovisibleIcon->load( _icon ) )
		KMessageBox::error( this, i18n("Can't find 'novisible.png'."), i18n("Canvas") );

	_icon = locate( "kis_pics", "linked.png", KisFactory::global() );
	mLinkedIcon = new QPixmap;
	if( !mLinkedIcon->load( _icon ) )
		KMessageBox::error( this, i18n("Can't find 'linked.png'."), i18n("Canvas") );
	mLinkedRect = QRect( QPoint( 25,( CELLHEIGHT - mLinkedIcon->height() ) / 2 ),
			     mLinkedIcon->size() );

	_icon = locate( "kis_pics", "unlinked.png", KisFactory::global() );
	mUnlinkedIcon = new QPixmap;
	if( !mUnlinkedIcon->load( _icon ) )
		KMessageBox::error( this, i18n("Can't find 'unlinked.png'."), i18n("Canvas") );

	mPreviewRect
		= QRect( QPoint( 50, (CELLHEIGHT - mLinkedIcon->height() ) /2 ),
			 mLinkedIcon->size() );

	updateTable();

	setCellWidth( CELLWIDTH );
	setCellHeight( CELLHEIGHT );

	m_selected = 0; // XXX (m_doc->currentImg() ? m_doc->currentImg()->getCurrentLayer()->numChannels() : 0) - 1;

	updateAllCells();

	QPopupMenu *submenu = new QPopupMenu();
	m_contextmenu = new QPopupMenu();
	m_contextmenu->setCheckable(TRUE);

	m_contextmenu->insertItem( i18n( "Visible" ), VISIBLE );
	m_contextmenu->insertItem( i18n( "Level" ), submenu );

	m_contextmenu->insertSeparator();

	m_contextmenu->insertItem( i18n( "Add Channel" ), ADDCHANNEL );
	m_contextmenu->insertItem( i18n( "Remove Channel"), REMOVECHANNEL );

	connect( m_contextmenu, SIGNAL( activated( int ) ),
		 SLOT( slotMenuAction( int ) ) );
	connect( submenu, SIGNAL( activated( int ) ),
		 SLOT( slotMenuAction( int ) ) );
}


void ChannelTable::paintCell( QPainter* _painter, int _row, int /* _col */)
{
	QString tmp;
	QColor color;
	//cId channels[MAX_CHANNELS];

	switch( _row )
	{
	case 0 :
		tmp = i18n( "Red" );
		color = red;
		break;
	case 1 :
		tmp = i18n( "Green" );
		color = green;
		break;
	case 2 :
		tmp = i18n( "Blue" );
		color = blue;
		break;
	default :
		tmp = i18n( "Alpha" );
		color = gray;
		break;
	}

	if( _row == m_selected )
		_painter->fillRect( 0, 0, cellWidth() - 1, cellHeight() - 1, color);

	style().drawPrimitive( QStyle::PE_Panel, _painter,
			       QRect( mVisibleRect.x(), mVisibleRect.y(),
				      mVisibleRect.width(), mVisibleRect.height() ),
			       colorGroup() ); // , true );

	if( /* m_doc->currentImg().channels[_row ]->isVisible()*/ true )
	{
		_painter->drawPixmap( mVisibleRect.topLeft(), *mVisibleIcon );
	}
	else
	{
		_painter->drawPixmap( mVisibleRect.topLeft(), *mNovisibleIcon );
	}

	style().drawPrimitive( QStyle::PE_Panel, _painter,
			       QRect( mPreviewRect.x(), mPreviewRect.y(),
				      mPreviewRect.width(), mPreviewRect.height() ),
			       colorGroup() ); //, true );

	_painter->drawRect( 0, 0, cellWidth() - 1, cellHeight() - 1);
	_painter->drawText( 80, 20, tmp );
}


void ChannelTable::updateTable()
{
	m_items = 4;
	setNumRows( 4 );
	setNumCols( 1 );
	resize( sizeHint() );
}

void ChannelTable::update_contextmenu( int  )
{
	m_contextmenu->setItemChecked( VISIBLE,  true
				       /* m_doc->currentImg()->layerList().at( _index )->visible()*/ );
}

void ChannelTable::selectChannel( int _index )
{
	int currentSel = m_selected;
	m_selected = -1;

	updateCell( currentSel, 0 );
	m_selected = _index;
	//m_doc->currentImg()->setCurrentLayer( m_selected );
	updateCell( m_selected, 0 );
}

void ChannelTable::slotInverseVisibility( int _index )
{
	//m_doc->currentImg()->layerList().at( _index )->setVisible( !m_doc->currentImg()->layerList().at( _index )->visible() );
	updateCell( _index, 0 );

	//m_doc->currentImg()->markDirty( m_doc->currentImg()->layerList().at( _index )->imageExtents() );
}

void ChannelTable::slotMenuAction( int _id )
{
	switch( _id )
	{
	case VISIBLE:
		slotInverseVisibility( m_selected );
		break;

	case ADDCHANNEL:
		slotAddChannel();
		break;

	case REMOVECHANNEL:
		slotRemoveChannel();
		break;

	default:
		break;
	}
}


QSize ChannelTable::sizeHint() const
{
	if(pChannelView)
		return QSize( CELLWIDTH, pChannelView->getFrame()->height());
	else
		return QSize( CELLWIDTH, CELLHEIGHT * 5 );

}

void ChannelTable::mousePressEvent( QMouseEvent *_event )
{
	int row = rowAt(_event -> pos().y());
	if (row < 0) return;

	QPoint localPoint( _event->pos().x() % cellWidth(),
			   _event->pos().y() % cellHeight() );

	if( _event->button() & LeftButton )
	{
		if( mVisibleRect.contains( localPoint ) )
		{
			slotInverseVisibility( row );
		}
		else if( row != -1 )
		{
			selectChannel( row );
		}
	}
	else if( _event->button() & RightButton )
	{
		selectChannel( row );
		update_contextmenu( row );
		m_contextmenu->popup( mapToGlobal( _event->pos() ) );
	}
}

void ChannelTable::slotAddChannel()
{

}

void ChannelTable::slotRemoveChannel()
{

}

void ChannelTable::updateAllCells()
{
	for( unsigned int i = 0; i < /* m_doc->layerList().count()*/ 3;  i++ )
		updateCell( i, 0 );
}

void ChannelTable::slotProperties()
{
#if 0
	if( ChannelPropertyDialog::editProperties(
		    *( m_doc->currentImg()->getCurrentLayer()->firstChannel() ) ) )
	{
		//QRect updateRect = m_doc->currentImg()->layerList().at( m_selected )->imageExtents();
		updateCell( m_selected, 0 );
		//m_doc->currentImg()->markDirty( updateRect );
	}
#endif
}

ChannelPropertyDialog::ChannelPropertyDialog( QString _channelname,
					      uchar _opacity, QWidget *_parent, const char *_name )
	: KDialogBase( _parent, _name, true, "", Ok | Cancel )
{
	QGridLayout *layout = new QGridLayout( this, 4, 2, KDialog::marginHint(), KDialog::spacingHint() );

	m_name = new QLineEdit( _channelname, this );
	layout->addWidget( m_name, 0, 1 );

	QLabel *lblName = new QLabel( m_name, i18n( "Name:" ), this );
	layout->addWidget( lblName, 0, 0 );

	m_opacity = new IntegerWidget( 0, 255, this );
	m_opacity->setValue( _opacity );
	m_opacity->setTickmarks( QSlider::Below );
	m_opacity->setTickInterval( 32 );
	layout->addWidget( m_opacity, 1, 1 );

	QLabel *lblOpacity = new QLabel( m_opacity, i18n( "Opacity:" ), this );
	layout->addWidget( lblOpacity, 1, 0 );

	layout->setRowStretch( 2, 1 );
}


bool ChannelPropertyDialog::editProperties( KisChannel &/*_channel*/ )
{
	ChannelPropertyDialog *dialog;

	dialog = new ChannelPropertyDialog(/* _channel.name(), _channel.opacity(),*/
		"red", 255, NULL, "channel_property_dialog" );

	if( dialog->exec() == Accepted )
	{
		//_channel.setName( dialog->m_name->text() );
		//_channel.setOpacity( dialog->m_opacity->value() );

		return true;
	}
	return false;
}


#include "kis_channelview.moc"

