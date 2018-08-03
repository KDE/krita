#include <QAction>
#include <QFileInfo>
#include <QSpinBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QMessageBox>

#include <KoDialog.h>
#include <KoColorSet.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>

#include <ui_WdgDlgPaletteEditor.h>

#include "KisDlgPaletteEditor.h"

KisDlgPaletteEditor::KisDlgPaletteEditor()
    : m_ui(new Ui_WdgDlgPaletteEditor)
    , m_actAddGroup(new QAction(i18n("Add a group")))
    , m_actDelGroup(new QAction(i18n("Delete this group")))
    , m_group(Q_NULLPTR)
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

    connect(m_actAddGroup.data(), SIGNAL(triggered(bool)), SLOT(slotAddGroup()));
    connect(m_actDelGroup.data(), SIGNAL(triggered(bool)), SLOT(slotDelGroup()));
}

KisDlgPaletteEditor::~KisDlgPaletteEditor()
{ }

void KisDlgPaletteEditor::setPalette(KoColorSet *colorSet)
{
    m_colorSet = colorSet;
    if (m_colorSet.isNull()) {
        return;
    }
    if (!m_original.isNull()) {
        delete m_original.data();
    }
    m_original.reset(new OriginalPaletteInfo);
    m_ui->lineEditName->setText(m_colorSet->name());
    m_original->name = m_colorSet->name();
    m_ui->lineEditFilename->setText(m_colorSet->filename());
    m_original->filename = m_colorSet->filename();
    m_ui->spinBoxCol->setValue(m_colorSet->columnCount());
    m_original->columnCount = m_colorSet->columnCount();
    m_ui->ckxGlobal->setCheckState(m_colorSet->isGlobal() ? Qt::Checked : Qt::Unchecked);
    m_original->isGlobal = m_colorSet->isGlobal();
    m_ui->ckxReadOnly->setCheckState(!m_colorSet->isEditable() ? Qt::Checked : Qt::Unchecked);
    m_original->isReadOnly = !m_colorSet->isEditable();

    for (const QString & groupName : m_colorSet->getGroupNames()) {
        KisSwatchGroup *group = m_colorSet->getGroup(groupName);
        m_groups[groupName] = GroupInfoType(group->name(), group->rowCount());
        m_original->groups[groupName] = GroupInfoType(group->name(), group->rowCount());
        m_ui->cbxGroup->addItem(groupName);
    }
    m_group = m_colorSet->getGlobalGroup();
    m_ui->cbxGroup->setCurrentText(KoColorSet::GLOBAL_GROUP_NAME);
    connect(m_ui->cbxGroup, SIGNAL(currentTextChanged(QString)), SLOT(slotGroupChosen(QString)));

    m_ui->spinBoxRow->setValue(m_group->rowCount());

    if (!m_colorSet->isEditable()) {
        m_ui->lineEditName->setEnabled(false);
        m_ui->lineEditFilename->setEnabled(false);
        m_ui->spinBoxCol->setEnabled(false);
        m_ui->spinBoxRow->setEnabled(false);
        m_ui->ckxGlobal->setEnabled(false);
        m_ui->ckxReadOnly->setEnabled(false);
        m_ui->bnAddGroup->setEnabled(false);
        m_ui->bnDelGroup->setEnabled(false);
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
    layout.addWidget(&spxRow);
    if (dlg.exec() != QDialog::Accepted) { return; }
    if (m_colorSet->getGroup(leName.text())) {
        QMessageBox msgNameDuplicate;
        msgNameDuplicate.setText(i18n("Group already exists"));
        msgNameDuplicate.setWindowTitle(i18n("Group already exists! Group not added."));
        msgNameDuplicate.exec();
        return;
    }
    m_colorSet->addGroup(leName.text());
    m_colorSet->getGroup(leName.text())->setRowCount(spxRow.value());
    m_groups.insert(leName.text(), GroupInfoType(leName.text(), spxRow.value()));

    qDebug() << "Adding a group";
}

void KisDlgPaletteEditor::slotDelGroup()
{
    qDebug() << "Deleting current group";
}

void KisDlgPaletteEditor::slotGroupChosen(const QString &groupName)
{
    m_group = m_colorSet->getGroup(groupName);
}
