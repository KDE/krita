/*
 *  kis_dlg_toolopts.cc - part of Krayon
 *
 *  Copyright (c) 2001 John Califf <jcaliff@compuzone.net>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qvbuttongroup.h>
#include <qvgroupbox.h> 

#include <klocale.h>
#include <kdebug.h>

#include "kis_dlg_toolopts.h"
#include "kis_dlg_toolopts_polygon_preview.h"

/*
    ToolOptionsDialog constructor.  This widget shows options for only
    one tool at a time - the current tool in use.  There will be another
    KDialog with an iconview selector for all tools, accessed from the
    setting menu for setting all tool opts int one place. 
*/
ToolOptionsDialog::ToolOptionsDialog( tooltype tt, ToolOptsStruct ts,
    QWidget *parent, const char *name )
    : KDialog( parent, name, true )
{
    setCaption( i18n("Current Tool Options") );
    QVBoxLayout* layout = new QVBoxLayout( this, 4 );
    
    // use tooltype enumerator - 
    switch(tt)
    {
        case tt_linetool:    
        case tt_polylinetool:
        case tt_ellipsetool:    
        case tt_rectangletool:
        
            pLineToolTab = new LineToolTab(ts, 
                static_cast<QWidget *>(this));
            layout->addWidget(pLineToolTab);    
            break;
            
        case tt_brushtool:

            pBrushToolTab = new BrushToolTab(ts, 
                static_cast<QWidget *>(this));
            layout->addWidget(pBrushToolTab);    
            break;

        case tt_airbrushtool:

            pAirBrushToolTab = new AirBrushToolTab(ts, 
                static_cast<QWidget *>(this));
            layout->addWidget(pAirBrushToolTab);    
            break;

        case tt_pentool:

            pPenToolTab = new PenToolTab(ts, 
                static_cast<QWidget *>(this));
            layout->addWidget(pPenToolTab);    
            break;

        case tt_erasertool:

            pEraserToolTab = new EraserToolTab(ts, 
                static_cast<QWidget *>(this));
            layout->addWidget(pEraserToolTab);    
            break;

        case tt_filltool:
        case tt_colorchangertool:

            pFillToolTab = new FillToolTab(ts, 
                static_cast<QWidget *>(this));
            layout->addWidget(pFillToolTab);    
            break;

        case tt_stamptool:
        case tt_pastetool:
        
            pStampToolTab = new StampToolTab(ts, 
                static_cast<QWidget *>(this));
            layout->addWidget(pStampToolTab);    
            break;

        case tt_polygontool:

            pPolygonToolTab = new PolygonToolTab(ts, 
                static_cast<QWidget *>(this));
            layout->addWidget(pPolygonToolTab);    
            break;

        default:
            // we really should show global option settings here
            pNullToolTab = new NullToolTab(ts, 
                static_cast<QWidget *>(this));
            layout->addWidget(pNullToolTab);                    
            break;
    }
    
    QHBoxLayout* buttons = new QHBoxLayout( layout, 3 );
    buttons->addStretch( 3 );

    QPushButton *ok, *cancel, *save;
    ok = new QPushButton( i18n("&OK"), this );
    ok->setDefault( true );
    ok->setMinimumSize( ok->sizeHint() );
    connect( ok, SIGNAL(clicked()), SLOT(accept()) );
    buttons->addWidget( ok );

    cancel = new QPushButton( i18n("&Cancel"), this );
    cancel->setMinimumSize( cancel->sizeHint() );
    connect( cancel, SIGNAL(clicked()), SLOT(reject()) );
    buttons->addWidget( cancel );

    save = new QPushButton( i18n("&Save"), this );
    save->setMinimumSize( save->sizeHint() );
    connect( save, SIGNAL(clicked()), SLOT(reject()) );
    buttons->addWidget( save );
 
    resize( 1, 1 );
}


/*
    KisToolTab - base class for all tool tab widgets
*/
KisToolTab::KisToolTab(ToolOptsStruct /*ts*/, QWidget *_parent, const char *_name)
    : QWidget( _parent, _name )
{

}


