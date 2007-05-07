/* This file is part of the KDE project
   Copyright (C) 2002, Benoit Vautrin <benoit.vautrin@free.fr>

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

#include "KoContextHelp.h"

#include <QPainter>
#include <QRegion>
#include <QFont>
#include <QLabel>
#include <QLayout>
#include <q3simplerichtext.h>
#include <QPixmap>
#include <QPaintEvent>
#include <QPolygon>
#include <QEvent>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QTimerEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <kicon.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kcursor.h>
#include <QString>
#include <ktoolinvocation.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <QAbstractEventDispatcher>

KoVerticalLabel::KoVerticalLabel( QWidget* parent, const char* /*name*/ )
		: QWidget( parent, Qt::WNoAutoErase )
{
	QFont f( font() );
	f.setPointSize( f.pointSize() + 2 );
	f.setBold( true );
	setFont( f );
	setBackgroundRole( QPalette::Light );
} // KoVerticalLabel::KoVerticalLabel

KoVerticalLabel::~KoVerticalLabel()
{
} // KoVerticalLabel::~KoVerticalLabel

void KoVerticalLabel::setText( const QString& text )
{
	m_text = text;
	QFontMetrics fm( font() );
	setMinimumSize( fm.height() + 2, fm.width( m_text ) + 4 );
	update();
} // KoVerticalLabel::setText

void KoVerticalLabel::paintEvent( QPaintEvent* )
{
	QPixmap pm( height(), width() );
	QPainter p( &pm );
	p.fillRect( 0, 0, height(), width(), palette().window() );
	p.setFont( font() );
	p.drawText( 0, 0, height(), width(), Qt::AlignCenter, m_text );
	p.end();
	QPainter ap( this );
	ap.rotate( 270. );
	ap.translate( -height(), 0 );
	ap.drawPixmap( 0, 0, pm );
} // KoVerticalLabel::paintEvent

static unsigned char upbits[]   = { 0xc, 0x1e, 0x3f, 0x3f };
static unsigned char downbits[] = { 0x3f, 0x3f, 0x1e, 0xc };

KoHelpNavButton::KoHelpNavButton( NavDirection d, QWidget* parent )
		: QWidget( parent )
{
	m_pressed = false;
	m_bitmap = QBitmap::fromData( QSize(8, 4), ( d == Up ? upbits : downbits ) );
	m_bitmap.setMask( m_bitmap );
	setFixedSize( 8, 6 );
	setBackgroundRole( QPalette::Light );
} // KoHelpNavButton::KoHelpNavButton

void KoHelpNavButton::paintEvent( QPaintEvent* )
{
	QPainter p( this );
	if ( isEnabled() )
	{
		if ( m_pressed )
			p.setPen( palette().color( QPalette::Highlight ) );
		else
			p.setPen( palette().color( QPalette::Text ));
		p.drawPixmap( 1, 1, m_bitmap );
	}
} // KoHelpNavButton::paintEvent

void KoHelpNavButton::enterEvent( QEvent* )
{
	if ( isEnabled() )
		emit pressed();
	m_pressed = true;
	update();
} // KoHelpNavButton::enterEvent

void KoHelpNavButton::leaveEvent( QEvent* )
{
	if ( isEnabled() )
		emit released();
	m_pressed = false;
	update();
} // KoHelpNavButton::leaveEvent

static unsigned char notstickybits[] = { 0x8, 0x1e, 0xc, 0xa, 0x1 };
static unsigned char stickybits[]    = { 0xe, 0x11, 0x15, 0x11, 0xe };
static unsigned char closebits[]     = { 0x11, 0xa, 0x4, 0xa, 0x11 };

KoTinyButton::KoTinyButton( Action a, QWidget* parent )
		: QWidget( parent ), m_action( a )
{
	m_pressed = false;
	m_toggled = false;
	switch ( a )
	{
		case Sticky:
			m_bitmap = QBitmap::fromData( QSize( 5, 5 ), notstickybits );
			break;

		default:
			m_bitmap = QBitmap::fromData( QSize(5, 5), closebits );
	}
	m_bitmap.setMask( m_bitmap );
	setMinimumSize( 7, 7 );
	setBackgroundRole( QPalette::Window );
} // KoTinyButton::KoTinyButton

void KoTinyButton::paintEvent( QPaintEvent* )
{
	QPainter p( this );
	if ( isEnabled() )
	{
		if ( m_pressed )
			p.setPen( palette().color( QPalette::Highlight ) );
		else
			p.setPen( palette().color( QPalette::Text ) );
		p.drawPixmap( width() / 2 - 2, 1, m_bitmap );
	}
} // KoTinyButton::paintEvent

