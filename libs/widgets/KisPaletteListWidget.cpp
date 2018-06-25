#include <QPointer>
#include <QScopedPointer>
#include <QGridLayout>

#include <kis_icon.h>

#include <ui_WdgPaletteListWidget.h>
#include "KisPaletteListWidget.h"
#include "KisPaletteListWidget_p.h"

KisPaletteListWidget::KisPaletteListWidget(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui_WdgPaletteListWidget)
    , m_d(new Private(this))
{
    m_ui->setupUi(this);
    m_ui->bnAdd->setIcon(KisIconUtils::loadIcon("list-add"));
    m_ui->bnRemove->setIcon(KisIconUtils::loadIcon("list-remove"));
    m_ui->bnEdit->setIcon(KisIconUtils::loadIcon("edit-rename"));
    m_ui->bnImport->setIcon(KisIconUtils::loadIcon("document-import"));
    m_ui->bnExport->setIcon(KisIconUtils::loadIcon("document-export"));
    m_ui->tableViewPalette->setItemDelegate(m_d->delegate.data());
    m_ui->tableViewPalette->setModel(m_d->model.data());
}

KisPaletteListWidget::~KisPaletteListWidget()
{ }
