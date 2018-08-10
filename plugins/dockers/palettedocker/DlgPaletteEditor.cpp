#include <QAction>
#include <QFileInfo>
#include <QSpinBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QComboBox>
#include <QInputDialog>
#include <QFormLayout>
#include <QPicture>
#include <QPushButton>

#include <KoResourceServerProvider.h>
#include <KoResourceServer.h>
#include <KoDialog.h>
#include <KoColorSet.h>
#include <kis_global.h>
#include <KisPaletteModel.h>

#include "PaletteEditor.h"

#include <ui_WdgDlgPaletteEditor.h>

#include "DlgPaletteEditor.h"

DlgPaletteEditor::DlgPaletteEditor()
    : m_ui(new Ui_WdgDlgPaletteEditor)
    , m_actAddGroup(new QAction(i18n("Add a group")))
    , m_actDelGroup(new QAction(i18n("Delete this group")))
    , m_actRenGroup(new QAction(i18n("Rename this group")))
    , m_paletteEditor(new PaletteEditor(this))
    , m_currentGroupOriginalName(KoColorSet::GLOBAL_GROUP_NAME)
{
    m_ui->setupUi(this);
    m_ui->gbxPalette->setTitle(i18n("Palette settings"));
    m_ui->labelFilename->setText(i18n("Filename"));
    m_ui->labelName->setText(i18n("Palette Name"));
    m_ui->bnAddGroup->setDefaultAction(m_actAddGroup.data());

    m_ui->gbxGroup->setTitle(i18n("Group settings"));
    m_ui->labelColCount->setText(i18n("Column count"));
    m_ui->labelRowCount->setText(i18n("Row count"));
    m_ui->bnDelGroup->setDefaultAction(m_actDelGroup.data());
    m_ui->bnRenGroup->setDefaultAction(m_actRenGroup.data());

    connect(m_actAddGroup.data(), SIGNAL(triggered(bool)), SLOT(slotAddGroup()));
    connect(m_actDelGroup.data(), SIGNAL(triggered(bool)), SLOT(slotDelGroup()));
    connect(m_actRenGroup.data(), SIGNAL(triggered(bool)), SLOT(slotRenGroup()));
    connect(m_ui->spinBoxRow, SIGNAL(valueChanged(int)), SLOT(slotRowCountChanged(int)));
    connect(m_ui->spinBoxCol, SIGNAL(valueChanged(int)), SLOT(slotColCountChanged(int)));
    connect(m_ui->lineEditName, SIGNAL(editingFinished()), SLOT(slotNameChanged()));
    connect(m_ui->lineEditFilename, SIGNAL(textEdited(QString)), SLOT(slotFilenameChanged(QString)));
    connect(m_ui->lineEditFilename, SIGNAL(editingFinished()), SLOT(slotFilenameInputFinished()));
    connect(m_ui->ckxGlobal, SIGNAL(stateChanged(int)), SLOT(slotSetGlobal(int)));
    connect(m_ui->ckxReadOnly, SIGNAL(stateChanged(int)), SLOT(slotSetReadOnly(int)));

    m_warnPalette.setColor(QPalette::Text, Qt::red);
}

DlgPaletteEditor::~DlgPaletteEditor()
{ }

void DlgPaletteEditor::setPaletteModel(KisPaletteModel *model)
{
    m_colorSet = model->colorSet();
    if (m_colorSet.isNull()) {
        return;
    }
    m_paletteEditor->setPaletteModel(model);

    m_ui->lineEditName->setText(m_colorSet->name());
    m_ui->lineEditFilename->setText(m_colorSet->filename());
    m_ui->spinBoxCol->setValue(m_colorSet->columnCount());
    m_ui->ckxGlobal->setCheckState(m_colorSet->isGlobal() ? Qt::Checked : Qt::Unchecked);
    m_ui->ckxReadOnly->setCheckState(!m_colorSet->isEditable() ? Qt::Checked : Qt::Unchecked);

    Q_FOREACH (const QString & groupName, m_colorSet->getGroupNames()) {
        m_ui->cbxGroup->addItem(groupName);
    }

    connect(m_ui->cbxGroup, SIGNAL(currentTextChanged(QString)), SLOT(slotGroupChosen(QString)));
    m_ui->cbxGroup->setCurrentText(KoColorSet::GLOBAL_GROUP_NAME);
    m_ui->bnDelGroup->setEnabled(false);
    m_ui->bnRenGroup->setEnabled(false);

    m_ui->spinBoxRow->setValue(m_paletteEditor->rowNumberOfGroup(KoColorSet::GLOBAL_GROUP_NAME));

    bool canWrite = m_colorSet->isEditable();
    m_ui->lineEditName->setEnabled(canWrite);
    m_ui->lineEditFilename->setEnabled(canWrite);
    m_ui->spinBoxCol->setEnabled(canWrite);
    m_ui->spinBoxRow->setEnabled(canWrite);
    m_ui->ckxGlobal->setEnabled(canWrite);
    m_ui->ckxReadOnly->setEnabled(canWrite);
    m_ui->bnAddGroup->setEnabled(canWrite);
}