void KoTinyButton::mousePressEvent( QMouseEvent* )
{
	if ( isEnabled() )
	{
		m_pressed = true;
		update();
	}
} // KoTinyButton::mousePressEvent

void KoTinyButton::mouseReleaseEvent( QMouseEvent* )
{
	if ( isEnabled() && m_pressed )
	{
		m_pressed = false;
		emit( clicked() );
		if ( ( m_action == Sticky ) )
		{
			m_toggled = !m_toggled;
			emit( toggled( m_toggled ) );
			//switch ( m_action )
			//{
			//	case Sticky:
				m_bitmap = QBitmap::fromData( QSize( 5, 5), ( m_toggled ? stickybits : notstickybits ) );
			//}
			m_bitmap.setMask( m_bitmap );
		}
		update();
	}
} // KoTinyButton::mouseReleaseEvent

KoHelpView::KoHelpView( QWidget* parent )
		: QWidget( parent )
{
	currentText = 0L;
	setBackgroundRole( QPalette::Light );
	parent->installEventFilter( this );
	setMouseTracking( true );
} // KoHelpView::KoHelpView

KoHelpView::~KoHelpView()
{
	if ( currentText )
		delete currentText;
} // KoHelpView::~KoHelpView

void KoHelpView::setText( const QString& text )
{
	if ( currentText )
		delete currentText;
	currentText = new Q3SimpleRichText( text, font() );
	currentText->setWidth( width() );
	setFixedHeight( currentText->height() );
} // KoHelpView::setText

void KoHelpView::mousePressEvent( QMouseEvent* e )
{
	currentAnchor = currentText->anchorAt( e->pos() );
	if ( !currentAnchor.isEmpty() )
		e->accept();
	else
		e->ignore();
} // KoHelpView::mousePressEvent

void KoHelpView::mouseReleaseEvent( QMouseEvent* e )
{
	if ( ( !currentAnchor.isEmpty() ) && ( currentAnchor == currentText->anchorAt( e->pos() ) ) )
	{
		e->accept();
		if (currentAnchor.startsWith("help://#")) {
			//that's not really useful, since koffice documents can be embedded
			KToolInvocation::invokeHelp(currentAnchor.right(currentAnchor.length()-8));
		}
		else
		if (currentAnchor.startsWith("help://")) {
			// that's the useful version of a help link
			QString helpapp=currentAnchor.right(currentAnchor.length()-7);
			QString helpanchor;
			int pos;
			if ((pos=helpapp.indexOf("#"))!=-1) {
				helpanchor=helpapp.right(helpapp.length()-pos-1);
				helpapp=helpapp.left(pos);
			}
			KToolInvocation::invokeHelp(helpanchor,helpapp);
		}
		else
		emit linkClicked( currentAnchor );
		currentAnchor = "";
	}
	else
		e->ignore();
} // KoHelpView::mouseReleaseEvent

void KoHelpView::mouseMoveEvent( QMouseEvent* e )
{
	if ( !currentText->anchorAt( e->pos() ).isEmpty() )
		setCursor( Qt::PointingHandCursor );
	else
		setCursor( Qt::ArrowCursor );
} // KoHelpView::mouseMove

bool KoHelpView::eventFilter( QObject*, QEvent* e )
{
	if ( ( currentText ) && ( e->type() == QEvent::Resize ) )
	{
		setFixedWidth( ( (QResizeEvent*)e )->size().width() );
		currentText->setWidth( width() );
		setFixedHeight( currentText->height() );

		return true;
	}
	return false;
} // KoHelpView::resizeEvent

void KoHelpView::paintEvent( QPaintEvent* )
{
	QPainter p( this );
	currentText->draw( &p, 0, 0, QRect(), palette() );
} // KoHelpView::paintEvent

