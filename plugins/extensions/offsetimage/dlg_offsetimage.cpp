/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "dlg_offsetimage.h"

#include <klocalizedstring.h>
#include <kis_debug.h>
#include <kis_config.h>

#include "kis_document_aware_spin_box_unit_manager.h"

const QString DlgOffsetImage::PARAM_PREFIX = "imageoffsetdlg";
const QString DlgOffsetImage::PARAM_XOFFSET_UNIT = DlgOffsetImage::PARAM_PREFIX + "_xoffsetunit";
const QString DlgOffsetImage::PARAM_YOFFSET_UNIT = DlgOffsetImage::PARAM_PREFIX + "_yoffsetunit";

DlgOffsetImage::DlgOffsetImage(QWidget *  parent, const char * name, QSize imageSize)
    :   KoDialog(parent),
      m_offsetSize(imageSize)
{
    setCaption("BUG: No sane caption is set");
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setObjectName(name);


    m_lock = false;

    m_page = new WdgOffsetImage(this);
    Q_CHECK_PTR(m_page);
    m_page->setObjectName("offset_image");

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    _widthUnitManager = new KisDocumentAwareSpinBoxUnitManager(this);
    _heightUnitManager = new KisDocumentAwareSpinBoxUnitManager(this, KisDocumentAwareSpinBoxUnitManager::PIX_DIR_Y);

    _widthUnitManager->setApparentUnitFromSymbol("px");
    _heightUnitManager->setApparentUnitFromSymbol("px");

    m_page->offsetXdoubleSpinBox->setUnitManager(_widthUnitManager);
    m_page->offsetYdoubleSpinBox->setUnitManager(_heightUnitManager);
    m_page->offsetXdoubleSpinBox->setDecimals(2);
    m_page->offsetYdoubleSpinBox->setDecimals(2);
    m_page->offsetXdoubleSpinBox->setDisplayUnit(false);
    m_page->offsetYdoubleSpinBox->setDisplayUnit(false);

    m_page->offsetXdoubleSpinBox->setReturnUnit("px");
    m_page->offsetYdoubleSpinBox->setReturnUnit("px");

    m_page->unitXComboBox->setModel(_widthUnitManager);
    m_page->unitYComboBox->setModel(_heightUnitManager);

    KisConfig cfg;

    QString unitx = cfg.readEntry<QString>(PARAM_XOFFSET_UNIT, "px");
    QString unity = cfg.readEntry<QString>(PARAM_YOFFSET_UNIT, "px");

    _widthUnitManager->setApparentUnitFromSymbol(unitx);
    _heightUnitManager->setApparentUnitFromSymbol(unity);

    const int xUnitIndex = _widthUnitManager->getsUnitSymbolList().indexOf(unitx);
    const int yUnitIndex = _heightUnitManager->getsUnitSymbolList().indexOf(unity);

    m_page->unitXComboBox->setCurrentIndex(xUnitIndex);
    m_page->unitYComboBox->setCurrentIndex(yUnitIndex);

    connect(this, SIGNAL(okClicked()),this, SLOT(okClicked()));
    connect(m_page->middleOffsetBtn, SIGNAL(clicked()), this, SLOT(slotMiddleOffset()));
    connect(m_page->offsetXdoubleSpinBox, SIGNAL(valueChangedPt(double)), this, SLOT(slotOffsetXChanged(double)));
    connect(m_page->offsetYdoubleSpinBox, SIGNAL(valueChangedPt(double)), this, SLOT(slotOffsetYChanged(double)));

    connect(m_page->unitXComboBox, SIGNAL(currentIndexChanged(int)), _widthUnitManager, SLOT(selectApparentUnitFromIndex(int)));
    connect(m_page->unitYComboBox, SIGNAL(currentIndexChanged(int)), _heightUnitManager, SLOT(selectApparentUnitFromIndex(int)));
    connect(_widthUnitManager, SIGNAL(unitChanged(int)), m_page->unitXComboBox, SLOT(setCurrentIndex(int)));
    connect(_heightUnitManager, SIGNAL(unitChanged(int)), m_page->unitYComboBox, SLOT(setCurrentIndex(int)));

    slotMiddleOffset();
}

DlgOffsetImage::~DlgOffsetImage()
{
    KisConfig cfg;

    cfg.writeEntry<QString>(PARAM_XOFFSET_UNIT, _widthUnitManager->getApparentUnitSymbol());
    cfg.writeEntry<QString>(PARAM_YOFFSET_UNIT, _heightUnitManager->getApparentUnitSymbol());

    delete m_page;
}

void DlgOffsetImage::slotOffsetXChanged(double newOffsetX)
{
    m_offsetX = qRound(newOffsetX);
}

void DlgOffsetImage::slotOffsetYChanged(double newOffsetY)
{
    m_offsetY = qRound(newOffsetY);
}

void DlgOffsetImage::slotMiddleOffset()
{
    int offsetX = m_offsetSize.width() / 2;
    int offsetY = m_offsetSize.height() / 2;
    m_page->offsetXdoubleSpinBox->changeValue(offsetX);
    m_page->offsetYdoubleSpinBox->changeValue(offsetY);
}

void DlgOffsetImage::okClicked()
{
    accept();
}


