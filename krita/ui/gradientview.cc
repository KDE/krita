#include <kdebug.h>
#include <qpainter.h>

#include <kpixmap.h>
#include <kpixmapeffect.h>

#include "gradientview.h"

GradientView::GradientView( QWidget *_parent, const char *_name )
  : QWidget( _parent, _name )
{
    addItem( red   , green , 0.10, 0.40 );
    addItem( green , blue  , 0.45, 0.50 );
    addItem( blue  , yellow, 0.52, 0.55 );
    addItem( yellow, red   , 0.68, 0.70 );
    addItem( red   , green , 0.90, 1.00 );

    //m_pixmap.resize( size () );
    m_pixmap.resize( 300, 50 );
}

GradientView::~GradientView()
{
}

void GradientView::addItem( QColor _leftColor, QColor _rightColor, float _middle, float _right )
{
    GradientItem *item = new GradientItem;

    item->leftColor = _leftColor;
    item->rightColor = _rightColor;
    item->middle = _middle;
    item->right = _right;

    m_lstGradientItems.append( item );
}


QSize GradientView::sizeHint() const
{
    return QSize( 100, 50 );
}


void GradientView::paintEvent( QPaintEvent */*_event*/ )
{
    kdDebug() << "GradientView::paintEvent\n";

    //int w;
    int pos = 0;
    //float index = 0.0;

    QPainter p;
    p.begin( this );

    updatePixmap();
    p.drawPixmap( pos, 0, m_pixmap );

/*
    GradientItem* item = m_lstGradientItems.first();
    for( int i = 0; i < m_lstGradientItems.count(); i++ )
    {
        w = (int) (( item->right - index ) * (float) width());

//  kdDebug() << "Michael : processing gradient item, width : " << w << endl;

        p.fillRect( pos, 0, w, height(), item->leftColor );
        pos += w;
        index = item->right;

        item = m_lstGradientItems.next();
  }
*/
    p.end();
}


void GradientView::updatePixmap()
{
    kdDebug() << "GradientView::updatePixmap()\n";

    int w, pos = 0;
    float index = 0.0;
    KPixmap grad;
    GradientItem* item;

    m_pixmap.resize( size() );

    item = m_lstGradientItems.first();
    for( unsigned int i = 0; i < m_lstGradientItems.count(); i++ )
    {
        // FIXME: Round up, if needed.
        if( i != ( m_lstGradientItems.count() -1 ) )
            w = (int) (( item->right - index ) * (float) width());
        else
            w = size().width() - pos;

        grad.resize( w, height() );

        KPixmapEffect::gradient( grad, item->leftColor, item->rightColor,
            KPixmapEffect::HorizontalGradient );
        bitBlt( &m_pixmap, pos, 0, &grad );

        pos += w;
        index = item->right;
        item = m_lstGradientItems.next();
    }
}

void GradientView::readGIMPGradientFile( const QString& /*_file*/ )
{
/*
Golden gradient

GIMP Gradient
14
0.000000 0.080316 0.163606 0.137255 0.156863 0.011760 1.000000 0.533330 0.415600 0.086270 1.000000 0 0
0.163606 0.193879 0.224151 0.533330 0.415600 0.086270 1.000000 0.650000 0.550000 0.161000 1.000000 0 0
0.224151 0.254424 0.284697 0.650000 0.550000 0.161000 1.000000 0.800000 0.710000 0.290000 1.000000 0 0
0.284697 0.314969 0.345242 0.800000 0.710000 0.290000 1.000000 0.920000 0.859000 0.400000 1.000000 0 0
0.345242 0.382304 0.414023 0.920000 0.859000 0.400000 1.000000 0.960000 0.925000 0.440000 1.000000 0 0
0.414023 0.467446 0.516416 0.960000 0.925000 0.440000 1.000000 0.820000 0.745000 0.298000 1.000000 0 0
0.516416 0.541681 0.571953 0.820000 0.745000 0.298000 1.000000 0.733300 0.612000 0.200000 1.000000 0 0
0.571953 0.602226 0.632499 0.733300 0.612000 0.200000 1.000000 0.658800 0.556900 0.165000 1.000000 0 0
0.632499 0.662771 0.698052 0.658800 0.556900 0.165000 1.000000 0.792160 0.682300 0.266667 1.000000 0 0
0.698052 0.728325 0.757930 0.792160 0.682300 0.266667 1.000000 0.855000 0.792000 0.337000 1.000000 0 0
0.757930 0.787201 0.817474 0.855000 0.792000 0.337000 1.000000 0.816000 0.733300 0.286300 1.000000 0 0
0.817474 0.847746 0.878019 0.816000 0.733300 0.286300 1.000000 0.733300 0.612000 0.200000 1.000000 0 0
0.878019 0.906511 0.934891 0.733300 0.612000 0.200000 1.000000 0.537000 0.423500 0.101000 1.000000 0 0
0.934891 0.973289 1.000000 0.537000 0.423500 0.101000 1.000000 0.137255 0.156863 0.011760 1.000000 0 0
*/
}

#include "gradientview.moc"












