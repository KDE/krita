/*
 *  dlg_imagesplit.cc - part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
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

#include "dlg_imagesplit.h"

#include <klocalizedstring.h>
#include <kis_debug.h>

#include <KisViewManager.h>
#include <kis_image.h>
#include <kis_paint_device.h>

DlgImagesplit::DlgImagesplit(KisViewManager* view, const QString &suffix, QStringList listMimeFilter, int defaultMimeIndex)
    : KoDialog(view->mainWindow())
{
    m_page = new WdgImagesplit(this);

    setCaption(i18n("Image Split"));
    setButtons(Apply | Close);
    setDefaultButton(Apply);

    connect(this, SIGNAL(applyClicked()), this, SLOT(applyClicked()));

    setMainWidget(m_page);
    m_page->lineEdit->setText(suffix);
    m_page->cmbFileType->clear();
    m_page->cmbFileType->addItems(listMimeFilter);
    m_page->cmbFileType->setCurrentIndex(defaultMimeIndex);
    cmbIndex = defaultMimeIndex;

    connect(m_page->chkAutoSave, SIGNAL(stateChanged(int)), SLOT(lineEditEnable()));
    connect(m_page->cmbFileType, SIGNAL(activated(int)), this, SLOT(setMimeType(int)));
}

DlgImagesplit::~DlgImagesplit()
{
}

void DlgImagesplit::lineEditEnable()
{
    if (m_page->chkAutoSave->isChecked()) {
        m_page->lblSuffix->setEnabled(true);
        m_page->lineEdit->setEnabled(true);
        m_page->lblFileType->setEnabled(true);
        m_page->cmbFileType->setEnabled(true);
    }
    else
    {
        m_page->lblSuffix->setEnabled(false);
        m_page->lineEdit->setEnabled(false);
        m_page->lblFileType->setEnabled(false);
        m_page->cmbFileType->setEnabled(false);
    }

}

bool DlgImagesplit::autoSave()
{
    if (m_page->chkAutoSave->isChecked())
        return true;
    else
        return false;
}

bool DlgImagesplit::sortHorizontal()
{
    if (m_page->chkHorizontal->isChecked())
        return true;
    else
        return false;
}

int DlgImagesplit::horizontalLines()
{
    return m_page->intHorizontalSplitLines->value();
}

int DlgImagesplit::verticalLines()
{
    return m_page->intVerticalSplitLines->value();
}

QString DlgImagesplit::suffix()
{
    return m_page->lineEdit->text();
}

void DlgImagesplit::setMimeType(int index)
{
    cmbIndex = index;
}

void DlgImagesplit::applyClicked()
{
    accept();
}