KoHelpWidget::KoHelpWidget( const QString & help, QWidget* parent )
		: QWidget( parent )
{
	QGridLayout* layout = new QGridLayout( this );
	layout->setMargin( 2 );
	layout->addWidget( m_upButton = new KoHelpNavButton( KoHelpNavButton::Up, this ), 0, 1, Qt::AlignHCenter );
	layout->addWidget( m_helpViewport = new QWidget( this ), 1, 1 );
	layout->addWidget( m_downButton = new KoHelpNavButton( KoHelpNavButton::Down, this ), 2, 1, Qt::AlignHCenter );
	layout->setSpacing( 5 );
	layout->setColumnStretch( 1, 1 );

	m_helpView = new KoHelpView( m_helpViewport );
	m_helpViewport->setBackgroundRole( QPalette::Light );
	setText( help );

	setBackgroundRole( QPalette::Light );

	connect( m_upButton, SIGNAL( pressed() ), this, SLOT( startScrollingUp() ) );
	connect( m_downButton, SIGNAL( pressed() ), this, SLOT( startScrollingDown() ) );
	connect( m_upButton, SIGNAL( released() ), this, SLOT( stopScrolling() ) );
	connect( m_downButton, SIGNAL( released() ), this, SLOT( stopScrolling() ) );
	connect( m_helpView, SIGNAL( linkClicked( const QString& ) ), this, SIGNAL( linkClicked( const QString& ) ) );
} // KoHelpWidget::KoHelpWidget

void KoHelpWidget::updateButtons()
{
	m_upButton->setEnabled( m_ypos < 0 );
	m_downButton->setEnabled( m_helpViewport->height() - m_ypos < m_helpView->height() );
} // KoHelpWidget::updateButtons

void KoHelpWidget::setText( const QString & text )
{
	m_helpView->setText( text );
	m_helpView->move( 0, 0 );
	m_ypos = 0;
	updateButtons();
} // KoHelpWidget::setText

void KoHelpWidget::resizeEvent( QResizeEvent* )
{
	updateButtons();
} // KoHelpWidget::resizeEvent

void KoHelpWidget::startScrollingUp()
{
	if ( !m_upButton->isEnabled() )
		return;
	m_scrollDown = false;
	startTimer( 80 );
} // KoHelpWidget::startScrollingUp

void KoHelpWidget::startScrollingDown()
{
	if ( !m_downButton->isEnabled() )
		return;
	m_scrollDown = true;
	startTimer( 80 );
} // KoHelpWidget::startScrollingDown

void KoHelpWidget::scrollUp()
{
	if ( m_ypos > 0 )
		stopScrolling();
	else
	{
		m_ypos += 2;
		m_helpViewport->scroll( 0, 2 );
		m_helpViewport->update();
		updateButtons();
	}
} // KoHelpWidget::scrollUp()

void KoHelpWidget::scrollDown()
{
	if ( m_helpViewport->height() - m_helpView->height() - m_ypos > 0 )
		stopScrolling();
	else
	{
		m_ypos -= 2;
		m_helpViewport->scroll( 0, -2 );
		m_helpViewport->update();
		updateButtons();
	}
} // KoHelpWidget::scrollUp()

void KoHelpWidget::timerEvent( QTimerEvent* )
{
	if ( m_scrollDown )
		scrollDown();
	else
		scrollUp();
} // KoHelpWidget::timerEvent

void KoHelpWidget::stopScrolling()
{
	QAbstractEventDispatcher::instance()->unregisterTimers(this);
} // KoHelpWidget::stopScrolling

KoContextHelpPopup::KoContextHelpPopup( QWidget* parent )
		: QWidget( parent, Qt::WType_Dialog | Qt::WStyle_Customize | Qt::WStyle_NoBorder )
{
	QGridLayout* layout = new QGridLayout( this );
	QHBoxLayout* buttonLayout;
	layout->addWidget( m_helpIcon = new QLabel( this ), 0, 0 );
	layout->addWidget( m_helpTitle = new KoVerticalLabel( this ), 1, 0 );
	buttonLayout = new QHBoxLayout();
	layout->addLayout( buttonLayout, 2, 0 );
	layout->addWidget( m_helpViewer = new KoHelpWidget( "", this ), 0, 1, 3, 1 );
	buttonLayout->addWidget( m_close = new KoTinyButton( KoTinyButton::Close, this ) );
	buttonLayout->addWidget( m_sticky = new KoTinyButton( KoTinyButton::Sticky, this ) );
	layout->setMargin( 3 );
	layout->setSpacing( 1 );
	layout->setRowStretch( 1, 1 );
	buttonLayout->setSpacing( 1 );
	setMinimumSize( 180, 180 );

	m_isSticky = false;
	setFocusPolicy( Qt::StrongFocus );

	connect( m_close, SIGNAL( clicked() ), this, SIGNAL( wantsToBeClosed() ) );
	connect( m_sticky, SIGNAL( toggled( bool ) ), this, SLOT( setSticky( bool ) ) );
	connect( m_helpViewer, SIGNAL( linkClicked( const QString& ) ), this, SIGNAL( linkClicked( const QString& ) ) );
} // KoContextHelpPopup::KoContextHelpPopup

KoContextHelpPopup::~KoContextHelpPopup()
{
} // KoContextHelpPopup::~KoContextHelpPopup

