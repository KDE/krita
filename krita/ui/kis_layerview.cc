/*
 *  kis_layerview.cc - part of Krayon
 *
 *  Copyright (c) 1999 Andrew Richards <A.Richards@phys.canterbury.ac.nz>
 *                1999 Michael Koch    <koch@kde.org>
 *                2000 Matthias Elter  <elter@kde.org>
 *                2001 John Califf
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
#include <qpixmap.h>
#include <qpainter.h>
#include <qlineedit.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qstyle.h>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include <koFrameButton.h>

#include "kis_doc.h"
#include "kis_view.h"
#include "kis_util.h"
#include "kis_layerview.h"
#include "kis_factory.h"
#include "integerwidget.h"

//#define KISBarIcon( x ) BarIcon( x, KisFactory::global() )

const int iheight = 32;

KisLayerView::KisLayerView( KisDoc *doc, QWidget *parent, const char *name )
	: QWidget( parent, name )
{
	buttons = new QHBox( this );
	buttons->setMaximumHeight(12);

	pbAddLayer = new KoFrameButton( buttons );
	pbAddLayer->setPixmap( BarIcon( "newlayer" ) );

	pbRemoveLayer = new KoFrameButton( buttons );
	pbRemoveLayer->setPixmap( BarIcon( "deletelayer" ) );

	pbUp = new KoFrameButton( buttons );
	pbUp->setPixmap( BarIcon( "raiselayer" ) );

	pbDown = new KoFrameButton( buttons );
	pbDown->setPixmap( BarIcon( "lowerlayer" ) );

	// only serves as beautifier for the widget
	frame = new QHBox( this );
	frame->setFrameStyle( QFrame::Panel | QFrame::Sunken );

	layertable = new LayerTable( doc, frame, this, "layerlist" );

	connect( pbAddLayer, SIGNAL( clicked() ),
		 layertable, SLOT( slotAddLayer() ) );
	connect( pbRemoveLayer, SIGNAL( clicked() ),
		 layertable, SLOT( slotRemoveLayer() ) );
	connect( pbUp, SIGNAL( clicked() ),
		 layertable, SLOT( slotRaiseLayer() ) );
	connect( pbDown, SIGNAL( clicked() ),
		 layertable, SLOT( slotLowerLayer() ) );

	QToolTip::add( pbAddLayer, i18n( "Create new layer" ) );
	QToolTip::add( pbRemoveLayer, i18n( "Remove current layer" ) );
	QToolTip::add( pbUp, i18n( "Raise current layer" ) );
	QToolTip::add( pbDown, i18n( "Lower current layer" ) );

	initGUI();
}

void KisLayerView::initGUI()
{
	QVBoxLayout *mainLayout = new QVBoxLayout( this, 2);
	QHBoxLayout *buttonsLayout = new QHBoxLayout( buttons, 4 );

	buttonsLayout->addWidget(pbAddLayer);
	buttonsLayout->addWidget(pbRemoveLayer);
	buttonsLayout->addWidget(pbUp);
	buttonsLayout->addWidget(pbDown);

	mainLayout->addWidget( frame);
	mainLayout->addWidget( buttons);
}

KisLayerView::~KisLayerView()
{
	delete pbAddLayer;
	delete pbRemoveLayer;
	delete pbUp;
	delete pbDown;
	delete buttons;
	delete layertable;
	delete frame;
}

void KisLayerView::showScrollBars( )
{
	resizeEvent(0L);
}

LayerTable::LayerTable( QWidget* parent, const char* name )
	: super( parent, name )
{
	pLayerView = 0L;
	init( 0 );
}


LayerTable::LayerTable( KisDoc* doc, QWidget* parent, const char* name )
	: super( parent, name )
{
	pLayerView = 0L;
	init( doc );
}

LayerTable::LayerTable( KisDoc* doc, QWidget* parent,
			KisLayerView *layerview, const char* name )
	: super(parent, name )
{
	pLayerView = layerview;
	init( doc );
}

void LayerTable::init( KisDoc* doc)
{
	m_doc = doc;
	setBackgroundColor( white );

	// load icon pixmaps
	QString icon = locate( "kis_pics", "visible.png", KisFactory::global());
	mVisibleIcon = new QPixmap;
	if( !mVisibleIcon->load( icon ) )
		KMessageBox::error( this, i18n("Can't find 'visible.png'."), i18n("Canvas") );
	mVisibleRect = QRect( QPoint( 3, (iheight - 24)/2), QSize(24,24));

	icon = locate( "kis_pics", "novisible.png",
		       KisFactory::global() );
	mNovisibleIcon = new QPixmap;
	if( !mNovisibleIcon->load( icon ) )
		KMessageBox::error( this, i18n("Can't find 'novisible.png'."), i18n("Canvas") );

	icon = locate( "kis_pics", "linked.png", KisFactory::global() );
	mLinkedIcon = new QPixmap;
	if( !mLinkedIcon->load( icon ) )
		KMessageBox::error( this, i18n("Can't find 'linked.png'."), i18n("Canvas") );
	mLinkedRect = QRect(QPoint(30, (iheight - 24)/2), QSize(24,24));

	icon = locate( "kis_pics", "unlinked.png", KisFactory::global() );
	mUnlinkedIcon = new QPixmap;
	if( !mUnlinkedIcon->load( icon ) )
		KMessageBox::error( this, i18n("Can't find 'unlinked.png'."), i18n("Canvas") );
	mPreviewRect = QRect(QPoint(57, (iheight - 24)/2), QSize(24,24));

	updateTable();

	setCellWidth( CELLWIDTH );
	setCellHeight( iheight );
	m_selected = (m_doc -> currentImg() ? m_doc -> currentImg()->layerList().size() : 0) - 1;

	QPopupMenu *submenu = new QPopupMenu();

	submenu->insertItem( i18n( "Upper" ), UPPERLAYER );
	submenu->insertItem( i18n( "Lower" ), LOWERLAYER );
	submenu->insertItem( i18n( "Foremost" ), FRONTLAYER );
	submenu->insertItem( i18n( "Hindmost" ), BACKLAYER );

	m_contextmenu = new QPopupMenu();

	m_contextmenu->setCheckable(TRUE);

	m_contextmenu->insertItem( i18n( "Visible" ), VISIBLE );
	m_contextmenu->insertItem( i18n( "Selection"), SELECTION );
	m_contextmenu -> insertItem(i18n("Level"), submenu, LEVEL);
	m_contextmenu->insertItem( i18n( "Linked"), LINKING );
	m_contextmenu->insertItem( i18n( "Properties..."), PROPERTIES );

	m_contextmenu->insertSeparator();

	m_contextmenu->insertItem( i18n( "Add Layer..." ), ADDLAYER );
	m_contextmenu->insertItem( i18n( "Remove Layer"), REMOVELAYER );
	m_contextmenu->insertItem( i18n( "Add Mask" ), ADDMASK );
	m_contextmenu->insertItem( i18n( "Remove Mask"), REMOVEMASK );

	connect(m_contextmenu, SIGNAL(activated(int)), SLOT(slotMenuAction(int)));
	connect(m_contextmenu, SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));
	connect(submenu, SIGNAL(activated(int)), SLOT(slotMenuAction(int)));
	connect(doc, SIGNAL(layersUpdated()), this, SLOT(slotDocUpdated()));
}

void LayerTable::slotDocUpdated()
{
	updateTable();
	updateAllCells();

	if(pLayerView)
		pLayerView -> showScrollBars();
}

void LayerTable::paintCell(QPainter *painter, int row, int /*col*/)
{
	KisImage *img = m_doc -> currentImg();
	KisLayer *lay = img -> layerList().at(row);

	if (!lay)
		return;

	if (row == m_selected)
		painter -> fillRect(0, 0, cellWidth() - 1, cellHeight() - 1, gray);
	else
		painter -> fillRect(0, 0, cellWidth() - 1, cellHeight() - 1, lightGray);

	style().drawPrimitive(QStyle::PE_Panel, painter, QRect(mVisibleRect.x(), mVisibleRect.y(), mVisibleRect.width(), mVisibleRect.height()), colorGroup());
	QPoint pt(mVisibleRect.left() + 2, mVisibleRect.top() + 2);

	if (lay -> visible())
		painter -> drawPixmap(pt, *mVisibleIcon);
	else
		painter -> drawPixmap(pt, *mNovisibleIcon);

	style().drawPrimitive(QStyle::PE_Panel, painter, QRect(mLinkedRect.x(), mLinkedRect.y(), mLinkedRect.width(), mLinkedRect.height()), colorGroup());
	pt = QPoint(mLinkedRect.left() + 2, mLinkedRect.top() + 2);

	if (lay -> linked())
		painter -> drawPixmap(pt, *mLinkedIcon);
	else
		painter -> drawPixmap(pt, *mUnlinkedIcon);

	style().drawPrimitive(QStyle::PE_Panel, painter, QRect(mPreviewRect.x(), mPreviewRect.y(), mPreviewRect.width(), mPreviewRect.height()), colorGroup());
	painter -> drawRect(0, 0, cellWidth() - 1, cellHeight() - 1);
	painter -> drawText(iheight * 3 + 3*3, 20, lay -> name());
}