void DlgPaletteEditor::setView(KisViewManager *view)
{
    m_paletteEditor->setView(view);
}

void DlgPaletteEditor::slotAddGroup()
{
    QString newGroupName = m_paletteEditor->addGroup();
    if (!newGroupName.isEmpty()) {
        m_ui->cbxGroup->addItem(newGroupName);
        m_ui->cbxGroup->setCurrentIndex(m_ui->cbxGroup->count() - 1);
    };
}

void DlgPaletteEditor::slotRenGroup()
{
    QString newName = m_paletteEditor->renameGroup(m_currentGroupOriginalName);
    if (!newName.isEmpty()) {
        int idx = m_ui->cbxGroup->currentIndex();
        m_ui->cbxGroup->removeItem(idx);
        m_ui->cbxGroup->insertItem(idx, newName);
        m_ui->cbxGroup->setCurrentIndex(idx);
    }
}

void DlgPaletteEditor::slotDelGroup()
{
    int deletedIdx = m_ui->cbxGroup->currentIndex();
    if (m_paletteEditor->removeGroup(m_currentGroupOriginalName)) {
        m_ui->cbxGroup->setCurrentIndex(0);
        m_ui->cbxGroup->removeItem(deletedIdx);
    }
}

void DlgPaletteEditor::slotGroupChosen(const QString &groupName)
{
    if (groupName == KoColorSet::GLOBAL_GROUP_NAME) {
        m_ui->bnDelGroup->setEnabled(false);
        m_ui->bnRenGroup->setEnabled(false);
    } else {
        m_ui->bnDelGroup->setEnabled(true);
        m_ui->bnRenGroup->setEnabled(true);
    }
    m_currentGroupOriginalName = m_paletteEditor->oldNameFromNewName(groupName);
    m_ui->spinBoxRow->setValue(m_paletteEditor->rowNumberOfGroup(m_currentGroupOriginalName));
}

void DlgPaletteEditor::uploadChange()
{
    m_paletteEditor->updatePalette();
}

void DlgPaletteEditor::slotRowCountChanged(int newCount)
{
    m_paletteEditor->changeGroupRowCount(m_currentGroupOriginalName, newCount);
}

void DlgPaletteEditor::slotSetGlobal(int state)
{
    m_paletteEditor->setGlobal(state == Qt::Checked);
}

void DlgPaletteEditor::slotSetReadOnly(int state)
{
    m_paletteEditor->setReadOnly(state == Qt::Checked);
}

void DlgPaletteEditor::slotNameChanged()
{
    m_paletteEditor->rename(qobject_cast<QLineEdit*>(sender())->text());
}

void DlgPaletteEditor::slotFilenameChanged(const QString &newFilename)
{
    if (m_paletteEditor->duplicateExistsFilename(newFilename)) {
        m_ui->lineEditFilename->setPalette(m_warnPalette);
        m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        return;
    }
    m_ui->lineEditFilename->setPalette(m_normalPalette);
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    m_paletteEditor->changeFilename(newFilename);
}

void DlgPaletteEditor::slotFilenameInputFinished()
{
    QString newName = m_ui->lineEditFilename->text();
    if (m_paletteEditor->duplicateExistsFilename(newName)) {
        return;
    }
    m_paletteEditor->changeFilename(newName);
}

void DlgPaletteEditor::slotColCountChanged(int newCount)
{
    m_paletteEditor->changeColCount(newCount);
}
