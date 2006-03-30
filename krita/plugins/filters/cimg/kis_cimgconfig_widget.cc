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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Ported from the CImg Gimp plugin by Victor Stinner and David Tschumperlé.
 */
#include "qlayout.h"
#include "qcheckbox.h"
#include "qpushbutton.h"
//Added by qt3to4:
#include <Q3HBoxLayout>

#include "knuminput.h"

#include "wdg_cimg.h"
#include "kis_cimgconfig_widget.h"
#include "kis_cimg_filter.h"

KisCImgconfigWidget::KisCImgconfigWidget(KisFilter* nfilter, QWidget * parent, const char * name, Qt::WFlags f)
    : KisFilterConfigWidget(parent, name, f)
{
    m_page = new WdgCImg(this);
    Q_CHECK_PTR(m_page);

    Q3HBoxLayout * l = new Q3HBoxLayout(this);
    Q_CHECK_PTR(l);

    l->add(m_page);
    nfilter->setAutoUpdate(false);
    
//     connect(  m_page->bnRefresh, SIGNAL(clicked()), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->numDetail, SIGNAL(valueChanged (double)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->numGradient, SIGNAL(valueChanged (double)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->numTimeStep, SIGNAL(valueChanged (double)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->numBlur, SIGNAL(valueChanged (double)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->numBlurIterations, SIGNAL(valueChanged (int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->numAngularStep, SIGNAL(valueChanged (double)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->numIntegralStep, SIGNAL(valueChanged (double)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->numGaussian, SIGNAL(valueChanged (double)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->chkLinearInterpolation, SIGNAL(toggled(bool)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_page->chkNormalize, SIGNAL(toggled(bool)), SIGNAL(sigPleaseUpdatePreview()));
}


KisCImgFilterConfiguration * KisCImgconfigWidget::config()
{
    KisCImgFilterConfiguration * cfg = new KisCImgFilterConfiguration();
    Q_CHECK_PTR(cfg);

    cfg->power1         = m_page->numDetail->value();
    cfg->power2         = m_page->numGradient->value();
    cfg->dt             = m_page->numTimeStep->value();
    cfg->sigma          = m_page->numBlur->value();
    cfg->nb_iter        = m_page->numBlurIterations->value();
    cfg->dtheta         = m_page->numAngularStep->value();
    cfg->dlength        = m_page->numIntegralStep->value();
    cfg->gauss_prec     = m_page->numGaussian->value();
    cfg->linear         = m_page->chkLinearInterpolation->isChecked();
    cfg->onormalize     = m_page->chkNormalize->isChecked();

    return cfg;

}

void KisCImgconfigWidget::setConfiguration(KisFilterConfiguration * config)
{
    KisCImgFilterConfiguration * cfg = dynamic_cast<KisCImgFilterConfiguration*>(config);
    if (!cfg) return;

    m_page->numDetail->setValue(cfg->power1);
    m_page->numGradient->setValue(cfg->power2);
    m_page->numTimeStep->setValue(cfg->dt);
    m_page->numBlur->setValue(cfg->sigma);
    m_page->numAngularStep->setValue(cfg->nb_iter);
    m_page->numIntegralStep->setValue(cfg->dlength);
    m_page->numGaussian->setValue(cfg->gauss_prec);
    m_page->chkLinearInterpolation->setChecked(cfg->linear);
    m_page->chkNormalize->setChecked(cfg->onormalize);
}

#include "kis_cimgconfig_widget.moc"
