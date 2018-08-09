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

#include <KoDialog.h>
#include <KoColorSet.h>

#include <ui_WdgDlgPaletteEditor.h>

#include "KisDlgPaletteEditor.h"

KisDlgPaletteEditor::KisDlgPaletteEditor()
    : m_ui(new Ui_WdgDlgPaletteEditor)
    , m_actAddGroup(new QAction(i18n("Add a group")))
    , m_actDelGroup(new QAction(i18n("Delete this group")))
    , m_actRenGroup(new QAction(i18n("Rename this group")))
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
}

KisDlgPaletteEditor::~KisDlgPaletteEditor()
{ }

void KisDlgPaletteEditor::setPalette(KoColorSet *colorSet)
{
    m_colorSet = colorSet;
    if (m_colorSet.isNull()) {
        return;
    }
    m_original.reset(new OriginalPaletteInfo);
    m_ui->lineEditName->setText(m_colorSet->name());
    m_ui->lineEditFilename->setText(m_colorSet->filename());
    m_ui->spinBoxCol->setValue(m_colorSet->columnCount());
    m_ui->ckxGlobal->setCheckState(m_colorSet->isGlobal() ? Qt::Checked : Qt::Unchecked);
    m_ui->ckxReadOnly->setCheckState(!m_colorSet->isEditable() ? Qt::Checked : Qt::Unchecked);
    m_original->name = m_colorSet->name();
    m_original->filename = m_colorSet->filename();
    m_original->columnCount = m_colorSet->columnCount();
    m_original->isGlobal = m_colorSet->isGlobal();
    m_original->isReadOnly = !m_colorSet->isEditable();

    Q_FOREACH (const QString & groupName, m_colorSet->getGroupNames()) {
        KisSwatchGroup *group = m_colorSet->getGroup(groupName);
        m_groups[groupName] = GroupInfo(groupName, group->rowCount());
        m_original->groups[groupName] = GroupInfo(groupName, group->rowCount());
        m_ui->cbxGroup->addItem(groupName);
    }
    connect(m_ui->cbxGroup, SIGNAL(currentTextChanged(QString)), SLOT(slotGroupChosen(QString)));
    m_ui->cbxGroup->setCurrentText(KoColorSet::GLOBAL_GROUP_NAME);
    m_ui->bnDelGroup->setEnabled(false);
    m_ui->bnRenGroup->setEnabled(false);

    m_ui->spinBoxRow->setValue(m_groups[KoColorSet::GLOBAL_GROUP_NAME].rowNumber);

    bool canWrite = m_colorSet->isEditable();
    m_ui->lineEditName->setEnabled(canWrite);
    m_ui->lineEditFilename->setEnabled(canWrite);
    m_ui->spinBoxCol->setEnabled(canWrite);
    m_ui->spinBoxRow->setEnabled(canWrite);
    m_ui->ckxGlobal->setEnabled(canWrite);
    m_ui->ckxReadOnly->setEnabled(canWrite);
    m_ui->bnAddGroup->setEnabled(canWrite);
}

QString KisDlgPaletteEditor::name() const
{
    return m_ui->lineEditName->text();
}

QString KisDlgPaletteEditor::filename() const
{
    return m_ui->lineEditFilename->text();
}

const QSet<QString> &KisDlgPaletteEditor::newGroupNames() const
{
    return m_newGroups;
}

int KisDlgPaletteEditor::columnCount() const
{
    return m_ui->spinBoxCol->value();
}

bool KisDlgPaletteEditor::isGlobal() const
{
    return m_ui->ckxGlobal->checkState() == Qt::Checked;
}

bool KisDlgPaletteEditor::isReadOnly() const
{
    return m_ui->ckxReadOnly->checkState() == Qt::Checked;
}

bool KisDlgPaletteEditor::isModified() const
{
    Q_ASSERT(!m_original.isNull());
    return m_original->isReadOnly != isReadOnly() ||
            m_original->isGlobal != isGlobal() ||
            m_original->name != name() ||
            m_original->filename != filename() ||
            m_original->columnCount != columnCount() ||
            m_original->groups != m_groups;
}

bool KisDlgPaletteEditor::groupRemoved(const QString &groupName) const
{
    if (groupName == KoColorSet::GLOBAL_GROUP_NAME) { return false; }
    return m_original->groups.contains(groupName) && !m_groups.contains(groupName);
}

QString KisDlgPaletteEditor::groupRenamedTo(const QString &oriGroupName) const
{
    if (!m_groups.contains(oriGroupName) || m_groups[oriGroupName].newName == oriGroupName) {
        return QString();
    }
    return m_groups[oriGroupName].newName;
}

