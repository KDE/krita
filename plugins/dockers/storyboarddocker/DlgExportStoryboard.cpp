/*
 *  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "DlgExportStoryboard.h"

#include "KoFileDialog.h"
#include <kis_config.h>
#include <kis_file_name_requester.h>
#include <kis_time_span.h>

#include <QSpinBox>
#include <QMessageBox>

DlgExportStoryboard::DlgExportStoryboard(ExportFormat format, KisTimeSpan span)
        : KoDialog()
        , m_format(format)
{
    m_page = new WdgExportStoryboard(this);

    setCaption(format == ExportFormat::PDF
               ? i18nc("Export storyboard dialog caption", "Export Storyboard as PDF")
               : i18nc("Export storyboard dialog caption", "Export Storyboard as SVG"));

    setButtons(Apply | Cancel);
    setButtonText(Apply, i18n("Export"));
    setDefaultButton(Apply);

    connect(this, SIGNAL(applyClicked()), this, SLOT(slotExportClicked()));
    connect(m_page->chkUseSVGLayout, SIGNAL(stateChanged(int)), this, SLOT(slotChkUseSvgLayoutChanged(int)));

    m_page->spinboxRow->setMinimum(1);
    m_page->spinboxColumn->setMinimum(1);

    KisConfig cfg(true);
    m_page->spinboxFirstItem->setValue(span.start());
    m_page->spinboxLastItem->setValue(span.end());
    m_page->spinboxRow->setValue(cfg.readEntry<int>("storyboard/rows", 3));
    m_page->spinboxColumn->setValue(cfg.readEntry<int>("storyboard/columns", 3));
    m_page->spinboxFontSize->setValue(cfg.readEntry<int>("storyboard/fontSize", 15));
    m_page->svgLayoutFileName->setFileName(cfg.readEntry<QString>("storyboard/svgLayoutFileName", ""));
    m_page->svgFileBaseName->setText(cfg.readEntry<QString>("storyboard/svgFileBaseName", ""));
    m_page->chkUseSVGLayout->setChecked(cfg.readEntry<bool>("storyboard/chkUseSVGLayout", false));
    m_page->exportFileName->setFileName(cfg.readEntry<QString>("storyboard/exportFileName", ""));

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
        m_page->lblExportFileName->setText(i18n("Export Directory : "));
    }

    QStringList mimeTypes;
    mimeTypes << "image/svg+xml";
    m_page->svgLayoutFileName->setMimeTypeFilters(mimeTypes);
    m_page->svgLayoutFileName->setMode(KoFileDialog::OpenFile);

    setMainWidget(m_page);
    slotChkUseSvgLayoutChanged(m_page->chkUseSVGLayout->checkState());
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
    if (firstItem() > lastItem()) {
        QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("Please enter correct range. The first frame should be less than or equal to the last frame."));
        return;
    }

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

    KisConfig cfg(false);
    cfg.writeEntry("storyboard/firstItem", m_page->spinboxFirstItem->value());
    cfg.writeEntry("storyboard/lastItem", m_page->spinboxLastItem->value());
    cfg.writeEntry("storyboard/rows", m_page->spinboxRow->value());
    cfg.writeEntry("storyboard/columns", m_page->spinboxColumn->value());
    cfg.writeEntry("storyboard/fontSize", m_page->spinboxFontSize->value());
    cfg.writeEntry("storyboard/svgLayoutFileName", m_page->svgLayoutFileName->fileName());
    cfg.writeEntry("storyboard/svgFileBaseName", m_page->svgFileBaseName->text());
    cfg.writeEntry("storyboard/chkUseSVGLayout", m_page->chkUseSVGLayout->isChecked());
    cfg.writeEntry("storyboard/exportFileName", m_page->exportFileName->fileName());

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
