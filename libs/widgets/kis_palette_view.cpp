/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_palette_view.h"

#include <QWheelEvent>
#include <QHeaderView>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QMenu>

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

int KisPaletteView::MININUM_ROW_HEIGHT = 10;

struct KisPaletteView::Private
{
    QPointer<KisPaletteModel> model;
    bool allowPaletteModification {false}; // if modification is allowed from this widget
};

KisPaletteView::KisPaletteView(QWidget *parent)
    : QTableView(parent)
    , m_d(new Private)
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
    horizontalHeader()->setMinimumSectionSize(MININUM_ROW_HEIGHT);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setMinimumSectionSize(MININUM_ROW_HEIGHT);

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
    cmbGroups->addItems(m_d->model->colorSet()->getGroupNames());
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
    lnName->setText(i18nc("Prefix of a color swatch default name, as in Color 1","Color")+" " + QString::number(m_d->model->colorSet()->colorCount()+1));
    lnIDName->setText(QString::number(m_d->model->colorSet()->colorCount() + 1));
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
        m_d->model->addEntry(newEntry, groupName);
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
    lnName->setText(i18nc("Part of default name for a new group", "Color Group")+""+QString::number(m_d->model->colorSet()->getGroupNames().size()+1));
    editableItems->addRow(i18nc("Name for a group", "Name"), lnName);

    if (dialog.exec() == KoDialog::Accepted) {
        KisSwatchGroup group;
        group.setName(lnName->text());
        m_d->model->addGroup(group);
        saveModification();
        return true;
    }
    return false;
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
    m_d->model->removeEntry(index, keepColors);

    saveModification();

    return true;
}

void KisPaletteView::selectClosestColor(const KoColor &color)
{
    KoColorSetSP color_set = m_d->model->colorSet();
    if (!color_set) {
        return;
    }
    //also don't select if the color is the same as the current selection
    if (m_d->model->getEntry(currentIndex()).color() == color) {
        return;
    }

    selectionModel()->clearSelection();
    QModelIndex index = m_d->model->indexForClosest(color);

    selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
}

const KoColor KisPaletteView::closestColor(const KoColor &color) const
{
    QModelIndex index = m_d->model->indexForClosest(color);
    KisSwatch swatch = m_d->model->getEntry(index);
    return swatch.color();
}

void KisPaletteView::slotFGColorChanged(const KoColor &color)
{
    selectClosestColor(color);
}

void KisPaletteView::setPaletteModel(KisPaletteModel *model)
{
    if (m_d->model) {
        disconnect(m_d->model, 0, this, 0);
    }
    m_d->model = model;
    setModel(model);
    slotAdditionalGuiUpdate();

    connect(model, SIGNAL(sigPaletteModified()), SLOT(slotAdditionalGuiUpdate()));
    connect(model, SIGNAL(sigPaletteChanged()), SLOT(slotAdditionalGuiUpdate()));
}

KisPaletteModel* KisPaletteView::paletteModel() const
{
    return m_d->model;
}

void KisPaletteView::setAllowModification(bool allow)
{
    m_d->allowPaletteModification = allow;
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
    //qDebug() << "saving modification in palette view" << m_d->model->colorSet()->filename() << m_d->model->colorSet()->storageLocation();
    KisResourceUserOperations::updateResourceWithUserInput(this, m_d->model->colorSet());
}

void KisPaletteView::removeSelectedEntry()
{
    if (selectedIndexes().size() <= 0) {
        return;
    }
    m_d->model->removeEntry(currentIndex());
}

void KisPaletteView::slotAdditionalGuiUpdate()
{
    clearSpans();
    resizeRows(verticalHeader()->defaultSectionSize());
    for (int groupNameRowNumber : m_d->model->m_rowGroupNameMap.keys()) {
        if (groupNameRowNumber == -1) { continue; }
        setSpan(groupNameRowNumber, 0, 1, m_d->model->columnCount());
        setRowHeight(groupNameRowNumber, fontMetrics().lineSpacing() + 6);
        verticalHeader()->resizeSection(groupNameRowNumber, fontMetrics().lineSpacing() + 6);
    }
}

void KisPaletteView::slotCurrentSelectionChanged(const QModelIndex &newCurrent)
{
    if (!newCurrent.isValid()) { return; }

    const bool isGroupName = newCurrent.data(KisPaletteModel::IsGroupNameRole).toBool();
    const bool isCheckSlot = newCurrent.data(KisPaletteModel::CheckSlotRole).toBool();

    const KisSwatch newEntry = m_d->model->getEntry(newCurrent);

    emit sigIndexSelected(newCurrent);
    if (isGroupName) {
        return;
    }

    if (isCheckSlot) {
        emit sigColorSelected(newEntry.color());
    }
}

void KisPaletteView::setDisplayRenderer(const KoColorDisplayRendererInterface *displayRenderer)
{
    Q_ASSERT(m_d->model);
    m_d->model->setDisplayRenderer(displayRenderer);
}