void KisDlgPaletteEditor::slotAddGroup()
{
    KoDialog dlg;
    QVBoxLayout layout(&dlg);
    dlg.mainWidget()->setLayout(&layout);
    QLabel lblName(i18n("Name"), &dlg);
    layout.addWidget(&lblName);
    QLineEdit leName(&dlg);
    layout.addWidget(&leName);
    QLabel lblRowCount(i18n("Row count"), &dlg);
    layout.addWidget(&lblRowCount);
    QSpinBox spxRow(&dlg);
    spxRow.setValue(20);
    layout.addWidget(&spxRow);
    if (dlg.exec() != QDialog::Accepted) { return; }
    if (m_colorSet->getGroup(leName.text())) {
        QMessageBox msgNameDuplicate;
        msgNameDuplicate.setText(i18n("Group already exists"));
        msgNameDuplicate.setWindowTitle(i18n("Group already exists! Group not added."));
        msgNameDuplicate.exec();
        return;
    }
    m_groups.insert(leName.text(), GroupInfo(leName.text(), spxRow.value()));
    m_newGroups.insert(leName.text());
    m_ui->cbxGroup->addItem(leName.text());
    m_ui->cbxGroup->setCurrentIndex(m_ui->cbxGroup->count() - 1);
}

void KisDlgPaletteEditor::slotRenGroup()
{
    KoDialog dlg;
    QFormLayout form(&dlg);
    dlg.mainWidget()->setLayout(&form);
    QLineEdit leNewName;
    leNewName.setText(m_groups[m_currentGroupOriginalName].newName);
    form.addRow(i18nc("Renaming swatch group", "New name"), &leNewName);
    if (dlg.exec() != KoDialog::Accepted) { return; }
    if (leNewName.text().isEmpty()) { return; }
    if (m_groups.contains(leNewName.text())) { return; }
    m_groups[m_currentGroupOriginalName].newName = leNewName.text();
    if (m_newGroups.remove(m_currentGroupOriginalName)) {
        m_newGroups.insert(leNewName.text());
    }
    int idx = m_ui->cbxGroup->currentIndex();
    m_ui->cbxGroup->removeItem(idx);
    m_ui->cbxGroup->insertItem(idx, leNewName.text());
    m_ui->cbxGroup->setCurrentIndex(idx);
}

void KisDlgPaletteEditor::slotDelGroup()
{
    if (m_currentGroupOriginalName == KoColorSet::GLOBAL_GROUP_NAME) {
        QMessageBox msgNameDuplicate;
        msgNameDuplicate.setText(i18n("Can't delete group"));
        msgNameDuplicate.setWindowTitle(i18n("Can't delete the main group of a palette."));
        msgNameDuplicate.exec();
        return;
    }

    KoDialog window(this);
    window.setWindowTitle(i18nc("@title:window", "Removing Group"));
    QFormLayout editableItems(&window);
    QCheckBox chkKeep(&window);
    window.mainWidget()->setLayout(&editableItems);
    editableItems.addRow(i18nc("Shows up when deleting a swatch group", "Keep the Colors"), &chkKeep);
    if (window.exec() != KoDialog::Accepted) { return; }
    if (chkKeep.isChecked()) {
        m_keepColorGroups.insert(m_currentGroupOriginalName);
    }

    QString deletedName = m_currentGroupOriginalName;
    m_ui->cbxGroup->setCurrentIndex(0);
    m_ui->cbxGroup->removeItem(m_ui->cbxGroup->findText(m_groups[deletedName].newName));
    m_groups.remove(deletedName);
    m_newGroups.remove(deletedName);
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
    m_currentGroupOriginalName = oldNameFromNewName(groupName);
    m_ui->spinBoxRow->setValue(m_groups[m_currentGroupOriginalName].rowNumber);
}

int KisDlgPaletteEditor::groupRowNumber(const QString &groupName) const
{
    return m_groups[groupName].rowNumber;
}

bool KisDlgPaletteEditor::groupKeepColors(const QString &groupName) const
{
    return m_keepColorGroups.contains(groupName);
}

void KisDlgPaletteEditor::slotRowCountChanged(int newCount)
{
    m_groups[m_currentGroupOriginalName].rowNumber = newCount;
}

QString KisDlgPaletteEditor::oldNameFromNewName(const QString &newName) const
{
    Q_FOREACH (const QString &oldGroupName, m_groups.keys()) {
        if (m_groups[oldGroupName].newName == newName) {
            return oldGroupName;
        }
    }
    return QString();
}

bool operator ==(const KisDlgPaletteEditor::GroupInfo &lhs, const KisDlgPaletteEditor::GroupInfo &rhs)
{
    return lhs.newName == rhs.newName && lhs.rowNumber == rhs.rowNumber;
}