void LayerTable::updateTable()
{
	KisImage *img = m_doc -> currentImg();

	if (img) {
		m_items = img -> layerList().size();
		setNumRows(m_items);
		setNumCols(1);
	}
	else {
		m_items = 0;
		setNumRows(0);
		setNumCols(0);
	}

	resize(sizeHint());

	if (pLayerView)
		pLayerView -> showScrollBars();

	repaint();
}

void LayerTable::update_contextmenu(int indx)
{
	KisImage *img = m_doc -> currentImg();

	if (static_cast<unsigned int>(indx) < img -> layerList().size()) {
		KisLayer *lay = img -> layerList().at(indx);

		m_contextmenu -> setItemChecked(VISIBLE, lay -> visible());
		m_contextmenu -> setItemChecked(LINKING, lay -> linked());
	}
}

void LayerTable::selectLayer(int indx)
{
	KisImage *img = m_doc -> currentImg();
	int currentSel = m_selected;

	m_selected = -1;
	updateCell(currentSel, 0);
	m_selected = indx;
	img -> setCurrentLayer(m_selected);
	updateCell(m_selected, 0);
}

void LayerTable::slotInverseVisibility(int indx)
{
	KisImage *img = m_doc -> currentImg();
	KisLayer *lay = img -> layerList().at(indx);

	lay -> setVisible(!img -> layerList().at(indx) -> visible());
	updateCell(indx, 0);
	img -> markDirty(lay -> imageExtents());
	m_doc -> setModified(true);
}