void KoContextHelpPopup::setContextHelp( const QString& title, const QString& text, const QPixmap* icon )
{
	m_helpIcon->setPixmap( icon ? *icon : BarIcon( "help-contents" ) );
	m_helpTitle->setText( title );
	m_helpViewer->setText( text );
} // KoContextHelpPopup::updateHelp

void KoContextHelpPopup::mousePressEvent( QMouseEvent* e )
{
	m_mousePos = e->globalPos() - pos();
} // KoContextHelpPopup::mousePressEvent

void KoContextHelpPopup::mouseMoveEvent( QMouseEvent* e )
{
	move( e->globalPos() - m_mousePos );
} // KoContextHelpPopup::mouseMoveEvent

void KoContextHelpPopup::resizeEvent( QResizeEvent* )
{
	QBitmap mask( width(), height() );
	QPolygon a;
	QPainter p( &mask );
	p.fillRect( 0, 0, width(), height(), Qt::color1 );
	p.setPen( Qt::color0 );
	p.setBrush( Qt::color0 );
	p.drawLine( 0, 0, 0, 3 );
	p.drawLine( 0, 0, 3, 0 );
	p.drawPoint( 1, 1 );
	a.setPoints( 3, 0, height() - 5, 4, height() - 1, 0, height() - 1 );
	p.drawPolygon( a );
	a.setPoints( 3, width() - 5, 0, width() - 1, 4, width() - 1, 0 );
	p.drawPolygon( a );
	p.drawLine( width() - 1, height() - 1, width() - 4, height() - 1 );
	p.drawLine( width() - 1, height() - 1, width() - 1, height() - 4 );
	p.drawPoint( width() - 2, height() - 2 );
	p.drawPoint( 0, height() - 6 );
	p.drawPoint( width() - 6, 0 );
	p.drawPoint( width() - 5, height() - 3 );
	p.drawPoint( width() - 3, height() - 5 );
	p.setPen( Qt::NoPen );
	p.setBrush( QBrush( Qt::color0, Qt::Dense4Pattern ) );
	p.drawRect( 0, height() - 2, width() - 1, height() - 1 );
	p.drawRect( width() - 2, 0, width() - 1, height() - 1 );
	p.drawRect( width() - 4, height() - 4, width() - 2, height() - 2 );
	p.end();
	setMask( QRegion( mask ) );
} // KoContextHelpPopup::resizeEvent

void KoContextHelpPopup::paintEvent( QPaintEvent* )
{
	QPainter p( this );
	p.fillRect( 0, 0, width(), height(), palette().light() );
	p.setPen( Qt::black );
	p.drawRect( 0, 0, width(), height() );
	p.fillRect( width() - 3, 0, width() - 1, height() - 1, Qt::black );
	p.fillRect( 0, height() - 3, width() - 1, height() - 1, Qt::black );
	p.drawLine( 1, 2, 1, 3 );
	p.drawLine( 2, 1, 3, 1 );
	p.drawLine( width() - 4, 2, width() - 4, 3 );
	p.drawLine( width() - 5, 1, width() - 6, 1 );
	p.drawLine( 1, height() - 5, 1, height() - 6 );
	p.drawLine( 2, height() - 4, 3, height() - 4 );
	p.drawLine( width() - 4, height() - 5, width() - 4, height() - 6 );
	p.drawLine( width() - 4, height() - 4, width() - 6, height() - 4 );
} // KoContextHelpPopup::paintEvent

void KoContextHelpPopup::windowActivationChange( bool )
{
	if ( !isActiveWindow() && !m_isSticky )
		emit wantsToBeClosed();
} // KoContestHelpPopup::windowActivationChange

void KoContextHelpPopup::keyPressEvent( QKeyEvent* e )
{
	switch ( e->key() )
	{
/*		case Qt::Key_Up:
				m_helpViewer->startScrollingUp();
			break;

		case Qt::Key_Down:
				m_helpViewer->startScrollingDown();
			break;*/
		case Qt::Key_Up:
				m_helpViewer->scrollUp();
			break;

		case Qt::Key_Down:
				m_helpViewer->scrollDown();
			break;
	}
} // KoContextHelpPopup::keyPressEvent

void KoContextHelpPopup::keyReleaseEvent( QKeyEvent* e )
{
	switch ( e->key() )
	{
		/*case Qt::Key_Up:
		case Qt::Key_Down:
				m_helpViewer->stopScrolling();
			break;*/

		case Qt::Key_Escape:
				emit wantsToBeClosed();
			break;
	}
} // KoContextHelpPopup::keyPressEvent

