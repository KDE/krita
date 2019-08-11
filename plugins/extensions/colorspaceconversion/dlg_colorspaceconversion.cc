/*
 *  dlg_colorspaceconversion.cc - part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include "dlg_colorspaceconversion.h"

#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QList>

#include <klocalizedstring.h>
#include <kis_debug.h>

#include "KoColorProfile.h"
#include "KoColorSpace.h"
#include "KoID.h"

#include "widgets/kis_cmb_idlist.h"
#include <KisSqueezedComboBox.h>// TODO: add a label that would display if there isn't a good color conversion path (use KoColorConversionSystem::isGoodPath), all color spaces shipped with Calligra are expected to have a good path, but better warn the user in case

DlgColorSpaceConversion::DlgColorSpaceConversion(QWidget *  parent,
        const char * name)
        : KoDialog(parent)
{
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setObjectName(name);
    m_page = new WdgConvertColorSpace(this);
    Q_CHECK_PTR(m_page);
    m_page->setObjectName("colorspace_conversion");

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    m_intentButtonGroup.addButton(m_page->radioAbsoluteColorimetric, KoColorConversionTransformation::IntentAbsoluteColorimetric);
    m_intentButtonGroup.addButton(m_page->radioPerceptual, KoColorConversionTransformation::IntentPerceptual);
    m_intentButtonGroup.addButton(m_page->radioRelativeColorimetric, KoColorConversionTransformation::IntentRelativeColorimetric);
    m_intentButtonGroup.addButton(m_page->radioSaturation, KoColorConversionTransformation::IntentSaturation);

    connect(this, SIGNAL(okClicked()),
            this, SLOT(okClicked()));


    connect(m_page->colorSpaceSelector, SIGNAL(selectionChanged(bool)), this, SLOT(selectionChanged(bool)));

}



DlgColorSpaceConversion::~DlgColorSpaceConversion()
{
    delete m_page;
}

void DlgColorSpaceConversion::setInitialColorSpace(const KoColorSpace *cs)
{
    if (!cs) {
        return;
    }
    if (cs->profile()->getEstimatedTRC()[0]==1.0) {
    //this tries to automatically determine whether optimizations ought to be checked or not.
    //if the space you're converting from is linear TRC, uncheck.
        m_page->chkAllowLCMSOptimization->setCheckState(Qt::Unchecked);
    } else {
        m_page->chkAllowLCMSOptimization->setCheckState(Qt::Checked);
    }
    m_page->colorSpaceSelector->setCurrentColorSpace(cs);
}

void DlgColorSpaceConversion::selectionChanged(bool valid)
{
    //TODO: Figure out how to uncheck when moving TO a linear TRC.
    Q_UNUSED(valid);
    enableButtonOk(m_page->colorSpaceSelector->currentColorSpace());
}

// SLOTS

void DlgColorSpaceConversion::okClicked()
{
    accept();
}