void LayerTable::slotInverseLinking(int indx)
{
	KisImage *img = m_doc -> currentImg();
	KisLayer *lay = img -> layerList().at(indx);

	lay -> setLinked(!lay -> linked());
	updateCell(indx, 0);
	m_doc -> setModified(true);
}

void LayerTable::slotMenuAction(int id)
{
	switch(id) {
	case VISIBLE:
		slotInverseVisibility(m_selected);
		break;
	case LINKING:
		slotInverseLinking(m_selected);
		break;
	case PROPERTIES:
		slotProperties();
		break;
	case ADDLAYER:
		slotAddLayer();
		break;
	case REMOVELAYER:
		slotRemoveLayer();
		break;
	case UPPERLAYER:
		slotRaiseLayer();
		break;
	case LOWERLAYER:
		slotLowerLayer();
		break;
	case FRONTLAYER:
		slotFrontLayer();
		break;
	case BACKLAYER:
		slotBackgroundLayer();
		break;
	default:
		break;
	}
}

QSize LayerTable::sizeHint() const
{
	int h = pLayerView ? pLayerView -> getFrame() -> height() : iheight * 5;

	return QSize(CELLWIDTH, h);
}

void LayerTable::mousePressEvent( QMouseEvent *ev)
{
	int row = rowAt(ev -> pos().y());
	QPoint localPoint(ev -> pos().x() % cellWidth(), ev -> pos().y() % cellHeight());

	if (ev -> button() & LeftButton) {
		if (mVisibleRect.contains(localPoint)) {
		       	slotInverseVisibility(row);
		}
		else if (mLinkedRect.contains(localPoint)) {
			slotInverseLinking( row );
		}
		else if (row != -1) {
			selectLayer( row );
		}
	}
	else if (ev -> button() & RightButton) {
		if (row != -1) {
			selectLayer(row);
			update_contextmenu(row);
			m_contextmenu -> popup(mapToGlobal(ev -> pos()));
		}
	}
}