/*
    NullToolTab - General Settings
*/
NullToolTab::NullToolTab( ToolOptsStruct ts,
    QWidget *_parent, const char *_name  )
    : KisToolTab(ts,  _parent, _name )
{
    QVBoxLayout* lout = new QVBoxLayout( this, 4 );    
    QGridLayout* grid = new QGridLayout(lout, 2, 4);

    mpThickness = new KIntNumInput( ts.lineThickness, this );
    mpThickness->setRange( 1, 16, 1 );
    mpThickness->setLabel( i18n( "Line thickness:" ) );

    grid->addWidget( mpThickness, 0, 0 );

    mpOpacity = new KIntNumInput( ts.lineOpacity, this );
    mpOpacity->setRange( 0, 255, 32 );
    mpOpacity->setLabel( i18n( "Opacity:" ) );

    grid->addWidget( mpOpacity, 1, 0 );

    mpSolid = new QCheckBox( i18n("Fill interior regions"), this );
    mpSolid->setChecked( ts.fillShapes );
    grid->addWidget( mpSolid, 2, 0 );

    mpUsePattern = new QCheckBox( i18n("Use current pattern"), this );
    mpUsePattern->setChecked( ts.usePattern );
    grid->addWidget( mpUsePattern, 3, 0 );
}


/*
    LineToolTab - for lines, circle, ellipses, polylines,
    rectangles, pologons.  All Qt drawing primitives except
    text, which needs its own dialog for font selection 
*/
LineToolTab::LineToolTab( ToolOptsStruct ts,
    QWidget *_parent, const char *_name  )
    : KisToolTab(ts,  _parent, _name )
{
    QVBoxLayout* lout = new QVBoxLayout( this, 4 );    
    QGridLayout* grid = new QGridLayout(lout, 2, 4);

    mpThickness = new KIntNumInput( ts.lineThickness, this );
    mpThickness->setRange( 1, 16, 1 );
    mpThickness->setLabel( i18n( "Thickness:" ) );

    grid->addWidget( mpThickness, 0, 0 );

    mpOpacity = new KIntNumInput( ts.lineOpacity, this );
    mpOpacity->setRange( 0, 255, 32 );
    mpOpacity->setLabel( i18n( "Opacity:" ) );

    grid->addWidget( mpOpacity, 1, 0 );

    mpSolid = new QCheckBox( i18n("Fill interior regions"), this );
    mpSolid->setChecked( ts.fillShapes );
    grid->addWidget( mpSolid, 2, 0 );

    mpUsePattern = new QCheckBox( i18n("Use current pattern"), this );
    mpUsePattern->setChecked( ts.usePattern );
    grid->addWidget( mpUsePattern, 3, 0 );

    mpUseGradient = new QCheckBox( i18n("Fill with gradient"), this );
    mpUseGradient->setChecked( ts.useGradient );
    grid->addWidget( mpUseGradient, 4, 0 );
    
}


/*
    FillToolTab - for lines, circle, ellipses, polylines,
    rectangles, pologons.  All Qt drawing primitives except
    text, which needs its own dialog for font selection 
*/
FillToolTab::FillToolTab( ToolOptsStruct ts,
    QWidget *_parent, const char *_name  )
    : KisToolTab(ts, _parent, _name )
{
    QVBoxLayout* lout = new QVBoxLayout( this, 4 );    
    QGridLayout* grid = new QGridLayout(lout, 2, 3);

    mpOpacity = new KIntNumInput( ts.opacity, this );
    mpOpacity->setRange( 0, 255, 32 );
    mpOpacity->setLabel( i18n( "Opacity:" ) );

    grid->addWidget( mpOpacity, 0, 0 );

    mpUsePattern = new QCheckBox( i18n("Fill with pattern"), this );
    mpUsePattern->setChecked( ts.usePattern );
    grid->addWidget( mpUsePattern, 1, 0 );

    mpUseGradient = new QCheckBox( i18n("Fill with gradient"), this );
    mpUseGradient->setChecked( ts.useGradient );
    grid->addWidget( mpUseGradient, 2, 0 );

}


/*
    PenToolTab - for lines, circle, ellipses, polylines,
    rectangles, pologons.  All Qt drawing primitives except
    text, which needs its own dialog for font selection 
*/
PenToolTab::PenToolTab( ToolOptsStruct ts,
    QWidget *_parent, const char *_name  )
    : KisToolTab(ts, _parent, _name )
{
    QVBoxLayout* lout = new QVBoxLayout( this, 4 );    
    QGridLayout* grid = new QGridLayout(lout, 2, 4);

    mpOpacity = new KIntNumInput( ts.opacity, this );
    mpOpacity->setRange( 0, 255, 32 );
    mpOpacity->setLabel( i18n( "Opacity:" ) );

    grid->addWidget( mpOpacity, 0, 0 );

    mpPenThreshold = new KIntNumInput( ts.penThreshold, this );
    mpPenThreshold->setRange( 0, 255, 32 );
    mpPenThreshold->setLabel( i18n( "Paint threshold:" ) );

    grid->addWidget( mpPenThreshold, 1, 0 );

    mpUsePattern = new QCheckBox( i18n("Paint with pattern"), this );
    mpUsePattern->setChecked( ts.usePattern );
    grid->addWidget( mpUsePattern, 2, 0 );

    mpUseGradient = new QCheckBox( i18n("Paint with gradient"), this );
    mpUseGradient->setChecked( ts.useGradient );
    grid->addWidget( mpUseGradient, 3, 0 );

}

