/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_pdf_import_widget.h"

#define UNSTABLE_POPPLER_QT4
// poppler's headers
#include <poppler-qt4.h>

// Qt's headers
#include <qradiobutton.h>

// KDE's headers
#include <kis_debug.h>
#include <k3listbox.h>
#include <knuminput.h>

// For ceil()
#include <math.h>


KisPDFImportWidget::KisPDFImportWidget(Poppler::Document* pdfDoc, QWidget * parent)
        : QWidget(parent), m_pdfDoc(pdfDoc)
{
    setupUi(this);
    m_pages.push_back(0); // The first page is selected
    updateMaxCanvasSize();

    for (int i = 1; i <= m_pdfDoc->numPages(); i++) {
        listPages->insertItem(QString::number(i));
    }

    connect(intWidth, SIGNAL(valueChanged(int)), this, SLOT(updateHRes()));
    connect(intHeight, SIGNAL(valueChanged(int)), this, SLOT(updateHVer()));
    connect(intHorizontal, SIGNAL(valueChanged(int)), this, SLOT(updateWidth()));
    connect(intVertical, SIGNAL(valueChanged(int)), this, SLOT(updateHeight()));
    connect(boolAllPages, SIGNAL(toggled(bool)), this, SLOT(selectAllPages(bool)));
    connect(boolFirstPage, SIGNAL(toggled(bool)), this, SLOT(selectFirstPage(bool)));
    connect(boolSelectionPage, SIGNAL(toggled(bool)), this, SLOT(selectSelectionOfPages(bool)));
    connect(listPages, SIGNAL(selectionChanged()), this, SLOT(updateSelectionOfPages()));
}


KisPDFImportWidget::~KisPDFImportWidget()
{
}

void KisPDFImportWidget::selectAllPages(bool v)
{
    if (v) {
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
        m_pages.clear();
        m_pages.push_back(0); // The first page is selected
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
        if (listPages->isSelected(i)) m_pages.push_back(i);
    }
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
    updateWidth();
    updateHeight();
}

void KisPDFImportWidget::updateWidth()
{
    intWidth->blockSignals(true);
    intWidth->setValue((int) ceil(m_maxWidthInch * intHorizontal->value()));
    intWidth->blockSignals(false);
}
void KisPDFImportWidget::updateHeight()
{
    intHeight->blockSignals(true);
    intHeight->setValue((int) ceil(m_maxHeightInch * intVertical->value()));
    intHeight->blockSignals(false);
}
void KisPDFImportWidget::updateHRes()
{
    intHorizontal->blockSignals(true);
    intHorizontal->setValue((int)(intWidth->value() / m_maxWidthInch));
    intHorizontal->blockSignals(false);
}
void KisPDFImportWidget::updateHVer()
{
    intVertical->blockSignals(true);
    intVertical->setValue((int)(intHeight->value() / m_maxHeightInch));
    intVertical->blockSignals(false);
}

#include "kis_pdf_import_widget.moc"
