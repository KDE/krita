/*
 *  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "DlgExportStoryboard.h"
#include "StoryboardModel.h"

#include "KoFileDialog.h"
#include <kis_config.h>
#include <kis_file_name_requester.h>
#include <kis_time_span.h>

#include <QSpinBox>
#include <QMessageBox>

DlgExportStoryboard::DlgExportStoryboard(ExportFormat format, QSharedPointer<StoryboardModel> model)
        : KoDialog()
        , m_format(format)
        , m_model(model)
{
    m_page = new WdgExportStoryboard(this);

    setCaption(format == ExportFormat::PDF
               ? i18nc("Export storyboard dialog caption", "Export Storyboard as PDF")
               : i18nc("Export storyboard dialog caption", "Export Storyboard as SVG"));

    setButtons(Apply | Cancel);
    setButtonText(Apply, i18n("Export"));
    setDefaultButton(Apply);

    connect(this, SIGNAL(applyClicked()), this, SLOT(slotExportClicked()));
    connect(m_page->boardLayoutComboBox, SIGNAL(activated(int)), this, SLOT(slotLayoutChanged(int)));
    connect(m_page->pageSizeComboBox, SIGNAL(activated(int)), this, SLOT(slotPageSettingsChanged(int)));
    connect(m_page->pageOrientationComboBox, SIGNAL(activated(int)), this, SLOT(slotPageSettingsChanged(int)));
    connect(m_page->rowsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotPageSettingsChanged(int)));
    connect(m_page->columnsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotPageSettingsChanged(int)));

    KisConfig cfg(true);
    m_page->boardLayoutComboBox->setCurrentIndex(cfg.readEntry<int>("storyboard/layoutType", ExportLayout::ROWS));
    m_page->pageOrientationComboBox->setCurrentIndex(cfg.readEntry<int>("storyboard/pageOrientation", 0));
    m_page->rowsSpinBox->setValue(cfg.readEntry<int>("storyboard/rows", 3));
    m_page->columnsSpinBox->setValue(cfg.readEntry<int>("storyboard/columns", 3));
    m_page->fontSizeSpinBox->setValue(cfg.readEntry<int>("storyboard/fontSize", 15));
    m_page->svgTemplatePathFileRequester->setFileName(cfg.readEntry<QString>("storyboard/svgLayoutFileName", ""));
    m_page->exportPathFileRequester->setFileName(cfg.readEntry<QString>("storyboard/exportFilePath"));

    if (format == ExportFormat::PDF) {
        QStringList mimeTypes;
        mimeTypes << "application/pdf";
        m_page->exportPathFileRequester->setMimeTypeFilters(mimeTypes);
        m_page->exportPathFileRequester->setMode(KoFileDialog::SaveFile);
    }
    else {
        m_page->exportPathFileRequester->setMode(KoFileDialog::OpenDirectory);
    }

    QStringList mimeTypes;
    mimeTypes << "image/svg+xml";
    m_page->svgTemplatePathFileRequester->setMimeTypeFilters(mimeTypes);
    m_page->svgTemplatePathFileRequester->setMode(KoFileDialog::OpenFile);

    setMainWidget(m_page);
    slotLayoutChanged(m_page->boardLayoutComboBox->currentIndex());
    setUsableMaximums(pageSize(), pageOrientation(), exportLayout());
}

DlgExportStoryboard::~DlgExportStoryboard()
{
}


int DlgExportStoryboard::rows() const
{
    const int layoutIndex = m_page->boardLayoutComboBox->currentIndex();
    if (layoutIndex == ExportLayout::COLUMNS || layoutIndex == ExportLayout::SVG_TEMPLATE) {
        return 1;
    }
    else {
        return qMax(m_page->rowsSpinBox->value(), 1);
    }
}

int DlgExportStoryboard::columns() const
{
    const int layoutIndex = m_page->boardLayoutComboBox->currentIndex();
    if (layoutIndex == ExportLayout::ROWS || layoutIndex == ExportLayout::SVG_TEMPLATE) {
        return 1;
    }
    else {
        return qMax(m_page->columnsSpinBox->value(), 1);
    }
}

QPageSize DlgExportStoryboard::pageSize() const
{
    int index = m_page->pageSizeComboBox->currentIndex();
    switch (index) {
        case 0:
            return QPageSize(QPageSize::PageSizeId::A0);
        case 1:
            return QPageSize(QPageSize::PageSizeId::A1);
        case 2:
            return QPageSize(QPageSize::PageSizeId::A2);
        case 3:
            return QPageSize(QPageSize::PageSizeId::A3);
        case 4:
            return QPageSize(QPageSize::PageSizeId::A4);
        case 5:
            return QPageSize(QPageSize::PageSizeId::A5);
        case 6:
        default:
            return QPageSize(QPageSize::PageSizeId::Letter);
    }
}

QPageLayout::Orientation DlgExportStoryboard::pageOrientation() const
{
    return (QPageLayout::Orientation)m_page->pageOrientationComboBox->currentIndex();
}

bool DlgExportStoryboard::layoutSpecifiedBySvgFile() const
{
    const int layoutIndex = m_page->boardLayoutComboBox->currentIndex();
    return layoutIndex == ExportLayout::SVG_TEMPLATE;
}

QString DlgExportStoryboard::layoutSvgFile() const
{
    return m_page->svgTemplatePathFileRequester->fileName();
}

QString DlgExportStoryboard::saveFileName() const
{
    return m_page->exportPathFileRequester->fileName();
}

ExportFormat DlgExportStoryboard::format() const
{
    return m_format;
}

ExportLayout DlgExportStoryboard::exportLayout() const
{
    return static_cast<ExportLayout>(m_page->boardLayoutComboBox->currentIndex());
}

int DlgExportStoryboard::fontSize() const
{
    return m_page->fontSizeSpinBox->value();
}

void DlgExportStoryboard::setUsableMaximums(QPageSize pPageSize, QPageLayout::Orientation pOrientation, ExportLayout pLayout)
{
    Q_UNUSED(pLayout);
    const QSize pointSize = pPageSize.sizePoints();
    const QSize orientedPointSize = pOrientation == QPageLayout::Landscape ? QSize(pointSize.height(), pointSize.width()) : pointSize;
    const QSize sizeInPointsPerBoard = QSize(orientedPointSize.width() / columns(), orientedPointSize.height() / rows());

    const int commentCount = m_model ? qMax(m_model->totalCommentCount(), 1) : 1;
    const bool stacked = sizeInPointsPerBoard.width() < sizeInPointsPerBoard.height();
    const QSize sizeInPointsPerComment = stacked ? QSize(sizeInPointsPerBoard.width(), sizeInPointsPerBoard.height() / commentCount)
                                                 : QSize(sizeInPointsPerBoard.width() / commentCount, sizeInPointsPerBoard.height());
    const QSize usableMaximumFontSize = sizeInPointsPerComment / 12;
    m_page->fontSizeSpinBox->setMaximum(qMin(usableMaximumFontSize.width(), usableMaximumFontSize.height()));
}

void DlgExportStoryboard::slotExportClicked()
{
    if (m_page->exportPathFileRequester->fileName().isEmpty()) {
        if (m_format == ExportFormat::PDF) {
            QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("Please enter a file name to export to."));
        }
        else {
            QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("Please enter a directory to export to."));
        }
        return;
    }

    if (m_format == ExportFormat::SVG) {

        QDir dir(m_page->exportPathFileRequester->fileName());
        if (!dir.exists()) {
            QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("Please enter an existing directory."));
            return;
        }

        QFileInfo info("[0-9]*.svg");
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

    if (layoutSpecifiedBySvgFile() && m_page->svgTemplatePathFileRequester->fileName().isEmpty()) {
        QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("Please choose svg file to specify the layout for exporting."));
        return;
    }
    QFileInfo fi(m_page->svgTemplatePathFileRequester->fileName());
    if (layoutSpecifiedBySvgFile() && !fi.exists()) {
        QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("The SVG file to specify layout doesn't exist. Please choose an existing SVG file."));
        return;
    }

    KisConfig cfg(false);
    cfg.writeEntry("storyboard/layoutType", m_page->boardLayoutComboBox->currentIndex());
    cfg.writeEntry("storyboard/pageOrientation", m_page->pageOrientationComboBox->currentIndex());
    cfg.writeEntry("storyboard/rows", m_page->rowsSpinBox->value());
    cfg.writeEntry("storyboard/columns", m_page->columnsSpinBox->value());
    cfg.writeEntry("storyboard/svgLayoutFileName", m_page->svgTemplatePathFileRequester->fileName());
    cfg.writeEntry("storyboard/exportFilePath", m_page->exportPathFileRequester->fileName());
    cfg.writeEntry("storyboard/fontSize", m_page->fontSizeSpinBox->value());

    accept();
}

void DlgExportStoryboard::slotLayoutChanged(int state)
{
    switch (state) {
    case ExportLayout::COLUMNS:
        m_page->rowsLabel->hide();
        m_page->rowsSpinBox->hide();
        m_page->svgTemplatePathFileRequester->hide();
        m_page->svgTemplatePathLabel->hide();

        m_page->columnsSpinBox->show();
        m_page->columnsLabel->show();
        break;
    case ExportLayout::ROWS:
        m_page->columnsLabel->hide();
        m_page->columnsSpinBox->hide();
        m_page->svgTemplatePathFileRequester->hide();
        m_page->svgTemplatePathLabel->hide();

        m_page->rowsSpinBox->show();
        m_page->rowsLabel->show();
        break;
    case ExportLayout::GRID:
        m_page->svgTemplatePathFileRequester->hide();
        m_page->svgTemplatePathLabel->hide();

        m_page->columnsLabel->show();
        m_page->columnsSpinBox->show();
        m_page->rowsSpinBox->show();
        m_page->rowsLabel->show();
        break;
    case ExportLayout::SVG_TEMPLATE:
        m_page->columnsLabel->hide();
        m_page->columnsSpinBox->hide();
        m_page->rowsSpinBox->hide();
        m_page->rowsLabel->hide();

        m_page->svgTemplatePathFileRequester->show();
        m_page->svgTemplatePathLabel->show();
        break;
    }
}

void DlgExportStoryboard::slotPageSettingsChanged(int)
{
    setUsableMaximums(pageSize(), pageOrientation(), exportLayout());
}