void LayerTable::mouseDoubleClickEvent( QMouseEvent *ev )
{
	if (ev -> button() & LeftButton)
		slotProperties();
}

void LayerTable::slotAddLayer()
{
	KisImage *img = m_doc->currentImg();

	QString name = i18n( "layer %1" ).arg( img->layerList().size() );

	img->addLayer( img->imageExtents(), KoColor::white(), true, name );

	QRect uR = img->layerList().at(img->layerList().size() - 1)->imageExtents();
	img->markDirty(uR);

	selectLayer( img->layerList().size() - 1 );

	updateTable();
	updateAllCells();

	m_doc->setModified( true );
}


void LayerTable::slotRemoveLayer()
{
	if( m_doc->currentImg()->layerList().size() != 0 )
	{
		QRect uR = m_doc->currentImg()->layerList().at(m_selected)->imageExtents();
		m_doc->currentImg()->removeLayer( m_selected );
		m_doc->currentImg()->markDirty(uR);

		if( m_selected == (int)m_doc->currentImg()->layerList().size() )
			m_selected--;

		updateTable();
		updateAllCells();

		m_doc->setModified( true );
	}
}

void LayerTable::swapLayers( int a, int b )
{
	if( ( m_doc->currentImg()->layerList().at( a )->visible() ) &&
	    ( m_doc->currentImg()->layerList().at( b )->visible() ) )
	{
		QRect l1 = m_doc->currentImg()->layerList().at( a )->imageExtents();
		QRect l2 = m_doc->currentImg()->layerList().at( b )->imageExtents();

		if( l1.intersects( l2 ) )
		{
			QRect rect = l1.intersect( l2 );
			m_doc->currentImg()->markDirty( rect );
		}
	}
}


void LayerTable::slotRaiseLayer()
{
	int newpos = m_selected > 0 ? m_selected - 1 : 0;

	if( m_selected != newpos )
	{
		m_doc->currentImg()->upperLayer( m_selected );
		repaint();
		swapLayers( m_selected, newpos );
		selectLayer( newpos );

		m_doc->setModified( true );
	}
}


void LayerTable::slotLowerLayer()
{
	int npos = (m_selected + 1) < (int)m_doc->currentImg()->layerList().size() ?
		m_selected + 1 : m_selected;

	if( m_selected != npos )
	{
		m_doc->currentImg()->lowerLayer( m_selected );
		repaint();
		swapLayers( m_selected, npos );
		selectLayer( npos );

		m_doc->setModified( true );
	}
}


void LayerTable::slotFrontLayer()
{
	if( m_selected != (int)(m_doc->currentImg()->layerList().size() - 1))
	{
		m_doc->currentImg()->setFrontLayer( m_selected );
		selectLayer( m_doc->currentImg()->layerList().size() - 1 );

		QRect uR = m_doc->currentImg()->layerList().at( m_selected )->imageExtents();
		m_doc->currentImg()->markDirty( uR );

		updateAllCells();

		m_doc->setModified( true );
	}
}


