/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Halla Rempt <halla@valdyas.org>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_palette_view.h"

#include <QWheelEvent>
#include <QHeaderView>
#include <QFormLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QScopedValueRollback>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>

#include <kis_icon_utils.h>

#include <KisKineticScroller.h>

#include <KoDialog.h>
#include <KoColorDisplayRendererInterface.h>

#include "KisPaletteDelegate.h"
#include "KisPaletteModel.h"
#include "kis_color_button.h"
#include <KisSwatch.h>
#include <KisResourceModel.h>
#include <kis_debug.h>
#include <KisResourceUserOperations.h>

int KisPaletteView::MINIMUM_ROW_HEIGHT = 10;

struct KisPaletteView::Private
{
    QPointer<KisPaletteModel> model;
    bool foregroundColorChangeInProgress = false;
};

KisPaletteView::KisPaletteView(QWidget *parent)
    : QTableView(parent)
    , d(new Private)
{
    setItemDelegate(new KisPaletteDelegate(this));

    setShowGrid(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    setAcceptDrops(false);

    /*
     * without this, a cycle might be created:
     * the view stretches to right border, and this make it need a scroll bar;
     * after the bar is added, the view shrinks to the bar, and this makes it
     * no longer need the bar any more, and the bar is removed again
     */
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    // set the size of swatches
    horizontalHeader()->setVisible(false);
    verticalHeader()->setVisible(false);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    horizontalHeader()->setMinimumSectionSize(MINIMUM_ROW_HEIGHT);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setMinimumSectionSize(MINIMUM_ROW_HEIGHT);

    connect(horizontalHeader(), SIGNAL(sectionResized(int,int,int)),
            SLOT(slotHorizontalHeaderResized(int,int,int)));
    setAutoFillBackground(true);

    QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(this);
    if (scroller) {
        connect(scroller, SIGNAL(stateChanged(QScroller::State)),
                this, SLOT(slotScrollerStateChanged(QScroller::State)));
    }

    connect(this, SIGNAL(clicked(QModelIndex)), SLOT(slotCurrentSelectionChanged(QModelIndex)));
}

KisPaletteView::~KisPaletteView()
{
}

void KisPaletteView::setCrossedKeyword(const QString &value)
{
    KisPaletteDelegate *delegate =
        dynamic_cast<KisPaletteDelegate*>(itemDelegate());
    KIS_ASSERT_RECOVER_RETURN(delegate);

    delegate->setCrossedKeyword(value);
}

bool KisPaletteView::addEntryWithDialog(KoColor color)
{
    KoDialog dialog;
    dialog.setWindowTitle(i18nc("@title:dialog", "Add a new Color Swatch"));
    QFormLayout *editableItems = new QFormLayout(dialog.mainWidget());

    QComboBox *cmbGroups = new QComboBox;
    QString defaultGroupName = i18nc("Name for default swatch group", "Default");
    cmbGroups->addItem(defaultGroupName);
    cmbGroups->addItems(d->model->colorSet()->swatchGroupNames());
    QLineEdit *lnIDName = new QLineEdit;
    QLineEdit *lnName = new QLineEdit;
    KisColorButton *bnColor = new KisColorButton;
    QCheckBox *chkSpot = new QCheckBox;
    chkSpot->setToolTip(i18nc("@info:tooltip", "A spot color is a color that the printer is able to print without mixing the paints it has available to it. The opposite is called a process color."));
    editableItems->addRow(i18n("Swatch Group:"), cmbGroups);
    editableItems->addRow(i18n("Swatch ID:"), lnIDName);
    editableItems->addRow(i18n("Color swatch name:"), lnName);
    editableItems->addRow(i18nc("Color as the Color of a Swatch in a Palette", "Color:"), bnColor);
    editableItems->addRow(i18n("Spot color:"), chkSpot);
    cmbGroups->setCurrentIndex(0);
    lnName->setText(i18nc("Prefix of a color swatch default name, as in Color 1","Color")+" " + QString::number(d->model->colorSet()->colorCount()+1));
    lnIDName->setText(QString::number(d->model->colorSet()->colorCount() + 1));
    bnColor->setColor(color);
    chkSpot->setChecked(false);

    if (dialog.exec() == KoDialog::Accepted) {
        QString groupName = cmbGroups->currentText();
        if (groupName == defaultGroupName) {
            groupName = QString();
        }
        KisSwatch newEntry;
        newEntry.setColor(bnColor->color());
        newEntry.setName(lnName->text());
        newEntry.setId(lnIDName->text());
        newEntry.setSpotColor(chkSpot->isChecked());
        d->model->addSwatch(newEntry, groupName);
        saveModification();
        return true;
    }

    return false;
}

bool KisPaletteView::addGroupWithDialog()
{
    KoDialog dialog;
    dialog.setWindowTitle(i18nc("@title:dialog","Add a new group"));
    QFormLayout *editableItems = new QFormLayout(dialog.mainWidget());
    QLineEdit *lnName = new QLineEdit();
    lnName->setText(i18nc("Part of default name for a new group", "Color Group")+""+QString::number(d->model->colorSet()->swatchGroupNames().size()+1));
    editableItems->addRow(i18nc("Name for a group", "Name"), lnName);

    if (dialog.exec() == KoDialog::Accepted) {
        d->model->addGroup(lnName->text());
        saveModification();
        return true;
    }
    return false;
}

void KisPaletteView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    if(!d->foregroundColorChangeInProgress) {
        QTableView::scrollTo(index, hint);
    }
}

