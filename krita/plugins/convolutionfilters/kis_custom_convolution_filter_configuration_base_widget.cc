#include <kdialog.h>
#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file './kis_custom_convolution_filter_configuration_base_widget.ui'
**
** Created: lun sep 27 15:20:57 2004
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "kis_custom_convolution_filter_configuration_base_widget.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include "kis_matrix_widget.h"

/*
 *  Constructs a KisCustomConvolutionFilterConfigurationBaseWidget as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
KisCustomConvolutionFilterConfigurationBaseWidget::KisCustomConvolutionFilterConfigurationBaseWidget( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "KisCustomConvolutionFilterConfigurationBaseWidget" );
    KisCustomConvolutionFilterConfigurationBaseWidgetLayout = new QHBoxLayout( this, 11, 6, "KisCustomConvolutionFilterConfigurationBaseWidgetLayout"); 

    layout5 = new QVBoxLayout( 0, 0, 6, "layout5"); 

    matrixWidget = new KisMatrixWidget( this, "matrixWidget" );
    layout5->addWidget( matrixWidget );

    layout4 = new QHBoxLayout( 0, 0, 6, "layout4"); 

    textLabelFactor = new QLabel( this, "textLabelFactor" );
    layout4->addWidget( textLabelFactor );

    spinBoxFactor = new QSpinBox( this, "spinBoxFactor" );
    spinBoxFactor->setMinValue( 1 );
    spinBoxFactor->setValue( 1 );
    layout4->addWidget( spinBoxFactor );
    spacer1 = new QSpacerItem( 21, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout4->addItem( spacer1 );
    layout5->addLayout( layout4 );

    layout3 = new QHBoxLayout( 0, 0, 6, "layout3"); 

    textLabelOffset = new QLabel( this, "textLabelOffset" );
    layout3->addWidget( textLabelOffset );

    spinBoxOffset = new QSpinBox( this, "spinBoxOffset" );
    spinBoxOffset->setMaxValue( 255 );
    spinBoxOffset->setMinValue( -255 );
    layout3->addWidget( spinBoxOffset );
    spacer2 = new QSpacerItem( 24, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout3->addItem( spacer2 );
    layout5->addLayout( layout3 );
    spacer24 = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );
    layout5->addItem( spacer24 );
    KisCustomConvolutionFilterConfigurationBaseWidgetLayout->addLayout( layout5 );
    spacer5 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    KisCustomConvolutionFilterConfigurationBaseWidgetLayout->addItem( spacer5 );
    languageChange();
    resize( QSize(138, 230).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
}

/*
 *  Destroys the object and frees any allocated resources
 */
KisCustomConvolutionFilterConfigurationBaseWidget::~KisCustomConvolutionFilterConfigurationBaseWidget()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void KisCustomConvolutionFilterConfigurationBaseWidget::languageChange()
{
    setCaption( tr2i18n( "Custom Convolution Filter Configuration Widget" ) );
    textLabelFactor->setText( tr2i18n( "Factor :" ) );
    textLabelOffset->setText( tr2i18n( "Offset :" ) );
}

#include "kis_custom_convolution_filter_configuration_base_widget.moc"
