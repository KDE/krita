/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_pdf_import_widget.h"

#define UNSTABLE_POPPLER_QT4
// poppler's headers
#include <poppler-qt5.h>

// Qt's headers
#include <QRadioButton>

// KDE's headers
#include <kis_debug.h>

// For ceil()
#include <math.h>

// For KoAspectButton
#include <klocalizedstring.h>



KisPDFImportWidget::KisPDFImportWidget(Poppler::Document* pdfDoc, QWidget * parent)
        : QWidget(parent)
        , m_pdfDoc(pdfDoc)
        , m_keepAspect(true)
{
    setupUi(this);
    m_pages.push_back(0); // The first page is selected
    updateMaxCanvasSize();

    for (int i = 1; i <= m_pdfDoc->numPages(); i++) {
        listPages->addItem(QString::number(i));
    }

    connect(intWidth, SIGNAL(valueChanged(int)), this, SLOT(updateHRes()));
    connect(intHeight, SIGNAL(valueChanged(int)), this, SLOT(updateHVer()));
    connect(intResolution, SIGNAL(valueChanged(int)), this, SLOT(updateResolution()));
    connect(intHeight, SIGNAL(valueChanged(int)), this, SLOT(heightAspectRatio()));
    connect(intWidth, SIGNAL(valueChanged(int)), this, SLOT(widthAspectRatio()));
    connect(boolAllPages, SIGNAL(toggled(bool)), this, SLOT(selectAllPages(bool)));
    connect(boolFirstPage, SIGNAL(toggled(bool)), this, SLOT(selectFirstPage(bool)));
    connect(boolSelectionPage, SIGNAL(toggled(bool)), this, SLOT(selectSelectionOfPages(bool)));
    connect(listPages, SIGNAL(itemSelectionChanged()), this, SLOT(updateSelectionOfPages()));
    connect(pixelAspectRatioBtn, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(slotAspectChanged(bool)));

}


KisPDFImportWidget::~KisPDFImportWidget()
{
}

void KisPDFImportWidget::selectAllPages(bool v)
{
    if (v) {
        if (listPages->selectedItems().count() != 0){
            listPages->clearSelection();
            boolAllPages->toggle();
        }
        m_pages.clear();
        for (int i = 0; i < m_pdfDoc->numPages(); i++) {
            m_pages.push_back(i);
        }
        updateMaxCanvasSize();
    }
}
void KisPDFImportWidget::selectFirstPage(bool v)
{
    if (v) {
        if (listPages->selectedItems().count() != 0){
            listPages->clearSelection();
            boolFirstPage->toggle();
        }
        m_pages.clear();
        m_pages.push_back(0); // The first page is selected
        updateMaxCanvasSize();
    }
}
void KisPDFImportWidget::selectSelectionOfPages(bool v)
{
    if (v) {
        updateSelectionOfPages();
        updateMaxCanvasSize();
    }

}

void KisPDFImportWidget::updateSelectionOfPages()
{
    if (! boolSelectionPage->isChecked()) boolSelectionPage->toggle();
    m_pages.clear();
    for (int i = 0; i < m_pdfDoc->numPages(); i++) {
        if (listPages->item(i)->isSelected()) m_pages.push_back(i);
    }
    updateMaxCanvasSize();
}


void KisPDFImportWidget::updateMaxCanvasSize()
{
    m_maxWidthInch = 0., m_maxHeightInch = 0.;
    for (QList<int>::const_iterator it = m_pages.constBegin(); it != m_pages.constEnd(); ++it) {
        Poppler::Page *p = m_pdfDoc->page(*it);
        QSizeF size = p->pageSizeF();
        if (size.width() > m_maxWidthInch) {
            m_maxWidthInch = size.width();
        }
        if (size.height() > m_maxHeightInch) {
            m_maxHeightInch = size.height();
        }
    }
    m_maxWidthInch /= 72.;
    m_maxHeightInch /= 72.;
    dbgFile << m_maxWidthInch << "" << m_maxHeightInch;
    updateResolution();
}

void KisPDFImportWidget::updateResolution()
{
    int resolution = intResolution->value();
    intWidth->blockSignals(true);
    intWidth->setValue((int) ceil(m_maxWidthInch * resolution));
    intWidth->blockSignals(false);
    intHeight->blockSignals(true);
    intHeight->setValue((int) ceil(m_maxHeightInch * resolution));
    intHeight->blockSignals(false);
}

void KisPDFImportWidget::updateHRes()
{
    intResolution->blockSignals(true);
    intResolution->setValue((int)(intWidth->value() / m_maxWidthInch));
    intResolution->blockSignals(false);
}
void KisPDFImportWidget::updateHVer()
{
    intResolution->blockSignals(true);
    intResolution->setValue((int)(intHeight->value() / m_maxHeightInch));
    intResolution->blockSignals(false);
}
void KisPDFImportWidget::heightAspectRatio()
{
    intWidth->blockSignals(true);
    if(m_keepAspect)
    {
        intWidth->setValue((int) ceil(((intHeight->value() * m_maxWidthInch * 1.) / m_maxHeightInch * 1.) + 0.5));
    }
    intWidth->blockSignals(false);
}
void KisPDFImportWidget::widthAspectRatio()
{
    intHeight->blockSignals(true);
    if(m_keepAspect)
    {
        intHeight->setValue((int) ceil(((intWidth->value() * m_maxHeightInch * 1.) / m_maxWidthInch * 1.) + 0.5));
    }
    intHeight->blockSignals(false);
}
void KisPDFImportWidget::slotAspectChanged(bool keep)
{
    pixelAspectRatioBtn->blockSignals(true);
    pixelAspectRatioBtn->setKeepAspectRatio(keep);
    pixelAspectRatioBtn->blockSignals(false);

    m_keepAspect = keep;

    if (keep)
    {
        heightAspectRatio();
    }
}

