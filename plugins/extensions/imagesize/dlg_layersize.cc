/*
 *  dlg_layersize.cc - part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Sven Langkamp <sven.langkamp@gmail.com>
 *  Copyright (c) 2013 Juan Palacios <jpalaciosdev@gmail.com>
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

#include "dlg_layersize.h"

#include <KoUnit.h>
#include <kis_config.h>

#include <klocalizedstring.h>

#include <kis_document_aware_spin_box_unit_manager.h>

#include <kis_filter_strategy.h>// XXX: I'm really real bad at arithmetic, let alone math. Here
// be rounding errors. (Boudewijn)

const QString DlgLayerSize::PARAM_PREFIX = "layersizedlg";

const QString DlgLayerSize::PARAM_WIDTH_UNIT = DlgLayerSize::PARAM_PREFIX + "_widthunit";
const QString DlgLayerSize::PARAM_HEIGHT_UNIT = DlgLayerSize::PARAM_PREFIX + "_heightunit";

const QString DlgLayerSize::PARAM_KEEP_AR = DlgLayerSize::PARAM_PREFIX + "_keepar";
const QString DlgLayerSize::PARAM_KEEP_PROP = DlgLayerSize::PARAM_PREFIX + "_keepprop";

static const QString pixelStr(KoUnit::unitDescription(KoUnit::Pixel));
static const QString percentStr(i18n("Percent (%)"));

DlgLayerSize::DlgLayerSize(QWidget *  parent, const char * name,
                           int width, int height, double resolution)
    : KoDialog(parent)
    , m_aspectRatio(((double) width) / height)
    , m_originalWidth(width)
    , m_originalHeight(height)
    , m_width(width)
    , m_height(height)
    , m_resolution(resolution)
    , m_keepAspect(true)
{
    setCaption(i18n("Layer Size"));
    setObjectName(name);
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    m_page = new WdgLayerSize(this);
    Q_CHECK_PTR(m_page);
    m_page->layout()->setMargin(0);
    m_page->setObjectName(name);

    KisConfig cfg(true);

    _widthUnitManager = new KisDocumentAwareSpinBoxUnitManager(this);
    _heightUnitManager = new KisDocumentAwareSpinBoxUnitManager(this, KisDocumentAwareSpinBoxUnitManager::PIX_DIR_Y);

    _widthUnitManager->setApparentUnitFromSymbol("px");
    _heightUnitManager->setApparentUnitFromSymbol("px");

    m_page->newWidthDouble->setUnitManager(_widthUnitManager);
    m_page->newHeightDouble->setUnitManager(_heightUnitManager);
    m_page->newWidthDouble->setDecimals(2);
    m_page->newHeightDouble->setDecimals(2);
    m_page->newWidthDouble->setDisplayUnit(false);
    m_page->newHeightDouble->setDisplayUnit(false);

    m_page->newWidthDouble->setValue(width);
    m_page->newWidthDouble->setFocus();
    m_page->newHeightDouble->setValue(height);

    m_page->filterCmb->setIDList(KisFilterStrategyRegistry::instance()->listKeys());
    m_page->filterCmb->setToolTip(KisFilterStrategyRegistry::instance()->formattedDescriptions());
    m_page->filterCmb->setCurrent("Bicubic");

    m_page->newWidthUnit->setModel(_widthUnitManager);
    m_page->newHeightUnit->setModel(_heightUnitManager);

    QString unitw = cfg.readEntry<QString>(PARAM_WIDTH_UNIT, "px");
    QString unith = cfg.readEntry<QString>(PARAM_HEIGHT_UNIT, "px");

    _widthUnitManager->setApparentUnitFromSymbol(unitw);
    _heightUnitManager->setApparentUnitFromSymbol(unith);

    const int wUnitIndex = _widthUnitManager->getsUnitSymbolList().indexOf(unitw);
    const int hUnitIndex = _widthUnitManager->getsUnitSymbolList().indexOf(unith);

    m_page->newWidthUnit->setCurrentIndex(wUnitIndex);
    m_page->newHeightUnit->setCurrentIndex(hUnitIndex);

    m_keepAspect = cfg.readEntry(PARAM_KEEP_AR,true);
    m_page->aspectRatioBtn->setKeepAspectRatio(m_keepAspect);
    m_page->constrainProportionsCkb->setChecked(cfg.readEntry(PARAM_KEEP_PROP,true));

    setMainWidget(m_page);
    connect(this, SIGNAL(okClicked()), this, SLOT(accept()));

    connect(m_page->aspectRatioBtn, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(slotAspectChanged(bool)));
    connect(m_page->constrainProportionsCkb, SIGNAL(toggled(bool)), this, SLOT(slotAspectChanged(bool)));

    connect(m_page->newWidthDouble, SIGNAL(valueChangedPt(double)), this, SLOT(slotWidthChanged(double)));
    connect(m_page->newHeightDouble, SIGNAL(valueChangedPt(double)), this, SLOT(slotHeightChanged(double)));

    connect(m_page->newWidthUnit, SIGNAL(currentIndexChanged(int)), _widthUnitManager, SLOT(selectApparentUnitFromIndex(int)));
    connect(m_page->newHeightUnit, SIGNAL(currentIndexChanged(int)), _heightUnitManager, SLOT(selectApparentUnitFromIndex(int)));
    connect(_widthUnitManager, SIGNAL(unitChanged(int)), m_page->newWidthUnit, SLOT(setCurrentIndex(int)));
    connect(_heightUnitManager, SIGNAL(unitChanged(int)), m_page->newHeightUnit, SLOT(setCurrentIndex(int)));
}

DlgLayerSize::~DlgLayerSize()
{

    KisConfig cfg(false);

    cfg.writeEntry<bool>(PARAM_KEEP_AR, m_page->aspectRatioBtn->keepAspectRatio());
    cfg.writeEntry<bool>(PARAM_KEEP_PROP, m_page->constrainProportionsCkb->isChecked());

    cfg.writeEntry<QString>(PARAM_WIDTH_UNIT, _widthUnitManager->getApparentUnitSymbol());
    cfg.writeEntry<QString>(PARAM_HEIGHT_UNIT, _heightUnitManager->getApparentUnitSymbol());

    delete m_page;
}

qint32 DlgLayerSize::width()
{
    return (qint32)m_width;
}

qint32 DlgLayerSize::height()
{
    return (qint32)m_height;
}

KisFilterStrategy *DlgLayerSize::filterType()
{
    KoID filterID = m_page->filterCmb->currentItem();
    KisFilterStrategy *filter = KisFilterStrategyRegistry::instance()->value(filterID.id());
    return filter;
}

// SLOTS

void DlgLayerSize::slotWidthChanged(double w)
{

    //this slot receiv values in pt from the unitspinbox, so just use the resolution.
    const double resValue = w*_widthUnitManager->getConversionFactor(KisSpinBoxUnitManager::LENGTH, "px");
    m_width = qRound(resValue);

    if (m_keepAspect) {
        m_height = qRound(m_width / m_aspectRatio);
        m_page->newHeightDouble->blockSignals(true);
        m_page->newHeightDouble->changeValue(w / m_aspectRatio);
        m_page->newHeightDouble->blockSignals(false);
    }

}

void DlgLayerSize::slotHeightChanged(double h)
{

    //this slot receiv values in pt from the unitspinbox, so just use the resolution.
    const double resValue = h*_heightUnitManager->getConversionFactor(KisSpinBoxUnitManager::LENGTH, "px");
    m_height = qRound(resValue);

    if (m_keepAspect) {
        m_width = qRound(m_height * m_aspectRatio);
        m_page->newWidthDouble->blockSignals(true);
        m_page->newWidthDouble->changeValue(h * m_aspectRatio);
        m_page->newWidthDouble->blockSignals(false);
    }
}

void DlgLayerSize::slotAspectChanged(bool keep)
{
    m_page->aspectRatioBtn->blockSignals(true);
    m_page->constrainProportionsCkb->blockSignals(true);

    m_page->aspectRatioBtn->setKeepAspectRatio(keep);
    m_page->constrainProportionsCkb->setChecked(keep);

    m_page->aspectRatioBtn->blockSignals(false);
    m_page->constrainProportionsCkb->blockSignals(false);

    m_keepAspect = keep;

    if (keep) {
        // values may be out of sync, so we need to reset it to defaults
        m_width = m_originalWidth;
        m_height = m_originalHeight;

        updateWidthUIValue(m_width);
        updateHeightUIValue(m_height);
    }
}

void DlgLayerSize::updateWidthUIValue(double value)
{
    m_page->newWidthDouble->blockSignals(true);
    const double resValue = value/_widthUnitManager->getConversionFactor(KisSpinBoxUnitManager::LENGTH, "px");
    m_page->newWidthDouble->changeValue(resValue);
    m_page->newWidthDouble->blockSignals(false);
}

void DlgLayerSize::updateHeightUIValue(double value)
{
    m_page->newHeightDouble->blockSignals(true);
    const double resValue = value/_heightUnitManager->getConversionFactor(KisSpinBoxUnitManager::LENGTH, "px");
    m_page->newHeightDouble->changeValue(resValue);
    m_page->newHeightDouble->blockSignals(false);
}

