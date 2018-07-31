#include <ui_WdgDlgPaletteEditor.h>

#include "KisDlgPaletteEditor.h"

KisDlgPaletteEditor::KisDlgPaletteEditor()
    : m_ui(new Ui_WdgDlgPaletteEditor)
{
    m_ui->setupUi(this);
    m_ui->labelFilename->setText(i18n("Filename"));
    m_ui->labelName->setText(i18n("Name"));
}

KisDlgPaletteEditor::~KisDlgPaletteEditor()
{ }

void KisDlgPaletteEditor::setPalette(KoColorSet *colorSet)
{
    m_colorSet = colorSet;
}
