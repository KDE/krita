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

#include "kis_palette_delegate.h"
#include "KisPaletteModel.h"
#include "kis_config.h"
#include <KLocalizedString>
#include <KoDialog.h>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <kis_color_button.h>
#include <QCheckBox>
#include <QComboBox>


struct KisPaletteView::Private
{
    KisPaletteModel *model = 0;
    bool allowPaletteModification = true;
};


KisPaletteView::KisPaletteView(QWidget *parent)
    : KoTableView(parent),
      m_d(new Private)
{
    setShowGrid(false);
    horizontalHeader()->setVisible(false);
    verticalHeader()->setVisible(false);
    setItemDelegate(new KisPaletteDelegate());

    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setDropIndicatorShown(true);

    KisConfig cfg;
    //QPalette pal(palette());
    //pal.setColor(QPalette::Base, cfg.getMDIBackgroundColor());
    //setAutoFillBackground(true);
    //setPalette(pal);

    int defaultSectionSize = cfg.paletteDockerPaletteViewSectionSize();
    horizontalHeader()->setDefaultSectionSize(defaultSectionSize);
    verticalHeader()->setDefaultSectionSize(defaultSectionSize);
    connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(modifyEntry(QModelIndex)));
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
    KoDialog *window = new KoDialog();
    window->setWindowTitle(i18nc("@title:window", "Add a new Colorset Entry"));
    QFormLayout *editableItems = new QFormLayout();
    window->mainWidget()->setLayout(editableItems);
    QComboBox *cmbGroups = new QComboBox();
    QString defaultGroupName = i18nc("Name for default group", "Default");
    cmbGroups->addItem(defaultGroupName);
    cmbGroups->addItems(m_d->model->colorSet()->getGroupNames());
    QLineEdit *lnIDName = new QLineEdit();
    QLineEdit *lnName = new QLineEdit();
    KisColorButton *bnColor = new KisColorButton();
    bnColor->setPaletteViewEnabled(false);
    QCheckBox *chkSpot = new QCheckBox();
    chkSpot->setToolTip(i18nc("@info:tooltip", "A spot color is a color that the printer is able to print without mixing the paints it has available to it. The opposite is called a process color."));
    editableItems->addRow(i18n("Group"), cmbGroups);
    editableItems->addRow(i18n("ID"), lnIDName);
    editableItems->addRow(i18n("Name"), lnName);
    editableItems->addRow(i18n("Color"), bnColor);
    editableItems->addRow(i18n("Spot"), chkSpot);
    cmbGroups->setCurrentIndex(0);
    lnName->setText(i18nc("Part of a default name for a color","Color")+" "+QString::number(m_d->model->colorSet()->nColors()+1));
    lnIDName->setText(QString::number(m_d->model->colorSet()->nColors()+1));
    bnColor->setColor(color);
    chkSpot->setChecked(false);

    //
    if (window->exec() == KoDialog::Accepted) {
        QString groupName = cmbGroups->currentText();
        if (groupName == defaultGroupName) {
            groupName = QString();
        }
        KoColorSetEntry newEntry;
        newEntry.color = bnColor->color();
        newEntry.name = lnName->text();
        newEntry.id = lnIDName->text();
        newEntry.spotColor = chkSpot->isChecked();
        m_d->model->addColorSetEntry(newEntry, groupName);
        m_d->model->colorSet()->save();
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
        QString groupName = lnName->text();
        m_d->model->addGroup(groupName);
        m_d->model->colorSet()->save();
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
            if (m_d->model->colorSetEntryFromIndex(currentI).color==color) {
                return;
            }
        }
    }
    quint32 i = color_set->getIndexClosestColor(color);
    QModelIndex index = m_d->model->indexFromId(i);
    this->selectionModel()->clearSelection();
    this->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
}

void KisPaletteView::mouseReleaseEvent(QMouseEvent *event)
{
    bool foreground = false;
    if (event->button()== Qt::LeftButton) {
        foreground = true;
    }
    entrySelection(foreground);
}

void KisPaletteView::paletteModelChanged()
{
    updateView();
    updateRows();
}

void KisPaletteView::setPaletteModel(KisPaletteModel *model)
{
    if (m_d->model) {
        disconnect(m_d->model, 0, this, 0);
    }
    m_d->model = model;
    setModel(model);
    paletteModelChanged();
    connect(m_d->model, SIGNAL(layoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)), this, SLOT(paletteModelChanged()));
    connect(m_d->model, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), this, SLOT(paletteModelChanged()));
    connect(m_d->model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(paletteModelChanged()));
    connect(m_d->model, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(paletteModelChanged()));
    connect(m_d->model, SIGNAL(modelReset()), this, SLOT(paletteModelChanged()));

}

