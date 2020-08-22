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
#include <kis_file_name_requester.h>

#include <QSpinBox>
#include <QMessageBox>

DlgExportStoryboard::DlgExportStoryboard(ExportFormat format)
        : KoDialog()
        , m_format(format)
{
    m_page = new WdgExportStoryboard(this);

    setCaption(i18n("Export Storyboard as ") + (format == ExportFormat::PDF ? "PDF":"SVG"));
    setButtons(Apply | Cancel);
    setButtonText(Apply, i18n("Export"));
    setDefaultButton(Apply);

    connect(this, SIGNAL(applyClicked()), this, SLOT(slotExportClicked()));
    connect(m_page->chkUseSVGLayout, SIGNAL(stateChanged(int)), this, SLOT(slotChkUseSvgLayoutChanged(int)));

    m_page->spinboxRow->setMinimum(1);
    m_page->spinboxColumn->setMinimum(1);

    QMap<QString, QVariant> defaults;
    defaults[ m_page->spinboxFirstItem->objectName() ] = QVariant::fromValue<int>(0);
    defaults[ m_page->spinboxLastItem->objectName() ] = QVariant::fromValue<int>(0);
    defaults[ m_page->spinboxRow->objectName() ] = QVariant::fromValue<int>(3);
    defaults[ m_page->spinboxColumn->objectName() ] = QVariant::fromValue<int>(3);
    defaults[ m_page->spinboxFontSize->objectName() ] = QVariant::fromValue<int>(15);

    KisDialogStateSaver::restoreState(m_page, "krita/storyboard_export", defaults);

    if (format == ExportFormat::PDF) {
        m_page->svgFileBaseName->hide();
        m_page->lblSvgFileBaseName->hide();

        QStringList mimeTypes;
        mimeTypes << "application/pdf";
        m_page->exportFileName->setMimeTypeFilters(mimeTypes);

        m_page->exportFileName->setMode(KoFileDialog::SaveFile);
    }
    else {
        m_page->exportFileName->setMode(KoFileDialog::OpenDirectory);
        m_page->lblExportFileName->setText("Export Directory : ");
    }

    QStringList mimeTypes;
    mimeTypes << "image/svg+xml";
    m_page->svgLayoutFileName->setMimeTypeFilters(mimeTypes);
    m_page->svgLayoutFileName->setMode(KoFileDialog::OpenFile);
    slotChkUseSvgLayoutChanged(layoutSpecifiedBySvgFile());

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

bool DlgExportStoryboard::layoutSpecifiedBySvgFile() const
{
    return m_page->chkUseSVGLayout->isChecked();
}

QString DlgExportStoryboard::layoutSvgFile() const
{
    return m_page->svgLayoutFileName->fileName();
}

QString DlgExportStoryboard::saveFileName() const
{
    return m_page->exportFileName->fileName();
}

QString DlgExportStoryboard::svgFileBaseName() const
{
    return m_page->svgFileBaseName->text();
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
    if (m_page->exportFileName->fileName().isEmpty()) {
        if (m_format == ExportFormat::PDF) {
            QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("Please enter a file name to export to."));
        }
        else {
            QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("Please enter a directory to export to."));
        }
        return;
    }

    if (m_format == ExportFormat::SVG) {

        QDir dir(m_page->exportFileName->fileName());
        if (!dir.exists()) {
            QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("Please enter an existing directory."));
            return;
        }

        QFileInfo info(svgFileBaseName() + "[0-9]*.svg");
        QStringList filesList = dir.entryList({ info.fileName() });

        if (!filesList.isEmpty()) {
            QMessageBox::StandardButton result =
                QMessageBox::warning(0,
                                     i18n("Existing files with similar naming scheme"),
                                     i18n("Files with the same naming "
                                          "scheme exist in the destination "
                                          "directory. They might be "
                                          "deleted, continue?\n\n"
                                          "Directory: %1\n"
                                          "Files: %2",
                                          dir.absolutePath(), filesList.at(0) + "..."),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);
            if (result == QMessageBox::No) {
                return;
            }
        }
    }

    if (m_page->chkUseSVGLayout->isChecked() && m_page->svgLayoutFileName->fileName().isEmpty()) {
        QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("Please choose svg file to specify the layout for exporting."));
        return;
    }
    QFileInfo fi(m_page->svgLayoutFileName->fileName());
    if (m_page->chkUseSVGLayout->isChecked() && !fi.exists()) {
        QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("The SVG file to specify layout doesn't exist. Please choose an existing SVG file."));
        return;
    }

    KisDialogStateSaver::saveState(m_page, "krita/storyboard_export");
    accept();
}

void DlgExportStoryboard::slotChkUseSvgLayoutChanged(int state)
{
    m_page->spinboxRow->setEnabled(state != Qt::Checked);
    m_page->spinboxColumn->setEnabled(state != Qt::Checked);
    m_page->cmbPageSize->setEnabled(state != Qt::Checked);
    m_page->cmbPageOrient->setEnabled(state != Qt::Checked);
    m_page->svgLayoutFileName->setEnabled(state == Qt::Checked);
}
