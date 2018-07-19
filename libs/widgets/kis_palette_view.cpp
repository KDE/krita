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
#include <QtMath> // qFloor

#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>
#include <kis_icon_utils.h>
#include <KoDialog.h>

#include "KisPaletteDelegate.h"
#include "KisPaletteModel.h"
#include "kis_color_button.h"
#include <KisSwatch.h>

int KisPaletteView::MINROWHEIGHT = 10;

struct KisPaletteView::Private
{
    Private()
    {
        entryClickedMenu.addAction("Add foreground color");
        entryClickedMenu.addAction("Choose a color to add");
        entryClickedMenu.addAction("Rename this spot");
        entryClickedMenu.addAction("Switch with another spot");
        entryClickedMenu.addAction("Delete color");
    }
    QPointer<KisPaletteModel> model;
    QMenu entryClickedMenu;
    bool allowPaletteModification;
    QAction actAdd;
    QAction actAddWithDlg;
    QAction actModify;
    QAction actSwitch;
    QAction actRemove;
};

KisPaletteView::KisPaletteView(QWidget *parent)
    : QTableView(parent)
    , m_d(new Private)
{
    m_d->allowPaletteModification = true;

    setItemDelegate(new KisPaletteDelegate(this));


    setShowGrid(true);
    setDropIndicatorShown(true);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setSelectionMode(QAbstractItemView::SingleSelection);

    /*
     * without this, a cycle might be created:
     * the view streches to right border, and this make it need a scroll bar;
     * after the bar is added, the view shrinks to the bar, and this makes it
     * no longer need the bar any more, and the bar is removed again
     */
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // set the size of swatches
    horizontalHeader()->setVisible(false);
    verticalHeader()->setVisible(false);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    horizontalHeader()->setMinimumSectionSize(MINROWHEIGHT);
    horizontalHeader()->setMaximumSectionSize(MINROWHEIGHT);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setMinimumSectionSize(MINROWHEIGHT);
    verticalHeader()->setDefaultSectionSize(MINROWHEIGHT);

    connect(horizontalHeader(), SIGNAL(sectionResized(int,int,int)), SLOT(slotResizeVerticalHeader(int,int,int)));
    connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotModifyEntry(QModelIndex)));

    KConfigGroup cfg(KSharedConfig::openConfig()->group(""));
    QPalette pal(palette());
    // pal.setColor(QPalette::Base, cfg.getMDIBackgroundColor());
    setAutoFillBackground(true);
    setPalette(pal);

}

KisPaletteView::~KisPaletteView()
{
}

void KisPaletteView::setCrossedKeyword(const QString &value)
{
    /*
    KisPaletteDelegate *delegate =
        dynamic_cast<KisPaletteDelegate*>(itemDelegate());
    KIS_ASSERT_RECOVER_RETURN(delegate);

    delegate->setCrossedKeyword(value);
    */
}

bool KisPaletteView::addEntryWithDialog(KoColor color)
{
    KoDialog *window = new KoDialog(this);
    window->setWindowTitle(i18nc("@title:window", "Add a new Colorset Entry"));
    QFormLayout *editableItems = new QFormLayout(window);
    window->mainWidget()->setLayout(editableItems);
    QComboBox *cmbGroups = new QComboBox(window);
    QString defaultGroupName = i18nc("Name for default group", "Default");
    cmbGroups->addItem(defaultGroupName);
    cmbGroups->addItems(m_d->model->colorSet()->getGroupNames());
    QLineEdit *lnIDName = new QLineEdit(window);
    QLineEdit *lnName = new QLineEdit(window);
    KisColorButton *bnColor = new KisColorButton(window);
    QCheckBox *chkSpot = new QCheckBox(window);
    chkSpot->setToolTip(i18nc("@info:tooltip", "A spot color is a color that the printer is able to print without mixing the paints it has available to it. The opposite is called a process color."));
    editableItems->addRow(i18n("Group"), cmbGroups);
    editableItems->addRow(i18n("ID"), lnIDName);
    editableItems->addRow(i18n("Name"), lnName);
    editableItems->addRow(i18n("Color"), bnColor);
    editableItems->addRow(i18nc("Spot color", "Spot"), chkSpot);
    cmbGroups->setCurrentIndex(0);
    lnName->setText(i18nc("Part of a default name for a color","Color")+" "+QString::number(m_d->model->colorSet()->colorCount()+1));
    lnIDName->setText(QString::number(m_d->model->colorSet()->colorCount()+1));
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

bool KisPaletteView::removeEntryWithDialog(QModelIndex index)
{
    bool keepColors = true;
    if (qvariant_cast<bool>(index.data(KisPaletteModel::IsHeaderRole))) {
        KoDialog *window = new KoDialog();
        window->setWindowTitle(i18nc("@title:window","Removing Group"));
        QFormLayout *editableItems = new QFormLayout();
        QCheckBox *chkKeep = new QCheckBox();
        window->mainWidget()->setLayout(editableItems);
        editableItems->addRow(i18nc("Shows up when deleting a group","Keep the Colors"), chkKeep);
        chkKeep->setChecked(keepColors);
        if (window->exec() == KoDialog::Accepted) {
            keepColors = chkKeep->isChecked();
            m_d->model->removeEntry(index, keepColors);
            m_d->model->colorSet()->save();
        }
    } else {
        m_d->model->removeEntry(index, keepColors);
        m_d->model->colorSet()->save();
    }
    return true;
}

void KisPaletteView::trySelectClosestColor(KoColor color)
{
    /*
    KoColorSet* color_set = m_d->model->colorSet();
    if (!color_set)
        return;
    //also don't select if the color is the same as the current selection
    if (selectedIndexes().size()>0) {
        QModelIndex currentI = currentIndex();
        if (!currentI.isValid()) {
            currentI = selectedIndexes().last();
        }
        if (!currentI.isValid()) {
            currentI = selectedIndexes().first();
        }
        if (currentI.isValid()) {
            if (m_d->model->colorSetEntryFromIndex(currentI).color()==color) {
                return;
            }
        }
    }
    quint32 i = color_set->getIndexClosestColor(color);
    QModelIndex index = m_d->model->indexFromId(i);
    this->selectionModel()->clearSelection();
    this->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
    */
}

void KisPaletteView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        m_d->entryClickedMenu.exec(QCursor::pos());
        return;
    }

    if (selectedIndexes().size() <= 0) {
        return;
    }

    QModelIndex index = currentIndex();

    if (qvariant_cast<bool>(index.data(KisPaletteModel::IsHeaderRole)) == false) {
        bool slotEmpty = !(qvariant_cast<bool>(index.data(KisPaletteModel::CheckSlotRole)));
        if (slotEmpty) {
            emit sigSetEntry(index);
            update(index);
        } else {
            KisSwatchGroup *group = static_cast<KisSwatchGroup*>(index.internalPointer());
            Q_ASSERT(group);
            KisSwatch entry = group->getEntry(index.column(), index.row());
            emit sigEntrySelected(entry);
        }
    }

}

