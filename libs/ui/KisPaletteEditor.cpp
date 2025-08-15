/*
 *  SPDX-FileCopyrightText: 2018 Michael Zhou <simeirxh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QHash>
#include <QString>
#include <QPointer>
#include <QFormLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QMessageBox>

#include <KoDialog.h>
#include <KoFileDialog.h>
#include <KoColorSet.h>
#include <KisSwatchGroup.h>
#include <kis_signal_compressor.h>
#include <KisViewManager.h>
#include <KisDocument.h>
#include <KoResourceServer.h>
#include <KisStorageModel.h>
#include <KoResourceServerProvider.h>
#include <KisPaletteModel.h>
#include <kis_color_button.h>

#include "KisPaletteEditor.h"
#include <KisGlobalResourcesInterface.h>
#include <KisResourceUserOperations.h>

struct KisPaletteEditor::PaletteInfo {
    QString name;
    QString filename;
    int columnCount;
    QString storageLocation;
    QHash<QString, KisSwatchGroupSP> groups;
};

struct KisPaletteEditor::Private
{
    bool isEditing {false};
    QSet<QString> modifiedGroupNames; // key is original group name
    QSet<QString> newGroupNames;
    QSet<QString> keepColorGroups;
    QSet<QString> pathsToRemove;
    QString groupBeingRenamed;
    QPointer<KisPaletteModel> model;
    QPointer<KisViewManager> view;
    KisPaletteEditor::PaletteInfo modifiedPaletteInfo;
    QPointer<KoDialog> query;
    KoResourceServer<KoColorSet> *rServer {0};

    QPalette normalPalette;
    QPalette warnPalette;

};

KisPaletteEditor::KisPaletteEditor(QObject *parent)
    : QObject(parent)
    , m_d(new Private)
{
    m_d->rServer = KoResourceServerProvider::instance()->paletteServer();
    m_d->warnPalette.setColor(QPalette::Text, Qt::red);
}

KisPaletteEditor::~KisPaletteEditor()
{ }

void KisPaletteEditor::setPaletteModel(KisPaletteModel *model)
{
    if (!model) { return; }
    if (m_d->model) {
        m_d->model->disconnect(this);
    }
    m_d->model = model;
    slotPaletteChanged();
    connect(model, SIGNAL(sigPaletteChanged()), SLOT(slotPaletteChanged()));
    connect(model, SIGNAL(sigPaletteModified()), SLOT(slotSetDocumentModified()));
}

void KisPaletteEditor::setView(KisViewManager *view)
{
    m_d->view = view;
}

KoColorSetSP KisPaletteEditor::addPalette()
{
    if (!m_d->view) { return 0; }
    if (!m_d->view->document()) { return 0; }

    KoColorSetSP colorSet(new KoColorSet());

    KoDialog dialog;
    QFormLayout *layout = new QFormLayout(dialog.mainWidget());
    QLineEdit *le = new QLineEdit(i18nc("Default name for a new palette","New Palette"));
    layout->addRow(i18n("New palette name:"), le);

    QComboBox *cmbPaletteType = new QComboBox();
    cmbPaletteType->addItem(i18nc("Palette File Type", "Krita Palette (KPL)"));
    cmbPaletteType->addItem(i18nc("Palette File Type", "GIMP Palette (GPL)"));
    layout->addRow(cmbPaletteType);

    QCheckBox *chkSaveInDocument = new QCheckBox(i18n("Save Palette in the Current Document"));
    chkSaveInDocument->setChecked(false);
    layout->addRow(chkSaveInDocument);


    if (dialog.exec() != QDialog::Accepted) { return 0; }

    QString name = le->text();
    if (cmbPaletteType->currentIndex() == 0) {
        colorSet->setPaletteType(KoColorSet::KPL);
    }
    else {
        colorSet->setPaletteType(KoColorSet::GPL);
    }
    colorSet->setValid(true);
    colorSet->setName(name);
    colorSet->setFilename(name.split(" ").join("_")+colorSet->defaultFileExtension());

    QString resourceLocation = "";
    if (chkSaveInDocument->isChecked()) {
        resourceLocation = m_d->view->document()->linkedResourcesStorageId();
    }

    if (KisResourceUserOperations::addResourceWithUserInput(m_d->view->mainWindowAsQWidget(), colorSet, resourceLocation)) {
        return colorSet;

    }

    return 0;
}

KoColorSetSP KisPaletteEditor::importPalette()
{
    KoFileDialog dialog(nullptr, KoFileDialog::OpenFile, "Open Palette");
    dialog.setCaption(i18n("Import Palette"));

    dialog.setDefaultDir(QDir::homePath());
    dialog.setMimeTypeFilters(QStringList() << "application/x-krita-palette" << "application/x-gimp-color-palette");

    QString filename = dialog.filename();
    if (filename.isEmpty()) {
        return nullptr;
    }

    QMessageBox messageBox;
    messageBox.setText(i18n("Do you want to store this palette in your current image?"));
    messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    QString storageLocation = "";
    if (messageBox.exec() == QMessageBox::Yes) {
        storageLocation = m_d->view->document()->linkedResourcesStorageId();
    }
    KoResourceSP resource = KisResourceUserOperations::importResourceFileWithUserInput(m_d->view->mainWindowAsQWidget(), storageLocation, ResourceType::Palettes, filename);

    KoColorSetSP palette;
    if (resource) {
        palette = resource.dynamicCast<KoColorSet>();
    }

    return palette;
}

void KisPaletteEditor::removePalette(KoColorSetSP cs)
{
    if (!m_d->view) { return; }
    if (!m_d->view->document()) { return; }
    if (!cs) { return; }
    m_d->rServer->removeResourceFromServer(cs);
}

int KisPaletteEditor::rowCountOfGroup(const QString &originalName) const
{
    if (!m_d->modifiedPaletteInfo.groups.contains(originalName)) {
        KoColorSetSP colorSet = m_d->model->colorSet();
        if (originalName.isEmpty()) {
            return colorSet->getGlobalGroup()->rowCount();
        }
        else {
            KisSwatchGroupSP group = colorSet->getGroup(originalName);
            if (group ){
                return group->rowCount();
            }
        }
        return 0;
    }
    return m_d->modifiedPaletteInfo.groups[originalName]->rowCount();
}

bool KisPaletteEditor::duplicateExistsGroupName(const QString &name) const
{
    if (name == m_d->groupBeingRenamed) { return false; }
    Q_FOREACH (const KisSwatchGroupSP group, m_d->modifiedPaletteInfo.groups.values()) {
        if (name == group->name()) { return true; }
    }
    return false;
}

bool KisPaletteEditor::duplicateExistsOriginalGroupName(const QString &name) const
{
    return m_d->modifiedPaletteInfo.groups.contains(name);
}

QString KisPaletteEditor::oldNameFromNewName(const QString &newName) const
{
    Q_FOREACH (const QString &oldGroupName, m_d->modifiedPaletteInfo.groups.keys()) {
        if (m_d->modifiedPaletteInfo.groups[oldGroupName]->name() == newName) {
            return oldGroupName;
        }
    }
    return QString();
}

void KisPaletteEditor::rename(const QString &newName)
{
    if (!m_d->isEditing) {
        return;
    }
    if (newName.isEmpty()) { return; }
    m_d->modifiedPaletteInfo.name = newName;
}

void KisPaletteEditor::changeColumnCount(int newCount)
{
    if (!m_d->isEditing) {
        return;
    }
    m_d->modifiedPaletteInfo.columnCount = newCount;
}

QString KisPaletteEditor::addGroup()
{
    if (!m_d->isEditing) {
        return QString();
    }
    KoDialog dialog;
    m_d->query = &dialog;

    QVBoxLayout *layout = new QVBoxLayout(dialog.mainWidget());

    layout->addWidget(new QLabel(i18n("New swatch group name:")));
    QLineEdit *leName = new QLineEdit(newGroupName());
    connect(leName, SIGNAL(textChanged(QString)), SLOT(slotGroupNameChanged(QString)));
    layout->addWidget(leName);
    layout->addWidget(new QLabel(i18n("Rows of swatches in group:")));
    QSpinBox *spxRow = new QSpinBox();
    spxRow->setValue(20);
    layout->addWidget(spxRow);

    if (dialog.exec() != QDialog::Accepted) { return QString(); }
    if (duplicateExistsGroupName(leName->text())) { return QString(); }

    QString realName = leName->text();
    QString name = realName;
    if (duplicateExistsOriginalGroupName(name)) {
        name = newGroupName();
    }
    m_d->modifiedPaletteInfo.groups[name].reset(new KisSwatchGroup);
    KisSwatchGroupSP newGroup = m_d->modifiedPaletteInfo.groups[name];
    newGroup->setName(realName);
    m_d->newGroupNames.insert(name);
    newGroup->setRowCount(spxRow->value());
    return realName;
}

bool KisPaletteEditor::removeGroup(const QString &name)
{
    if (!m_d->isEditing) {
        return false;
    }
    KoDialog dialog;
    dialog.setWindowTitle(i18nc("@title:dialog", "Removing Swatch Group"));
    QFormLayout *editableItems = new QFormLayout(dialog.mainWidget());
    QCheckBox *chkKeep = new QCheckBox();

    editableItems->addRow(i18nc("Shows up when deleting a swatch group", "Keep the Colors"), chkKeep);
    if (dialog.exec() != KoDialog::Accepted) { return false; }

    m_d->modifiedPaletteInfo.groups.remove(name);
    m_d->newGroupNames.remove(name);
    if (chkKeep->isChecked()) {
        m_d->keepColorGroups.insert(name);
    }
    return true;
}

QString KisPaletteEditor::renameGroup(const QString &oldName)
{
    if (!m_d->isEditing) {
        return QString();
    }
    if (oldName.isEmpty() || oldName == KoColorSet::GLOBAL_GROUP_NAME) { return QString(); }

    KoDialog dialog;
    m_d->query = &dialog;
    m_d->groupBeingRenamed = m_d->modifiedPaletteInfo.groups[oldName]->name();

    QFormLayout *form = new QFormLayout(dialog.mainWidget());

    QLineEdit *leNewName = new QLineEdit();
    connect(leNewName, SIGNAL(textChanged(QString)), SLOT(slotGroupNameChanged(QString)));
    leNewName->setText(m_d->modifiedPaletteInfo.groups[oldName]->name());

    form->addRow(i18n("New swatch group name:"), leNewName);

    if (dialog.exec() != KoDialog::Accepted) { return QString(); }
    if (leNewName->text().isEmpty()) { return QString(); }
    if (duplicateExistsGroupName(leNewName->text())) { return QString(); }

    m_d->modifiedPaletteInfo.groups[oldName]->setName(leNewName->text());
    m_d->modifiedGroupNames.insert(oldName);

    return leNewName->text();
}

void KisPaletteEditor::slotGroupNameChanged(const QString &newName)
{
    QLineEdit *leGroupName = qobject_cast<QLineEdit*>(sender());
    if (duplicateExistsGroupName(newName) || newName == QString()) {
        leGroupName->setPalette(m_d->warnPalette);
        if (m_d->query->button(KoDialog::Ok)) {
            m_d->query->button(KoDialog::Ok)->setEnabled(false);
        }
        return;
    }
    leGroupName->setPalette(m_d->normalPalette);
    if (m_d->query->button(KoDialog::Ok)) {
        m_d->query->button(KoDialog::Ok)->setEnabled(true);
    }
}

void KisPaletteEditor::changeGroupRowCount(const QString &name, int newRowCount)
{
    if (!m_d->isEditing) {
        return;
    }
    if (!m_d->modifiedPaletteInfo.groups.contains(name)) { return; }
    m_d->modifiedPaletteInfo.groups[name]->setRowCount(newRowCount);
    m_d->modifiedGroupNames.insert(name);
}

void KisPaletteEditor::setStorageLocation(QString location)
{
   m_d->modifiedPaletteInfo.storageLocation = location;
}

void KisPaletteEditor::setEntry(const KoColor &color, const QModelIndex &index)
{
    Q_ASSERT(m_d->model);
    if (!m_d->view) { return; }
    if (!m_d->view->document()) { return; }
    KisSwatch c = KisSwatch(color);
    c.setId(QString::number(m_d->model->colorSet()->colorCount() + 1));
    c.setName(i18nc("Default name for a color swatch","Color %1", QString::number(m_d->model->colorSet()->colorCount()+1)));
    m_d->model->setSwatch(c, index);
}

void KisPaletteEditor::slotSetDocumentModified()
{
    if (m_d->modifiedPaletteInfo.storageLocation == m_d->view->document()->linkedResourcesStorageId()) {
        KisResourceUserOperations::updateResourceWithUserInput(m_d->view->mainWindowAsQWidget(), m_d->model->colorSet());
        m_d->view->document()->setModified(true);
    }
    m_d->model->colorSet()->setDirty(true);
}

void KisPaletteEditor::removeEntry(const QModelIndex &index)
{
    Q_ASSERT(m_d->model);
    if (!m_d->view) { return; }
    if (!m_d->view->document()) { return; }
    if (qvariant_cast<bool>(index.data(KisPaletteModel::IsGroupNameRole))) {
        startEditing();
        removeGroup(qvariant_cast<QString>(index.data(KisPaletteModel::GroupNameRole)));
        endEditing();
    } else {
        m_d->model->removeSwatch(index, false);
    }
}

void KisPaletteEditor::modifyEntry(const QModelIndex &index)
{
    if (!m_d->view) { return; }
    if (!m_d->view->document()) { return; }

    KoDialog dialog;
    dialog.setCaption(i18nc("@title:dialog", "Add a new Color Swatch"));
    QFormLayout *editableItems = new QFormLayout(dialog.mainWidget());

    QString groupName = qvariant_cast<QString>(index.data(Qt::DisplayRole));
    if (qvariant_cast<bool>(index.data(KisPaletteModel::IsGroupNameRole))) {
        startEditing();
        renameGroup(groupName);
        endEditing();
    }
    else {

        QLineEdit *lnIDName = new QLineEdit();
        QLineEdit *lnGroupName = new QLineEdit();
        KisColorButton *bnColor = new KisColorButton();
        QCheckBox *chkSpot = new QCheckBox();
        chkSpot->setToolTip(i18nc("@info:tooltip", "A spot color is a color that the printer is able to print without mixing the paints it has available to it. The opposite is called a process color."));

        KisSwatch entry = m_d->model->getSwatch(index);

        editableItems->addRow(i18n("Swatch ID:"), lnIDName);
        editableItems->addRow(i18n("Color swatch name:"), lnGroupName);
        editableItems->addRow(i18nc("Color as the Color of a Swatch in a Palette", "Color:"), bnColor);
        editableItems->addRow(i18n("Spot color:"), chkSpot);

        lnGroupName->setText(entry.name());
        lnIDName->setText(entry.id());
        bnColor->setColor(entry.color());
        chkSpot->setChecked(entry.spotColor());

        if (dialog.exec() == KoDialog::Accepted) {
            entry.setName(lnGroupName->text());
            entry.setId(lnIDName->text());
            entry.setColor(bnColor->color());
            entry.setSpotColor(chkSpot->isChecked());
            m_d->model->setSwatch(entry, index);
        }
    }
}

void KisPaletteEditor::addEntry(const KoColor &color)
{
    Q_ASSERT(m_d->model);
    if (!m_d->view) { return; }
    if (!m_d->view->document()) { return; }

    KoDialog dialog;
    dialog.setWindowTitle(i18nc("@title:dialog", "Add a new Color Swatch"));

    QFormLayout *editableItems = new QFormLayout(dialog.mainWidget());

    QComboBox *cmbGroups = new QComboBox();
    cmbGroups->addItems(m_d->model->colorSet()->swatchGroupNames());
    cmbGroups->setCurrentIndex(0);

    QLineEdit *lnIDName = new QLineEdit();
    lnIDName->setText(QString::number(m_d->model->colorSet()->colorCount() + 1));

    QLineEdit *lnName = new QLineEdit();
    lnName->setText(i18nc("Default name for a color swatch","Color %1", QString::number(m_d->model->colorSet()->colorCount()+1)));

    KisColorButton *bnColor = new KisColorButton();
    bnColor->setColor(color);

    QCheckBox *chkSpot = new QCheckBox();
    chkSpot->setChecked(false);
    chkSpot->setToolTip(i18nc("@info:tooltip", "A spot color is a color that the printer is able to print without mixing the paints it has available to it. The opposite is called a process color."));

    editableItems->addRow(i18n("Swatch Group:"), cmbGroups);
    editableItems->addRow(i18n("Swatch ID:"), lnIDName);
    editableItems->addRow(i18n("Color swatch name:"), lnName);
    editableItems->addRow(i18nc("Color as the Color of a Swatch in a Palette", "Color:"), bnColor);
    editableItems->addRow(i18n("Spot color:"), chkSpot);

    if (dialog.exec() != KoDialog::Accepted) { return; }

    QString groupName = cmbGroups->currentText();

    KisSwatch newSwatch;
    newSwatch.setColor(bnColor->color());
    newSwatch.setName(lnName->text());
    newSwatch.setId(lnIDName->text());
    newSwatch.setSpotColor(chkSpot->isChecked());
    m_d->model->addSwatch(newSwatch, groupName);
    m_d->modifiedGroupNames.insert(groupName);
    // TODO: This is getting called when startEditing is never called. This avoids the crash, but what is its purpose?
    if (m_d->isEditing) {
        KisSwatchGroupSP cs = m_d->modifiedPaletteInfo.groups.value(groupName);
        cs->addSwatch(newSwatch);
        if (!m_d->modifiedPaletteInfo.groups.contains(groupName)) {
            m_d->modifiedPaletteInfo.groups.insert(groupName, cs);
        }
    }
}

bool KisPaletteEditor::isModified() const
{
    if (m_d->model->colorSet()) {
        return m_d->model->colorSet()->isDirty();
    } else {
        return false;
    }
}

void KisPaletteEditor::startEditing()
{
    if (m_d->isEditing) {
        qWarning() << "KisPaletteEditor::startEditing(): restarting palette editing.";
        clearStagedChanges();
    }

    if (!m_d->model || !m_d->model->colorSet()) {
        return;
    }
    KoColorSetSP palette = m_d->model->colorSet();

    m_d->modifiedPaletteInfo.name = palette->name();
    m_d->modifiedPaletteInfo.storageLocation = palette->storageLocation();
    m_d->modifiedPaletteInfo.columnCount = palette->columnCount();
    m_d->modifiedPaletteInfo.groups.clear();

    Q_FOREACH (const QString &groupName, palette->swatchGroupNames()) {
        KisSwatchGroupSP cs(new KisSwatchGroup(*palette->getGroup(groupName).data()));
        m_d->modifiedPaletteInfo.groups.insert(groupName, cs);
    }

    m_d->isEditing = true;
}

void KisPaletteEditor::endEditing(bool applyChanges)
{
    if (!m_d->isEditing) {
        qWarning("KisPaletteEditor::endEditing(): not in editing state; ignoring.");
        return;
    }

    if (!applyChanges) {
        clearStagedChanges();
        return;
    }

    if (!m_d->model) return;
    if (!m_d->model->colorSet()) return;
    if (!m_d->view) return;
    if (!m_d->view->document()) return;

    KoColorSetSP palette = m_d->model->colorSet();
    PaletteInfo &modified = m_d->modifiedPaletteInfo;

    if (modified.columnCount != palette->columnCount()) {
        m_d->model->setColumnCount(modified.columnCount);
    }
    if (modified.name != palette->name()) {
        KisResourceUserOperations::renameResourceWithUserInput(m_d->view->mainWindowAsQWidget(), palette, m_d->modifiedPaletteInfo.name);
    }
    QString resourceLocation = m_d->model->colorSet()->storageLocation();
    if (resourceLocation != m_d->modifiedPaletteInfo.storageLocation) {
        // We need functionality for moving the resource to the new resource storage...
    }

    Q_FOREACH (const QString &groupName, palette->swatchGroupNames()) {
        if (!modified.groups.contains(groupName)) {
            m_d->model->removeGroup(groupName, m_d->keepColorGroups.contains(groupName));
        }
    }
    Q_FOREACH (const QString &groupName, palette->swatchGroupNames()) {
        if (m_d->modifiedGroupNames.contains(groupName)) {
            m_d->model->setRowCountForGroup(groupName, modified.groups[groupName]->rowCount());
            if (groupName != modified.groups[groupName]->name()) {
                m_d->model->changeGroupName(groupName, modified.groups[groupName]->name());
                modified.groups[modified.groups[groupName]->name()] = modified.groups[groupName];
                modified.groups.remove(groupName);
            }
        }
    }
    Q_FOREACH (const QString &newGroupName, m_d->newGroupNames) {
        m_d->model->addGroup(newGroupName, palette->columnCount(), m_d->modifiedPaletteInfo.groups[newGroupName]->rowCount());
    }

    clearStagedChanges();
}

void KisPaletteEditor::clearStagedChanges()
{
    m_d->modifiedPaletteInfo.groups.clear();
    m_d->keepColorGroups.clear();
    m_d->newGroupNames.clear();
    m_d->modifiedGroupNames.clear();
    m_d->isEditing = false;
}

void KisPaletteEditor::saveNewPaletteVersion()
{
    if (!m_d->model || !m_d->model->colorSet()) { return; }

    QModelIndex index = m_d->rServer->resourceModel()->indexForResource(m_d->model->colorSet());
    bool isGlobal = false;
    if (index.isValid()) {
        bool ok = false;
        int storageId = m_d->rServer->resourceModel()->data(index, Qt::UserRole + KisAllResourcesModel::StorageId).toInt(&ok);
        if (ok) {
            KisStorageModel storageModel;
            KisResourceStorageSP storage = storageModel.storageForId(storageId);
            isGlobal = storage->type() != KisResourceStorage::StorageType::Memory;
        }
    }
    bool res = false;
    if (isGlobal) {
        if (m_d->view) {
            res = KisResourceUserOperations::updateResourceWithUserInput(m_d->view->mainWindowAsQWidget(), m_d->model->colorSet());
        } else if (m_d->model->colorSet()->version() >= 0) {
            //if the version is -1, then the resource should not be updated, because it was never saved to begin with...
            res = m_d->rServer->resourceModel()->updateResource(m_d->model->colorSet());
            dbgResources << Q_FUNC_INFO << "-- Updating resource without user input: " << m_d->model->colorSet()->name() << "Result:" << res;
        }
    }
}

void KisPaletteEditor::slotPaletteChanged()
{
    Q_ASSERT(m_d->model);
    clearStagedChanges();
}

QString KisPaletteEditor::newGroupName() const
{
    int i = 1;
    QString groupname = i18nc("Default new group name", "New Group %1", QString::number(i));
    while (m_d->modifiedPaletteInfo.groups.contains(groupname)) {
        i++;
        groupname = i18nc("Default new group name", "New Group %1", QString::number(i));
    }
    return groupname;
}

QString KisPaletteEditor::filenameFromPath(const QString &path) const
{
    return QDir::fromNativeSeparators(path).section('/', -1, -1);
}