/*
    BrushToolTab - for lines, circle, ellipses, polylines,
    rectangles, pologons.  All Qt drawing primitives except
    text, which needs its own dialog for font selection 
*/
BrushToolTab::BrushToolTab( ToolOptsStruct ts,
    QWidget *_parent, const char *_name  )
    : KisToolTab(ts, _parent, _name )
{
    QVBoxLayout* lout = new QVBoxLayout( this, 4 );    
    QGridLayout* grid = new QGridLayout(lout, 2, 3);

    mpOpacity = new KIntNumInput( ts.opacity, this );
    mpOpacity->setRange( 0, 255, 32 );
    mpOpacity->setLabel( i18n( "Opacity:" ) );

    grid->addWidget( mpOpacity, 0, 0 );

    mpUseGradient = new QCheckBox( i18n("Blend with current gradient"), this );
    mpUseGradient->setChecked( ts.useGradient );
    grid->addWidget( mpUseGradient, 1, 0 );

    mpUsePattern = new QCheckBox( i18n("Blend with current pattern"), this );
    mpUsePattern->setChecked( ts.usePattern );
    grid->addWidget( mpUsePattern, 2, 0 );
}

/*
    EraserToolTab - for lines, circle, ellipses, polylines,
    rectangles, pologons.  All Qt drawing primitives except
    text, which needs its own dialog for font selection 
*/
EraserToolTab::EraserToolTab( ToolOptsStruct ts,
    QWidget *_parent, const char *_name  )
    : KisToolTab(ts, _parent, _name )
{
    QVBoxLayout* lout = new QVBoxLayout( this, 4 );    
    QGridLayout* grid = new QGridLayout(lout, 2, 3);

    mpOpacity = new KIntNumInput( ts.opacity, this );
    mpOpacity->setRange( 0, 255, 32 );
    mpOpacity->setLabel( i18n( "Opacity:" ) );

    grid->addWidget( mpOpacity, 0, 0 );

    mpUseGradient = new QCheckBox( i18n("Blend with current gradient"), this );
    mpUseGradient->setChecked( ts.fillShapes );
    grid->addWidget( mpUseGradient, 1, 0 );

    mpUsePattern = new QCheckBox( i18n("Blend with current pattern"), this );
    mpUsePattern->setChecked( ts.usePattern );
    grid->addWidget( mpUsePattern, 2, 0 );
}

/*
    AirBrushToolTab -
*/
AirBrushToolTab::AirBrushToolTab( ToolOptsStruct ts,
    QWidget *_parent, const char *_name  )
    : KisToolTab(ts, _parent, _name )
{
    QVBoxLayout* lout = new QVBoxLayout( this, 4 );
    QGridLayout* grid = new QGridLayout(lout, 2, 3);

    mpOpacity = new KIntNumInput( ts.opacity, this );
    mpOpacity->setRange( 0, 255, 32 );
    mpOpacity->setLabel( i18n( "Opacity:" ) );

    grid->addWidget( mpOpacity, 0, 0 );

    mpUseGradient = new QCheckBox( i18n("Use current gradient"), this );
    mpUseGradient->setChecked( ts.useGradient );
    grid->addWidget( mpUseGradient, 1, 0 );

    mpUsePattern = new QCheckBox( i18n("Use current pattern"), this );
    mpUsePattern->setChecked( ts.usePattern );
    grid->addWidget( mpUsePattern, 2, 0 );
}