void KisPaletteView::setPaletteModel(KisPaletteModel *model)
{
    if (m_d->model) {
        disconnect(m_d->model, 0, this, 0);
    }
    m_d->model = model;
    setModel(model);
}

KisPaletteModel* KisPaletteView::paletteModel() const
{
    return m_d->model;
}

void KisPaletteView::setAllowModification(bool allow)
{
    m_d->allowPaletteModification = allow;
}

void KisPaletteView::wheelEvent(QWheelEvent *event)
{
    QTableView::wheelEvent(event);
    /*
    if (event->modifiers() & Qt::ControlModifier) {
        int numDegrees = event->delta() / 8;
        int numSteps = numDegrees / 7;
        int curSize = horizontalHeader()->sectionSize(0);
        int setSize = numSteps + curSize;

       if ( (event->delta() <= 0) && (setSize <= 8) ) {
           // Ignore scroll-zooming down below a certain size
       } else {
           horizontalHeader()->setDefaultSectionSize(setSize);
           verticalHeader()->setDefaultSectionSize(setSize);
           KConfigGroup cfg(KSharedConfig::openConfig()->group(""));
           cfg.writeEntry("paletteDockerPaletteViewSectionSize", setSize);
       }

        event->accept();
    } else {
        QTableView::wheelEvent(event);
    }
    */
}

void KisPaletteView::modifyEntry(QModelIndex index)
{
    if (m_d->allowPaletteModification) {
        KoDialog *dlg = new KoDialog();
        QFormLayout *editableItems = new QFormLayout(dlg);
        dlg->mainWidget()->setLayout(editableItems);
        QLineEdit *lnIDName = new QLineEdit(dlg);
        QLineEdit *lnGroupName = new QLineEdit(dlg);
        KisColorButton *bnColor = new KisColorButton(dlg);
        QCheckBox *chkSpot = new QCheckBox(dlg);

        if (qvariant_cast<bool>(index.data(KisPaletteModel::IsHeaderRole))) {
            QString groupName = qvariant_cast<QString>(index.data(Qt::DisplayRole));
            editableItems->addRow(i18nc("Name for a colorgroup","Name"), lnGroupName);
            lnGroupName->setText(groupName);
            if (dlg->exec() == KoDialog::Accepted) {
                m_d->model->colorSet()->changeGroupName(groupName, lnGroupName->text());
                m_d->model->colorSet()->save();
            }
            //rename the group.
        } else {
            KisSwatchGroup *group = static_cast<KisSwatchGroup*>(index.internalPointer());
            Q_ASSERT(group);
            KisSwatch entry = group->getEntry(index.column(), index.row());
            chkSpot->setToolTip(i18nc("@info:tooltip", "A spot color is a color that the printer is able to print without mixing the paints it has available to it. The opposite is called a process color."));
            editableItems->addRow(i18n("ID"), lnIDName);
            editableItems->addRow(i18n("Name"), lnGroupName);
            editableItems->addRow(i18n("Color"), bnColor);
            editableItems->addRow(i18n("Spot"), chkSpot);
            lnGroupName->setText(entry.name());
            lnIDName->setText(entry.id());
            bnColor->setColor(entry.color());
            chkSpot->setChecked(entry.spotColor());
            if (dlg->exec() == KoDialog::Accepted) {
                entry.setName(lnGroupName->text());
                entry.setId(lnIDName->text());
                entry.setColor(bnColor->color());
                entry.setSpotColor(chkSpot->isChecked());
                m_d->model->colorSet()->setEntry(entry, index.column(), index.row());
                m_d->model->colorSet()->save();
            }
        }

        delete dlg;
    }
    update(index);
}

void KisPaletteView::slotModifyEntry(const QModelIndex &index)
{
    modifyEntry(index);
}

void KisPaletteView::slotResizeVerticalHeader(int, int, int newSize)
{
    verticalHeader()->setDefaultSectionSize(newSize);
    verticalHeader()->setMaximumSectionSize(newSize);
    verticalHeader()->resizeSections(QHeaderView::Fixed);
}

void KisPaletteView::removeSelectedEntry()
{
    if (selectedIndexes().size() <= 0) {
        return;
    }
    m_d->model->removeEntry(currentIndex());
}
