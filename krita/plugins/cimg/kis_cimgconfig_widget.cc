/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
 *
 * Ported from the CImg Gimp plugin by Victor Stinner and David Tschumperlé.
 */
#include "qlayout.h"
#include "qcheckbox.h"
#include "qpushbutton.h"

#include "knuminput.h"

#include "kis_filter_configuration_widget.h"
#include "wdg_cimg.h"
#include "kis_cimgconfig_widget.h"
#include "kis_cimg_filter.h"

KisCImgconfigWidget::KisCImgconfigWidget(KisFilter* nfilter, QWidget * parent, const char * name, WFlags f)
	: KisFilterConfigurationWidget(nfilter, parent, name, f)
{
	m_page = new WdgCImg(this);
	QHBoxLayout * l = new QHBoxLayout(this);
	l -> add(m_page);
	filter() -> setAutoUpdate(false);
	connect( m_page -> bnRefresh, SIGNAL(clicked()), filter(), SLOT(refreshPreview()));
}


KisCImgFilterConfiguration * KisCImgconfigWidget::config()
{
	KisCImgFilterConfiguration * cfg = new KisCImgFilterConfiguration();

	cfg -> power1         = m_page -> numDetail -> value();
	cfg -> power2         = m_page -> numGradient -> value();
	cfg -> dt             = m_page -> numTimeStep -> value();
	cfg -> sigma          = m_page -> numBlur -> value();
	cfg -> nb_iter        = m_page -> numBlurIterations -> value();
	cfg -> dtheta         = m_page -> numAngularStep -> value();
	cfg -> dlength        = m_page -> numIntegralStep -> value();
	cfg -> gauss_prec     = m_page -> numGaussian -> value();
	cfg -> linear         = m_page -> chkLinearInterpolation -> isChecked();
	cfg -> onormalize     = m_page -> chkNormalize -> isChecked();

	return cfg;

}
#include "kis_cimgconfig_widget.moc"
