#include "kis_icon.h"
#include "KisPaletteChooser.h"
#include "ui_WdgPaletteChooser.h"

KisPaletteChooser::KisPaletteChooser(QWidget *parent)
    : QWidget(parent)
    , m_wdgPaletteChooser(new Ui_WdgPaletteChooser)
{
    m_wdgPaletteChooser->setupUi(this);
    m_wdgPaletteChooser->bnAdd->setIcon(KisIconUtils::loadIcon("list-add"));
    m_wdgPaletteChooser->bnRemove->setIcon(KisIconUtils::loadIcon("list-remove"));
    m_wdgPaletteChooser->bnEdit->setIcon(KisIconUtils::loadIcon("edit-rename"));
    m_wdgPaletteChooser->bnImport->setIcon(KisIconUtils::loadIcon("document-import"));
    m_wdgPaletteChooser->bnExport->setIcon(KisIconUtils::loadIcon("document-export"));
}
