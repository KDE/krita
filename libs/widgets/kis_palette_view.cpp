/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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
    setDragDropMode(QAbstractItemView::InternalMove);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(false);
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
    QScopedPointer<KoDialog> window(new KoDialog(this));
    window->setWindowTitle(i18nc("@title:window", "Add a new Colorset Entry"));
    QFormLayout *editableItems = new QFormLayout(window.data());
    window->mainWidget()->setLayout(editableItems);
    QComboBox *cmbGroups = new QComboBox(window.data());
    QString defaultGroupName = i18nc("Name for default group", "Default");
    cmbGroups->addItem(defaultGroupName);
    cmbGroups->addItems(m_d->model->colorSet()->getGroupNames());
    QLineEdit *lnIDName = new QLineEdit(window.data());
    QLineEdit *lnName = new QLineEdit(window.data());
    KisColorButton *bnColor = new KisColorButton(window.data());
    QCheckBox *chkSpot = new QCheckBox(window.data());
    chkSpot->setToolTip(i18nc("@info:tooltip", "A spot color is a color that the printer is able to print without mixing the paints it has available to it. The opposite is called a process color."));
    editableItems->addRow(i18n("Group"), cmbGroups);
    editableItems->addRow(i18n("ID"), lnIDName);
    editableItems->addRow(i18n("Name"), lnName);
    editableItems->addRow(i18n("Color"), bnColor);
    editableItems->addRow(i18nc("Spot color", "Spot"), chkSpot);
    cmbGroups->setCurrentIndex(0);
    lnName->setText(i18nc("Part of a default name for a color","Color")+" " + QString::number(m_d->model->colorSet()->colorCount()+1));
    lnIDName->setText(QString::number(m_d->model->colorSet()->colorCount() + 1));
    bnColor->setColor(color);
    chkSpot->setChecked(false);

    if (window->exec() == KoDialog::Accepted) {
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
        return true;
    }

    return false;
}

bool KisPaletteView::addGroupWithDialog()
{
    KoDialog *window = new KoDialog();
    window->setWindowTitle(i18nc("@title:window","Add a new group"));
    QFormLayout *editableItems = new QFormLayout();
    window->mainWidget()->setLayout(editableItems);
    QLineEdit *lnName = new QLineEdit();
    editableItems->addRow(i18nc("Name for a group", "Name"), lnName);
    lnName->setText(i18nc("Part of default name for a new group", "Color Group")+""+QString::number(m_d->model->colorSet()->getGroupNames().size()+1));
    if (window->exec() == KoDialog::Accepted) {
        KisSwatchGroup group;
        group.setName(lnName->text());
        m_d->model->addGroup(group);
        m_d->model->colorSet()->save();
        return true;
    }
    return false;
}

bool KisPaletteView::removeEntryWithDialog(QModelIndex index)
{
    bool keepColors = false;
    if (qvariant_cast<bool>(index.data(KisPaletteModel::IsGroupNameRole))) {
        QScopedPointer<KoDialog> window(new KoDialog(this));
        window->setWindowTitle(i18nc("@title:window","Removing Group"));
        QFormLayout *editableItems = new QFormLayout(window.data());
        QCheckBox *chkKeep = new QCheckBox(window.data());
        window->mainWidget()->setLayout(editableItems);
        editableItems->addRow(i18nc("Shows up when deleting a swatch group", "Keep the Colors"), chkKeep);
        if (window->exec() != KoDialog::Accepted) { return false; }
        keepColors = chkKeep->isChecked();
    }
    m_d->model->removeEntry(index, keepColors);
    if (m_d->model->colorSet()->isGlobal()) {
        m_d->model->colorSet()->save();
    }
    return true;
}

void KisPaletteView::selectClosestColor(const KoColor &color)
{
    KoColorSet* color_set = m_d->model->colorSet();
    if (!color_set) {
        return;
    }
    //also don't select if the color is the same as the current selection
    if (m_d->model->getEntry(currentIndex()).color() == color) {
        return;
    }

    selectionModel()->clearSelection();
    QModelIndex index = m_d->model->indexForClosest(color);
    blockSignals(true);
    selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
    blockSignals(false;)
}

void KisPaletteView::slotFGColorChanged(const KoColor &color)
{
    KConfigGroup group(KSharedConfig::openConfig(), "");
    if (group.readEntry("colorsettings/forcepalettecolors", false)) {
        selectClosestColor(color);
    }
}

void KisPaletteView::slotFGColorResourceChanged(const KoColor& color)
{
    // This slot is called, because fg color was changed in the resource manager.
    // To enable re-picking the swatch color again, we reset currentIndex
    // of the selectionModel. We are not clearing the selection itself,
    // so the user can see the swatch selected previously.
    // See bug 402072
    selectionModel()->clearCurrentIndex();
    slotFGColorChanged(color);
}

void KisPaletteView::slotSelectColor(const KoColor &color)
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
    setDragEnabled(allow);
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