KoContextHelpAction::KoContextHelpAction( KActionCollection* parent, QWidget* /*popupParent*/ )
    : KToggleAction( KIcon(BarIcon("help-contents")), i18n("Context Help"), parent)
{
    Q_ASSERT(parent);
    setShortcut(KShortcut("CTRL+Qt::SHIFT+F1"));

	m_popup = new KoContextHelpPopup( 0L );
	connect( m_popup, SIGNAL( wantsToBeClosed() ), this, SLOT( closePopup() ) );
	connect( this, SIGNAL( toggled( bool ) ), m_popup, SLOT( setShown( bool ) ) );
	connect( m_popup, SIGNAL( linkClicked( const QString& ) ), this, SIGNAL( linkClicked( const QString& ) ) );
    parent->addAction("help_context", this );
}

KoContextHelpAction::~KoContextHelpAction()
{
	delete m_popup;
} // KoContextHelpAction::~KoContextHelpAction

void KoContextHelpAction::updateHelp( const QString& title, const QString& text, const QPixmap* icon )
{
	m_popup->setContextHelp( title, text, icon );
} // KoContextHelpAction::updateHelp

void KoContextHelpAction::closePopup()
{
	activate(Trigger);
	setChecked( false ); // For a unknown reason, this is needed...
} // KoContextHelpAction::closePopup


KoContextHelpWidget::KoContextHelpWidget( QWidget* parent, const char* /*name*/ )
		: QWidget( parent )
{
	setWindowTitle( i18n( "Context Help" ) );
	QGridLayout* layout = new QGridLayout( this );
	layout->addWidget( m_helpIcon = new QLabel( this ), 0, 0 );
	layout->addWidget( m_helpTitle = new KoVerticalLabel( this ), 1, 0 );
	layout->addWidget( m_helpViewer = new KoHelpWidget( "", this ), 0, 1, 2, 1 );
	layout->setMargin( 2 );
	layout->setSpacing( 1 );
	layout->setRowStretch( 1, 1 );
	this->setMinimumSize( 180, 120 );
	this->show();
	setContextHelp( i18n( "Context Help" ), i18n( "Here will be shown help according to your actions" ), 0 );
	connect( m_helpViewer, SIGNAL( linkClicked( const QString& ) ), this, SIGNAL( linkClicked( const QString& ) ) );
} // KoContextHelpWidget::KoContextHelpWidget

KoContextHelpWidget::~KoContextHelpWidget()
{
} // KoContextHelpWidget::~KoContextHelpWidget

void KoContextHelpWidget::setContextHelp( const QString& title, const QString& text, const QPixmap* icon )
{
	m_helpIcon->setPixmap( icon ? *icon : BarIcon( "help-contents" ) );
	m_helpTitle->setText( title );
	m_helpViewer->setText( text );
} // KoContextHelpWidget::updateHelp


KoContextHelpDocker::KoContextHelpDocker( QWidget* parent, const char* name )
		: Q3DockWindow( parent, name )
{
	setWindowTitle( i18n( "Context Help" ) );
	QWidget* mainWidget = new QWidget( this );
	QGridLayout* layout = new QGridLayout( mainWidget );
	layout->addWidget( m_helpIcon = new QLabel( mainWidget ), 0, 0 );
	layout->addWidget( m_helpTitle = new KoVerticalLabel( mainWidget ), 1, 0 );
	layout->addWidget( m_helpViewer = new KoHelpWidget( "", mainWidget ), 0, 1, 2, 1 );
	layout->setMargin( 2 );
	layout->setSpacing( 1 );
	layout->setRowStretch( 1, 1 );
	mainWidget->setMinimumSize( 180, 120 );
	mainWidget->show();
	setWidget( mainWidget );
	setContextHelp( i18n( "Context Help" ), i18n( "Here will be shown help according to your actions" ), 0 );
	connect( m_helpViewer, SIGNAL( linkClicked( const QString& ) ), this, SIGNAL( linkClicked( const QString& ) ) );
} // KoContextHelpDocker::KoContextHelpDocker

KoContextHelpDocker::~KoContextHelpDocker()
{
} // KoContextHelpDocker::~KoContextHelpDocker

void KoContextHelpDocker::setContextHelp( const QString& title, const QString& text, const QPixmap* icon )
{
	m_helpIcon->setPixmap( icon ? *icon : BarIcon( "help-contents" ) );
	m_helpTitle->setText( title );
	m_helpViewer->setText( text );
} // KoContextHelpDocker::updateHelp

#include "KoContextHelp.moc"
