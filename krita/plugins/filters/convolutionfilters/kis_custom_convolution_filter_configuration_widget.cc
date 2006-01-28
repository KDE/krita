/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include "kis_custom_convolution_filter_configuration_widget.h"

#include <qlayout.h>
#include <qspinbox.h>

#include <klocale.h>

#include "kis_filter.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_view.h"
#include "kis_types.h"
#include "kis_colorspace.h"
#include "kis_custom_convolution_filter_configuration_base_widget.h"
#include "kis_matrix_widget.h"

KisCustomConvolutionFilterConfigurationWidget::KisCustomConvolutionFilterConfigurationWidget( KisFilter* nfilter, QWidget * parent, const char * name)
    : KisFilterConfigWidget ( parent, name )
{
    QGridLayout *widgetLayout = new QGridLayout(this, 2, 1);
    Q_CHECK_PTR(widgetLayout);

//     QPushButton *bnRefresh = new QPushButton(i18n("Refresh Preview"), this, "bnrefresh");
//     Q_CHECK_PTR(bnRefresh);

//     QSpacerItem *spacer = new QSpacerItem(100, 30, QSizePolicy::Expanding, QSizePolicy::Minimum);
//     Q_CHECK_PTR(spacer);

//     widgetLayout -> addWidget(bnRefresh, 0, 0);
//     widgetLayout -> addItem(spacer, 0, 1);

    m_ccfcws = new KisCustomConvolutionFilterConfigurationBaseWidget((QWidget*)this);
    Q_CHECK_PTR(m_ccfcws);

    widgetLayout -> addMultiCellWidget(m_ccfcws, 1, 1, 0, 1);

//     connect( bnRefresh, SIGNAL(clicked()), nfilter, SLOT(refreshPreview()));
    connect( m_ccfcws->matrixWidget, SIGNAL(valueChanged()), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_ccfcws->spinBoxFactor, SIGNAL(valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_ccfcws->spinBoxOffset, SIGNAL(valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
}

#include "kis_custom_convolution_filter_configuration_widget.moc"