KisPaletteModel* KisPaletteView::paletteModel() const
{
    return m_d->model;
}

void KisPaletteView::updateRows()
{
    this->clearSpans();
    if (m_d->model) {
        for (int r=0; r<=m_d->model->rowCount(); r++) {
            QModelIndex index = m_d->model->index(r, 0);
            if (qvariant_cast<bool>(index.data(KisPaletteModel::IsHeaderRole))) {
                setSpan(r, 0, 1, m_d->model->columnCount());
                setRowHeight(r, this->fontMetrics().lineSpacing()+6);
            } else {
                this->setRowHeight(r, this->columnWidth(0));
            }
        }
    }
}

void KisPaletteView::setAllowModification(bool allow)
{
    m_d->allowPaletteModification = allow;
}


void KisPaletteView::wheelEvent(QWheelEvent *event)
{
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
           KisConfig cfg;
           cfg.setPaletteDockerPaletteViewSectionSize(setSize);
       }

        event->accept();
    } else {
        KoTableView::wheelEvent(event);
    }
}

void KisPaletteView::entrySelection(bool foreground) {
    QModelIndex index;
    if (selectedIndexes().size()<=0) {
        return;
    }
    if (selectedIndexes().last().isValid()) {
        index = selectedIndexes().last();
    } else if (selectedIndexes().first().isValid()) {
        index = selectedIndexes().first();
    } else {
        return;
    }
    if (qvariant_cast<bool>(index.data(KisPaletteModel::IsHeaderRole))==false) {
        KoColorSetEntry entry = m_d->model->colorSetEntryFromIndex(index);
        if (foreground) {
            emit(entrySelected(entry));
            emit(indexEntrySelected(index));
        } else {
            emit(entrySelectedBackGround(entry));
            emit(indexEntrySelected(index));
        }
    }
}

void KisPaletteView::modifyEntry(QModelIndex index) {
    if (m_d->allowPaletteModification) {
        KoDialog *group = new KoDialog();
        QFormLayout *editableItems = new QFormLayout();
        group->mainWidget()->setLayout(editableItems);
        QLineEdit *lnIDName = new QLineEdit();
        QLineEdit *lnGroupName = new QLineEdit();
        KisColorButton *bnColor = new KisColorButton();
        bnColor->setPaletteViewEnabled(false);
        QCheckBox *chkSpot = new QCheckBox();

        if (qvariant_cast<bool>(index.data(KisPaletteModel::IsHeaderRole))) {
            QString groupName = qvariant_cast<QString>(index.data(Qt::DisplayRole));
            editableItems->addRow(i18nc("Name for a colorgroup","Name"), lnGroupName);
            lnGroupName->setText(groupName);
            if (group->exec() == KoDialog::Accepted) {
                m_d->model->colorSet()->changeGroupName(groupName, lnGroupName->text());
                m_d->model->colorSet()->save();
                updateRows();
            }
            //rename the group.
        } else {
            KoColorSetEntry entry = m_d->model->colorSetEntryFromIndex(index);
            QStringList entryList = qvariant_cast<QStringList>(index.data(KisPaletteModel::RetrieveEntryRole));
            chkSpot->setToolTip(i18nc("@info:tooltip", "A spot color is a color that the printer is able to print without mixing the paints it has available to it. The opposite is called a process color."));
            editableItems->addRow(i18n("ID"), lnIDName);
            editableItems->addRow(i18n("Name"), lnGroupName);
            editableItems->addRow(i18n("Color"), bnColor);
            editableItems->addRow(i18n("Spot"), chkSpot);
            lnGroupName->setText(entry.name);
            lnIDName->setText(entry.id);
            bnColor->setColor(entry.color);
            chkSpot->setChecked(entry.spotColor);
            if (group->exec() == KoDialog::Accepted) {
                entry.name = lnGroupName->text();
                entry.id = lnIDName->text();
                entry.color = bnColor->color();
                entry.spotColor = chkSpot->isChecked();
                m_d->model->colorSet()->changeColorSetEntry(entry, entryList.at(0), entryList.at(1).toUInt());
                m_d->model->colorSet()->save();
            }
        }
    }
}