/*
    StampBrushToolTab -
*/
StampToolTab::StampToolTab( ToolOptsStruct ts,
    QWidget *_parent, const char *_name  )
    : KisToolTab(ts, _parent, _name )
{
    QVBoxLayout* lout = new QVBoxLayout( this, 4 );
    QGridLayout* grid = new QGridLayout(lout, 2, 2);

    mpOpacity = new KIntNumInput( ts.opacity, this );
    mpOpacity->setRange( 0, 255, 32 );
    mpOpacity->setLabel( i18n( "Opacity:" ) );

    grid->addWidget( mpOpacity, 0, 0 );

    mpUseGradient = new QCheckBox( i18n("Blend with current gradient"), this );
    mpUseGradient->setChecked( ts.useGradient );
    grid->addWidget( mpUseGradient, 1, 0 );
}

/*
    PolygonToolTab
*/
PolygonToolTab::PolygonToolTab( ToolOptsStruct ts,
    QWidget *_parent, const char *_name  )
    : KisToolTab(ts, _parent, _name )
{
    QVBoxLayout *lout = new QVBoxLayout( this, 8 );
    QGridLayout *grid = new QGridLayout( lout, 10, 30 );

    QButtonGroup *group = new QVButtonGroup( this );

    mpConvexPolygon = new QRadioButton( i18n( "Polygon" ), group );
    mpConvexPolygon->setChecked( ts.convexPolygon );
    connect( mpConvexPolygon, SIGNAL( clicked() ), this, SLOT( slotConvexPolygon() ) );

    mpConcavePolygon = new QRadioButton( i18n( "Concave polygon" ), group );
    mpConcavePolygon->setChecked( ts.concavePolygon );
    connect( mpConcavePolygon, SIGNAL( clicked() ), this, SLOT( slotConcavePolygon() ) );

    grid->addWidget( group, 0, 0 );

    mpCorners = new KIntNumInput( ts.polygonCorners, this );
    mpCorners->setRange( 3, 100, 1 );
    mpCorners->setLabel( i18n( "Corners:" ) );
    grid->addWidget( mpCorners, 1, 0 );

    mpSharpness = new KIntNumInput( ts.polygonSharpness, this );
    mpSharpness->setRange( 0, 100, 1 );
    mpSharpness->setLabel( i18n( "Sharpness:" ) );
    grid->addWidget( mpSharpness, 2, 0 );

    if ( !ts.concavePolygon )
        mpSharpness->setEnabled( false );

    mpThickness = new KIntNumInput( ts.lineThickness, this );
    mpThickness->setRange( 1, 16, 1 );
    mpThickness->setLabel( i18n( "Thickness:" ) );
    grid->addWidget( mpThickness, 3, 0 );

    mpOpacity = new KIntNumInput( ts.lineOpacity, this );
    mpOpacity->setRange( 0, 255, 32 );
    mpOpacity->setLabel( i18n( "Opacity:" ) );
    grid->addWidget( mpOpacity, 4, 0 );


    mpSolid = new QCheckBox( i18n("Fill interior regions"), this );
    mpSolid->setChecked( ts.fillShapes );
    grid->addWidget( mpSolid, 5, 0 );

    mpUsePattern = new QCheckBox( i18n("Use current pattern"), this );
    mpUsePattern->setChecked( ts.usePattern );
    grid->addWidget( mpUsePattern, 6, 0 );

    mpUseGradient = new QCheckBox( i18n("Fill with gradient"), this );
    mpUseGradient->setChecked( ts.useGradient );
    grid->addWidget( mpUseGradient, 7, 0 );


    preview = new PolygonPreview( this, 0L, ts.polygonCorners, ts.polygonSharpness, ts.concavePolygon, ts.lineThickness );
    grid->addMultiCellWidget( preview, 0, 7, 2, 30 );
    grid->setColStretch( 30, 1 );
    grid->setRowStretch( 7, 1 );
    connect ( mpConvexPolygon, SIGNAL( clicked() ), preview,
              SLOT( slotConvexPolygon() ) );
    connect ( mpConcavePolygon, SIGNAL( clicked() ), preview,
              SLOT( slotConcavePolygon() ) );
    connect( mpCorners, SIGNAL( valueChanged( int ) ), preview,
             SLOT( slotConersValue( int ) ) );
    connect( mpSharpness, SIGNAL( valueChanged( int ) ), preview,
             SLOT( slotSharpnessValue( int ) ) );
    connect( mpThickness, SIGNAL( valueChanged( int ) ), preview,
             SLOT( slotThicknessValue( int ) ) );
}

void PolygonToolTab::slotConvexPolygon()
{
    mpSharpness->setEnabled( false );
}

void PolygonToolTab::slotConcavePolygon()
{
    mpSharpness->setEnabled( true );
}

#include "kis_dlg_toolopts.moc"