void LayerTable::slotBackgroundLayer()
{
	if( m_selected != 0 )
	{
		m_doc->currentImg()->setBackgroundLayer( m_selected );
		selectLayer( 0 );

		QRect uR = m_doc->currentImg()->layerList().at(m_selected)->imageExtents();
		m_doc->currentImg()->markDirty(uR);

		updateAllCells();

		m_doc->setModified( true );
	}
}


void LayerTable::updateAllCells()
{
	if(m_doc->currentImg())
		for( int i = 0; i < (int)m_doc->currentImg()->layerList().size(); i++ )
			updateCell( i, 0 );
}


void LayerTable::slotProperties()
{
	if (m_selected > m_doc->currentImg()->layerList().size())
		return;

	if( LayerPropertyDialog::editProperties(
		    *( m_doc->currentImg()->layerList().at(m_selected))))
	{
		QRect uR = m_doc->currentImg()->layerList().at( m_selected )->imageExtents();
		updateCell( m_selected, 0 );
		m_doc->currentImg()->markDirty( uR );

		m_doc->setModified( true );
	}
}

void LayerTable::slotAboutToShow()
{
	KisImage *img = m_doc -> currentImg();
	bool activate = static_cast<unsigned int>(m_selected) < img -> layerList().size();

	kdDebug() << "m_selected =  " << m_selected << endl;
	kdDebug() << "size = " << img -> layerList().size() << endl;

	m_contextmenu -> setItemEnabled(VISIBLE, activate);
	m_contextmenu -> setItemEnabled(LEVEL, activate);
	m_contextmenu -> setItemEnabled(LINKING, activate);
	m_contextmenu -> setItemEnabled(PROPERTIES, activate);
}

LayerPropertyDialog::LayerPropertyDialog( QString layername,
					  uchar opacity,  QWidget *parent, const char *name )
	: QDialog( parent, name, true )
{
	QGridLayout *layout = new QGridLayout( this, 4, 2, KDialog::marginHint(), KDialog::spacingHint() );

	m_name = new QLineEdit( layername, this );
	layout->addWidget( m_name, 0, 1 );

	QLabel *lblName = new QLabel( m_name, i18n( "Name" ), this );
	layout->addWidget( lblName, 0, 0 );

	m_opacity = new IntegerWidget( 0, 255, this );
	m_opacity->setValue( opacity );
	m_opacity->setTickmarks( QSlider::Below );
	m_opacity->setTickInterval( 32 );
	layout->addWidget( m_opacity, 1, 1 );

	QLabel *lblOpacity = new QLabel( m_opacity, i18n( "Opacity" ), this );
	layout->addWidget( lblOpacity, 1, 0 );

	layout->setRowStretch( 2, 1 );

	QHBox *buttons = new QHBox( this );
	layout->addMultiCellWidget( buttons, 3, 4, 0, 1 );

	(void) new QWidget( buttons );

	QPushButton *pbOk = new QPushButton( i18n("&OK"), buttons);
	pbOk->setDefault( true );
	QObject::connect( pbOk, SIGNAL(clicked()), this, SLOT(accept()));

	QPushButton *pbCancel = new QPushButton( i18n( "&Cancel" ), buttons);
	QObject::connect(pbCancel, SIGNAL(clicked()), this, SLOT(reject()));
}


bool LayerPropertyDialog::editProperties( KisLayer &layer )
{
	LayerPropertyDialog *dialog;

	dialog = new LayerPropertyDialog( layer.name(), layer.opacity(),
					  NULL, "Layer Properties" );

	if( dialog->exec() == Accepted )
	{
		layer.setName( dialog->m_name->text() );
		layer.setOpacity( dialog->m_opacity->value() );

		return true;
	}

	return false;
}

#include "kis_layerview.moc"
