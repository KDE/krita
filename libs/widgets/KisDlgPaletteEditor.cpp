#include <ui_WdgDlgPaletteEditor.h>

#include "KisDlgPaletteEditor.h"

KisDlgPaletteEditor::KisDlgPaletteEditor()
    : m_ui(new Ui_WdgDlgPaletteEditor)
{
    m_ui->setupUi(this);
    m_ui->labelFilename->setText(i18n("Filename"));
    m_ui->labelName->setText(i18n("Name"));
    m_ui->verticalLayoutLeft->setAlignment(Qt::AlignTop);
}

KisDlgPaletteEditor::~KisDlgPaletteEditor()
{ }

void KisDlgPaletteEditor::setPalette(KoColorSet *colorSet)
{
    m_colorSet = colorSet;
    if (!m_colorSet.isNull()) {
        m_ui->lineEditName->setText(m_colorSet->name());
        m_ui->lineEditFilename->setText(m_colorSet->filename());
        m_ui->spinBoxCol->setValue(m_colorSet->columnCount());
        m_ui->spinBoxRow->setValue(m_colorSet->rowCount());
    }
}

QString KisDlgPaletteEditor::name() const
{
    return m_ui->lineEditName->text();
}

QString KisDlgPaletteEditor::filename() const
{
    return m_ui->lineEditFilename->text();
}

int KisDlgPaletteEditor::columnCount() const
{
    return m_ui->spinBoxCol->value();
}

int KisDlgPaletteEditor::rowCount() const
{
    return m_ui->spinBoxRow->value();
}