bool KisPaletteView::removeEntryWithDialog(QModelIndex index)
{
    bool keepColors = false;
    if (qvariant_cast<bool>(index.data(KisPaletteModel::IsGroupNameRole))) {
        KoDialog dialog;
        dialog.setWindowTitle(i18nc("@title:dialog","Removing Group"));
        QFormLayout *editableItems = new QFormLayout(dialog.mainWidget());
        QCheckBox *chkKeep = new QCheckBox();
        editableItems->addRow(i18nc("Shows up when deleting a swatch group", "Keep the Colors"), chkKeep);

        if (dialog.exec() != KoDialog::Accepted) { return false; }
        keepColors = chkKeep->isChecked();
    }
    d->model->removeSwatch(index, keepColors);

    saveModification();

    return true;
}

void KisPaletteView::selectClosestColor(const KoColor &color)
{
    KoColorSetSP colorSet = d->model->colorSet();
    if (!colorSet || !colorSet->valid() || currentIndex().row() < 0) {
        return;
    }

    //also don't select if the color is the same as the current selection
    if (d->model->getSwatch(currentIndex()).color() == color) {
        return;
    }

    selectionModel()->clearSelection();
    QModelIndex index = d->model->indexForClosest(color);

    selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
}

const KoColor KisPaletteView::closestColor(const KoColor &color) const
{
    QModelIndex index = d->model->indexForClosest(color);
    KisSwatch swatch = d->model->getSwatch(index);
    return swatch.color();
}

void KisPaletteView::slotFGColorChanged(const KoColor &color)
{
    QScopedValueRollback<bool> rollback(d->foregroundColorChangeInProgress, true);
    selectClosestColor(color);
}

void KisPaletteView::setPaletteModel(KisPaletteModel *model)
{
    if (d->model) {
        disconnect(d->model, 0, this, 0);
    }
    d->model = model;
    setModel(model);
    slotAdditionalGuiUpdate();

    connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(slotAdditionalGuiUpdate()));
    connect(model, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), SLOT(slotAdditionalGuiUpdate()));
    connect(model, SIGNAL(modelReset()), SLOT(slotAdditionalGuiUpdate()));
}

KisPaletteModel* KisPaletteView::paletteModel() const
{
    return d->model;
}

void KisPaletteView::setAllowModification(bool allow)
{
    setAcceptDrops(allow);
}

void KisPaletteView::slotHorizontalHeaderResized(int, int, int newSize)
{
    resizeRows(newSize);
    slotAdditionalGuiUpdate();
}

void KisPaletteView::resizeRows(int newSize)
{
    verticalHeader()->setDefaultSectionSize(newSize);
    verticalHeader()->resizeSections(QHeaderView::Fixed);
}

void KisPaletteView::saveModification()
{
    KisResourceUserOperations::updateResourceWithUserInput(this, d->model->colorSet());
}

void KisPaletteView::removeSelectedEntry()
{
    if (selectedIndexes().size() <= 0) {
        return;
    }
    d->model->removeSwatch(currentIndex());
}

void KisPaletteView::slotAdditionalGuiUpdate()
{
    /*
     * Note: QTableView (Qt 5.15) does not clear spans on model resets.
     * But it does move spans on row inserts/removals, so incremental updates
     * would be possible.
     * Moving rows on the other hand does NOT update row spans accordingly...
     */
    if (!d->model->colorSet()) return;

    clearSpans();
    resizeRows(verticalHeader()->defaultSectionSize());

//    int row = -1;

    for (const QString &groupName : d->model->colorSet()->swatchGroupNames()) {
        if (groupName.isEmpty()) continue;

//        KisSwatchGroupSP group = d->model->colorSet()->getGroup(groupName);
//        row += group->rowCount() + 1;
//        setSpan(row, 0, 1, d->model->columnCount());
//        setRowHeight(row, fontMetrics().lineSpacing() + 6);
//        verticalHeader()->resizeSection(row, fontMetrics().lineSpacing() + 6);


        int rowNumber = d->model->colorSet()->startRowForGroup(groupName);
        setSpan(rowNumber, 0, 1, d->model->columnCount());
        setRowHeight(rowNumber, fontMetrics().lineSpacing() + 6);
        verticalHeader()->resizeSection(rowNumber, fontMetrics().lineSpacing() + 6);
    }
}

void KisPaletteView::slotCurrentSelectionChanged(const QModelIndex &newCurrent)
{
    if (!newCurrent.isValid()) { return; }

    const bool isGroupName = newCurrent.data(KisPaletteModel::IsGroupNameRole).toBool();
    const bool isCheckSlot = newCurrent.data(KisPaletteModel::CheckSlotRole).toBool();

    const KisSwatch newEntry = d->model->getSwatch(newCurrent);

    Q_EMIT sigIndexSelected(newCurrent);
    if (isGroupName) {
        return;
    }

    if (isCheckSlot) {
        Q_EMIT sigColorSelected(newEntry.color());
    }
}

void KisPaletteView::setDisplayRenderer(const KoColorDisplayRendererInterface *displayRenderer)
{
    Q_ASSERT(d->model);
    d->model->setDisplayRenderer(displayRenderer);
}

