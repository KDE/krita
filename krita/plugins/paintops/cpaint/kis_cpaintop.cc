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

#include <qrect.h>
#include <qcombobox.h>

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
#include <kis_point.h>
#include <kis_int_spinbox.h>

#include "kis_cpaintop.h"
#include "wdgcpaintoptions.h"
#include "brush.h"
#include "qtoolbutton.h"
#include "stroke.h"
#include "sample.h"
#include "bristle.h"

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

KisPaintOpSettings *KisCPaintOpFactory::settings(QWidget * parent, const KisInputDevice& inputDevice)
{
    return new KisCPaintOpSettings( parent,  m_brushes);
}

//=================

KisCPaintOpSettings::KisCPaintOpSettings( QWidget * parent,  QValueVector<Brush*> brushes)
    : KisPaintOpSettings( parent )
{
    m_brushes = brushes;
    m_options = new WdgCPaintOptions( parent );
    m_options->intInk->setRange( 0, 255 );
    m_options->intWater->setRange( 0, 255 );
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
    b->AddInk();
}


//=================

KisCPaintOp::KisCPaintOp(Brush * brush, const KisCPaintOpSettings * settings, KisPainter * painter)
    : super(painter)
{
    kdDebug() << "Create a new cpaintop\n";
    m_currentBrush = brush;
    m_ink = settings->ink();
    m_water = settings->water();
    newStrokeFlag = true;
    m_color = m_painter->paintColor();
    curStroke = 0;
}

KisCPaintOp::~KisCPaintOp()
{

}



void KisCPaintOp::paintAt(const KisPoint &pos, const KisPaintInformation& info)
{
    if (!m_painter->device()) return;

    KisPaintDeviceSP device = m_painter->device();
    KisColorSpace * colorSpace = device->colorSpace();


    Sample *newSam;
    Stroke *tmp;

    sampleCount++;
    newSam = new Sample;
    newSam->SetPressure ( info.pressure * 500 );
    newSam->SetX ( pos.x() );
    newSam->SetY ( pos.y() );
    newSam->SetTX ( info.xTilt );
    newSam->SetTY ( info.yTilt );


    kdDebug() << "New stroke flag: " << newStrokeFlag << ", stroke: " << curStroke << endl;
    if ( newStrokeFlag ) {
        kdDebug() << "new stroke\n";
        curStroke = new Stroke( m_currentBrush);
        curStroke->StoreColor( m_color );
        curStroke->sampleV.push_back( newSam );
        curStroke->StoreOldPath( pos.x(), pos.y() );
        newStrokeFlag = false;
    }
    else {
        if ( curStroke )
            curStroke->sampleV.push_back( newSam );
    }
    if ( curStroke )
        curStroke->Draw( device );
}

