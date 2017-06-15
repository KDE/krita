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
    QPalette pal(palette());
    pal.setColor(QPalette::Base, cfg.getMDIBackgroundColor());
    setAutoFillBackground(true);
    setPalette(pal);

    int defaultSectionSize = cfg.paletteDockerPaletteViewSectionSize();
    horizontalHeader()->setDefaultSectionSize(defaultSectionSize);
    verticalHeader()->setDefaultSectionSize(defaultSectionSize);
    connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(entrySelection()) );
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
    window->setWindowTitle("Add a new Colorset Entry");
    QFormLayout *editableItems = new QFormLayout();
    window->mainWidget()->setLayout(editableItems);
    QComboBox *cmbGroups = new QComboBox();
    QString defaultGroupName = "Default";
    cmbGroups->addItem(defaultGroupName);
    cmbGroups->addItems(m_d->model->colorSet()->getGroupNames());
    QLineEdit *lnIDName = new QLineEdit();
    QLineEdit *lnName = new QLineEdit();
    KisColorButton *bnColor = new KisColorButton();
    QCheckBox *chkSpot = new QCheckBox();
    editableItems->addRow(tr("Group"), cmbGroups);
    editableItems->addRow(tr("ID"), lnIDName);
    editableItems->addRow(tr("Name"), lnName);
    editableItems->addRow(tr("Color"), bnColor);
    editableItems->addRow(tr("Spot"), chkSpot);
    cmbGroups->setCurrentIndex(0);
    lnName->setText("Color "+QString::number(m_d->model->colorSet()->nColors()+1));
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
    window->setWindowTitle("Add a new group");
    QFormLayout *editableItems = new QFormLayout();
    window->mainWidget()->setLayout(editableItems);
    QLineEdit *lnName = new QLineEdit();
    editableItems->addRow(tr("Name"), lnName);
    lnName->setText("Color Group "+QString::number(m_d->model->colorSet()->getGroupNames().size()+1));
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
    if (qVariantValue<bool>(index.data(KisPaletteModel::IsHeaderRole))) {
        KoDialog *window = new KoDialog();
        window->setWindowTitle("Removing Group");
        QFormLayout *editableItems = new QFormLayout();
        QCheckBox *chkKeep = new QCheckBox();
        window->mainWidget()->setLayout(editableItems);
        editableItems->addRow(tr("Keep the Colors"), chkKeep);
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
    connect(m_d->model, SIGNAL(layoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)), this, SLOT(paletteModelChanged()));
    connect(m_d->model, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), this, SLOT(paletteModelChanged()));
    connect(m_d->model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(paletteModelChanged()));
    connect(m_d->model, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(paletteModelChanged()));

}

KisPaletteModel* KisPaletteView::paletteModel() const
{
    return m_d->model;
}

void KisPaletteView::updateRows()
{
    this->clearSpans();
    for (int r=0; r<=m_d->model->rowCount(); r++) {
        QModelIndex index = m_d->model->index(r, 0);
        if (qVariantValue<bool>(index.data(KisPaletteModel::IsHeaderRole))) {
            setSpan(r, 0, 1, m_d->model->columnCount());
            setRowHeight(r, this->fontMetrics().lineSpacing()+6);
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

        if ( setSize >= 12 ) {
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

void KisPaletteView::entrySelection() {
    QModelIndex index = selectedIndexes().last();
    if (!index.isValid()) {
        index = selectedIndexes().first();
    }
    if (!index.isValid()) {
        return;
    }
    if (qVariantValue<bool>(index.data(KisPaletteModel::IsHeaderRole))==false) {
        KoColorSetEntry entry = m_d->model->colorSetEntryFromIndex(index);
        emit(entrySelected(entry));
    }
}

void KisPaletteView::modifyEntry(QModelIndex index) {
    if (m_d->allowPaletteModification) {
        KoDialog *group = new KoDialog();
        //QHBoxLayout *mainLayout = new QHBoxLayout();
        QFormLayout *editableItems = new QFormLayout();
        group->mainWidget()->setLayout(editableItems);
        QLineEdit *lnIDName = new QLineEdit();
        QLineEdit *lnGroupName = new QLineEdit();
        KisColorButton *bnColor = new KisColorButton();
        QCheckBox *chkSpot = new QCheckBox();

        if (qVariantValue<bool>(index.data(KisPaletteModel::IsHeaderRole))) {
            QString groupName = qVariantValue<QString>(index.data(Qt::DisplayRole));
            editableItems->addRow(tr("Name"), lnGroupName);
            lnGroupName->setText(groupName);
            if (group->exec() == KoDialog::Accepted) {
                m_d->model->colorSet()->changeGroupName(groupName, lnGroupName->text());
                m_d->model->colorSet()->save();
                updateRows();
            }
            //rename the group.
        } else {
            KoColorSetEntry entry = m_d->model->colorSetEntryFromIndex(index);
            QStringList entryList = qVariantValue<QStringList>(index.data(KisPaletteModel::RetrieveEntryRole));
            editableItems->addRow(tr("ID"), lnIDName);
            editableItems->addRow(tr("Name"), lnGroupName);
            editableItems->addRow(tr("Color"), bnColor);
            editableItems->addRow(tr("Spot"), chkSpot);
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
