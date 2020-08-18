/*
 *  Copyright (c) 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "DlgExportStoryboard.h"

#include "KoFileDialog.h"
#include <KisDialogStateSaver.h>

#include <QSpinBox>

DlgExportStoryboard::DlgExportStoryboard(ExportFormat format)
        : KoDialog()
        , m_format(format)
{
    m_page = new WdgExportStoryboard(this);

    setCaption(i18n("Export Storyboard"));
    setButtons(Apply | Cancel);
    setButtonText(Apply, i18n("Export"));
    setDefaultButton(Apply);

    m_page->lblSvgFileName->hide();
    m_page->lblSvgFileName->setText("");
    m_page->lblSvgFileName->setWordWrap(true);

    QMap<QString, QVariant> defaults;
    defaults[ m_page->spinboxFirstItem->objectName() ] = QVariant::fromValue<int>(0);
    defaults[ m_page->spinboxLastItem->objectName() ] = QVariant::fromValue<int>(0);
    defaults[ m_page->spinboxRow->objectName() ] = QVariant::fromValue<int>(3);
    defaults[ m_page->spinboxColumn->objectName() ] = QVariant::fromValue<int>(3);

    KisDialogStateSaver::restoreState(m_page, "krita/storyboard_export", defaults);

    connect(this, SIGNAL(applyClicked()), this, SLOT(slotExportClicked()));
    connect(m_page->btnSpecifySVG, SIGNAL(clicked()), this, SLOT(slotSpecifySvgClicked()));
    setMainWidget(m_page);
}

DlgExportStoryboard::~DlgExportStoryboard()
{
}

int DlgExportStoryboard::firstItem() const
{
    return m_page->spinboxFirstItem->value();
}

int DlgExportStoryboard::lastItem() const
{
    return m_page->spinboxLastItem->value();
}

int DlgExportStoryboard::rows() const
{
    return m_page->spinboxRow->value();
}

int DlgExportStoryboard::columns() const
{
    return m_page->spinboxColumn->value();
}

QPageSize DlgExportStoryboard::pageSize() const
{
    int index = m_page->cmbPageSize->currentIndex();
    switch (index) {
        case 0:
            return QPageSize(QPageSize::PageSizeId::A4);
        case 1:
            return QPageSize(QPageSize::PageSizeId::A0);
        case 2:
            return QPageSize(QPageSize::PageSizeId::A1);
        case 3:
            return QPageSize(QPageSize::PageSizeId::A2);
        case 4:
            return QPageSize(QPageSize::PageSizeId::A3);
        default:
            return QPageSize(QPageSize::PageSizeId::A5);
    }
}

QPageLayout::Orientation DlgExportStoryboard::pageOrientation() const
{
    return (QPageLayout::Orientation)m_page->cmbPageOrient->currentIndex();
}

QString DlgExportStoryboard::exportSvgFile() const
{
    return m_page->lblSvgFileName->text();
}

QString DlgExportStoryboard::saveFileName() const
{
    return m_exportFileName;
}

ExportFormat DlgExportStoryboard::format() const
{
    return m_format;
}

int DlgExportStoryboard::fontSize() const
{
    return m_page->spinboxFontSize->value();
}

void DlgExportStoryboard::slotExportClicked()
{
    KoFileDialog savedlg(this, KoFileDialog::SaveFile, "Export File location");
    savedlg.setCaption(i18nc("Export File loacation for storyboard", "Export File location"));
    savedlg.setDefaultDir(QDir::cleanPath(QDir::homePath()));

    QStringList mimeTypes;
    if (m_format == ExportFormat::PDF) {
        mimeTypes << "application/pdf";
    }
    if (m_format == ExportFormat::SVG) {
        mimeTypes << "image/svg+xml";
    }
    savedlg.setMimeTypeFilters(mimeTypes);

    m_exportFileName = "";
    m_exportFileName = savedlg.filename();
    if (m_exportFileName.isEmpty()) {
        this->cancelClicked();
    }

    KisDialogStateSaver::saveState(m_page, "krita/storyboard_export");
    accept();
}

void DlgExportStoryboard::slotSpecifySvgClicked()
{
    KoFileDialog filedlg(this, KoFileDialog::OpenFile, "layout specification file");
    filedlg.setCaption(i18nc("@title:window", "Choose SVG File to Specify Layout"));
    filedlg.setDefaultDir(QDir::cleanPath(QDir::homePath()));

    QStringList mimeTypes;
    mimeTypes << "image/svg+xml";
    filedlg.setMimeTypeFilters(mimeTypes);
    QString fileName = filedlg.filename();

    if (fileName.isEmpty()) {
        return;
    }

    QFile f(fileName);
    if (f.exists()) {
        m_page->lblSvgFileName->setText(fileName);
        m_page->lblSvgFileName->show();

        //disable rows and column and size spinboxes
        m_page->spinboxRow->setValue(0);
        m_page->spinboxColumn->setValue(0);

        m_page->spinboxRow->setEnabled(false);
        m_page->spinboxColumn->setEnabled(false);
        m_page->cmbPageSize->setEnabled(false);
        m_page->cmbPageOrient->setEnabled(false);
    }
}
