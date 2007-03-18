/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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

#include <QRect>
#include <QComboBox>
#include <QPainter>
#include <QColor>

#include <kdebug.h>

#include <kis_brush.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_layer.h>
#include <kis_painter.h>
#include <kis_types.h>
#include <kis_paintop.h>
#include <kis_iterator.h>
#include <kis_iterators_pixel.h>
#include <kis_selection.h>

#include <kis_int_spinbox.h>

#include "kis_cpaintop.h"
#include "ui_wdgcpaintoptions.h"
#include "brush.h"
#include "qtoolbutton.h"
#include "stroke.h"
#include "sample.h"
#include "bristle.h"


namespace {
// XXX: Remove this with Qt 4.3
    static QRect toAlignedRect(QRectF rc)
    {
        int xmin = int(floor(rc.x()));
        int xmax = int(ceil(rc.x() + rc.width()));
        int ymin = int(floor(rc.y()));
        int ymax = int(ceil(rc.y() + rc.height()));
        return QRect(xmin, ymin, xmax - xmin, ymax - ymin);
    }
}

KisCPaintOpFactory::KisCPaintOpFactory()
{
    m_brushes.resize( 6 );
    for ( int i=0; i < 6; i++ )
        m_brushes[i] = new Brush ( i+1 );
}

KisCPaintOpFactory::~KisCPaintOpFactory()
{
    for (uint i = 0; i < m_brushes.count(); i++) {
        delete m_brushes[i];
        m_brushes[i] = 0;
    }

}

KisPaintOp * KisCPaintOpFactory::createOp(const KisPaintOpSettings *settings,
                                          KisPainter * painter)
{
    const KisCPaintOpSettings * cpaintOpSettings =
        dynamic_cast<const KisCPaintOpSettings*>( settings );

    Q_ASSERT( settings == 0 || cpaintOpSettings != 0 );
    int curBrush = cpaintOpSettings->brush();
    if ( curBrush > 5 ) {
        curBrush = 5;
    }
    else if ( curBrush < 0 ) {
        curBrush = 0;
    }

    KisPaintOp * op = new KisCPaintOp( m_brushes[curBrush], cpaintOpSettings, painter );

    Q_CHECK_PTR( op );
    return op;
}

KisPaintOpSettings *KisCPaintOpFactory::settings(QWidget * parent, const KoInputDevice& inputDevice)
{
    Q_UNUSED( inputDevice );
    return new KisCPaintOpSettings( parent,  m_brushes);
}

//=================

KisCPaintOpSettings::KisCPaintOpSettings( QWidget * parent,  Q3ValueVector<Brush*> brushes)
    : KisPaintOpSettings( parent )
{
    m_brushes = brushes;
    m_optionsWidget = new QWidget( parent );
    m_options = new Ui::WdgCPaintOptions( );
    m_options->setupUi( m_optionsWidget );
    m_options->intInk->setRange( 0, 255 );
    m_options->intWater->setRange( 0, 255 );


    connect( m_options->bnInk, SIGNAL( clicked() ), this, SLOT( resetCurrentBrush() ) );
}


int KisCPaintOpSettings::brush() const
{
    return m_options->cmbBrush->currentItem();
}

int KisCPaintOpSettings::ink() const
{
    return m_options->intInk->value();
}

int KisCPaintOpSettings::water() const
{
    return m_options->intWater->value();
}


void KisCPaintOpSettings::resetCurrentBrush()
{
    Brush * b = m_brushes[m_options->cmbBrush->currentItem()];
    b->addInk();
}


//=================

KisCPaintOp::KisCPaintOp(Brush * brush, const KisCPaintOpSettings * settings, KisPainter * painter)
    : super(painter)
{
    m_currentBrush = brush;
    m_ink = settings->ink();
    m_water = settings->water();
    newStrokeFlag = true;
    m_color = m_painter->paintColor();
    m_stroke = 0;
    KisPaintDeviceSP dev = painter->device();

    m_tempImage = QImage( dev->width(), dev->height(), QImage::Format_ARGB32 );

}

KisCPaintOp::~KisCPaintOp()
{
    delete m_stroke;
}



void KisCPaintOp::paintAt(const QPointF &pos, const KisPaintInformation& info)
{
    if (!m_painter->device()) return;

    KisPaintDeviceSP device = m_painter->device();

    sampleCount++;
    Sample * newSample = new Sample;
    newSample->setPressure ( info.pressure * 500 );
    newSample->setX ( pos.x() );
    newSample->setY ( pos.y() );
    newSample->setTiltX ( info.xTilt );
    newSample->setTiltY ( info.yTilt );


    if ( newStrokeFlag ) {
        m_lastPoint = pos;
        m_stroke = new Stroke( m_currentBrush);
        m_stroke->setColor( m_color );
        m_stroke->sampleV.push_back( newSample );
        m_stroke->storeOldPath( pos.x(), pos.y() );
        newStrokeFlag = false;
    }
    else {
        if ( m_stroke )
            m_stroke->sampleV.push_back( newSample );
    }

    if ( m_stroke ) {
        int brushSize = m_currentBrush->size();
        QPainter gc( device.data() );
        gc.setRenderHint(QPainter::Antialiasing);
        m_stroke->draw( gc );
        gc.end();

//        KisPaintDeviceSP dab = new KisPaintDevice(device->colorSpace());
//        dab->convertFromQImage( m_tempImage, "" ); // Use monitor profile?

//        m_painter->bitBlt( QPoint( 0, 0 ), dab, m_tempImage.rect() );

    }
    m_lastPoint = pos;
}

#include "kis_cpaintop.moc"
