/*
 *  Copyright (c) 2018 Michael Zhou <simeirxh@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

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
#include <QSignalBlocker>

#include <KoResourceServerProvider.h>
#include <KoResourceServer.h>
#include <KoDialog.h>
#include <KoColorSet.h>
#include <kis_global.h>
#include <KisPaletteModel.h>

#include "KisPaletteEditor.h"

#include "ui_WdgDlgPaletteEditor.h"

#include "KisDlgPaletteEditor.h"

KisDlgPaletteEditor::KisDlgPaletteEditor()
    : m_ui(new Ui_WdgDlgPaletteEditor)
    , m_actAddGroup(new QAction(i18n("Add a group")))
    , m_actDelGroup(new QAction(i18n("Delete this group")))
    , m_actRenGroup(new QAction(i18n("Rename this group")))
    , m_globalButtons(new QButtonGroup(this))
    , m_paletteEditor(new KisPaletteEditor(this))
    , m_currentGroupOriginalName(KoColorSet::GLOBAL_GROUP_NAME)
{
    setWindowTitle(i18n("Palette Editor"));

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

    m_globalButtons->addButton(m_ui->rb_global, 0);
    m_globalButtons->addButton(m_ui->rb_document, 1);
    connect(m_globalButtons.data(), SIGNAL(buttonClicked(int)), SLOT(slotSetGlobal()));


    connect(this, SIGNAL(accepted()), SLOT(slotAccepted()));

    m_warnPalette.setColor(QPalette::Text, Qt::red);
}

KisDlgPaletteEditor::~KisDlgPaletteEditor()
{ }

void KisDlgPaletteEditor::setPaletteModel(KisPaletteModel *model)
{
    m_colorSet = model->colorSet();
    if (m_colorSet.isNull()) {
        return;
    }
    m_paletteEditor->setPaletteModel(model);

    // don't let editor make changes when initializing
    const QSignalBlocker blocker1(m_ui->lineEditFilename);
    const QSignalBlocker blocker2(m_ui->lineEditName);
    const QSignalBlocker blocker3(m_ui->spinBoxCol);
    const QSignalBlocker blocker4(m_ui->spinBoxRow);
    const QSignalBlocker blocker5(m_ui->rb_global);
    const QSignalBlocker blocker6(m_ui->rb_document);
    const QSignalBlocker blocker7(m_ui->cbxGroup);

    m_ui->lineEditName->setText(m_colorSet->name());
    m_ui->lineEditFilename->setText(m_paletteEditor->relativePathFromSaveLocation());
    m_ui->spinBoxCol->setValue(m_colorSet->columnCount());
    if (m_colorSet->isGlobal()) {
        m_globalButtons->button(0)->setChecked(true);
    } else {
        m_globalButtons->button(1)->setChecked(true);
    }

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
    m_ui->rb_global->setEnabled(canWrite);
    m_ui->rb_document->setEnabled(canWrite);
    m_ui->bnAddGroup->setEnabled(canWrite);
}

void KisDlgPaletteEditor::setView(KisViewManager *view)
{
    m_paletteEditor->setView(view);
}

void KisDlgPaletteEditor::slotAddGroup()
{
    QString newGroupName = m_paletteEditor->addGroup();
    if (!newGroupName.isEmpty()) {
        m_ui->cbxGroup->addItem(newGroupName);
        m_ui->cbxGroup->setCurrentIndex(m_ui->cbxGroup->count() - 1);
    };
}

void KisDlgPaletteEditor::slotRenGroup()
{
    QString newName = m_paletteEditor->renameGroup(m_currentGroupOriginalName);
    if (!newName.isEmpty()) {
        int idx = m_ui->cbxGroup->currentIndex();
        m_ui->cbxGroup->removeItem(idx);
        m_ui->cbxGroup->insertItem(idx, newName);
        m_ui->cbxGroup->setCurrentIndex(idx);
    }
}

void KisDlgPaletteEditor::slotDelGroup()
{
    int deletedIdx = m_ui->cbxGroup->currentIndex();
    if (m_paletteEditor->removeGroup(m_currentGroupOriginalName)) {
        m_ui->cbxGroup->setCurrentIndex(0);
        m_ui->cbxGroup->removeItem(deletedIdx);
    }
}

void KisDlgPaletteEditor::slotGroupChosen(const QString &groupName)
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

void KisDlgPaletteEditor::slotRowCountChanged(int newCount)
{
    m_paletteEditor->changeGroupRowCount(m_currentGroupOriginalName, newCount);
}

void KisDlgPaletteEditor::slotSetGlobal()
{
    bool toGlobal = false;
    if (m_ui->rb_global->isChecked()) {
        toGlobal = true;
    }
    m_paletteEditor->setGlobal(toGlobal);
}

void KisDlgPaletteEditor::slotNameChanged()
{
    m_paletteEditor->rename(qobject_cast<QLineEdit*>(sender())->text());
}

void KisDlgPaletteEditor::slotFilenameChanged(const QString &newFilename)
{
    bool global = m_colorSet->isGlobal();
    if (m_paletteEditor->duplicateExistsFilename(newFilename, global)) {
        m_ui->lineEditFilename->setPalette(m_warnPalette);
        m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        return;
    }
    m_ui->lineEditFilename->setPalette(m_normalPalette);
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    m_paletteEditor->changeFilename(newFilename);
}

void KisDlgPaletteEditor::slotFilenameInputFinished()
{
    QString newName = m_ui->lineEditFilename->text();
    bool global = m_colorSet->isGlobal();
    if (m_paletteEditor->duplicateExistsFilename(newName, global)) {
        return;
    }
    m_paletteEditor->changeFilename(newName);
}

void KisDlgPaletteEditor::slotColCountChanged(int newCount)
{
    m_paletteEditor->changeColCount(newCount);
}

void KisDlgPaletteEditor::slotAccepted()
{
    m_paletteEditor->updatePalette();
}
