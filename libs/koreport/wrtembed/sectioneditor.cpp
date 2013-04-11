/*
 * OpenRPT report writer and rendering engine
 * Copyright (C) 2001-2007 by OpenMFG, LLC (info@openmfg.com)
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 * Copyright (C) 2012 by Friedrich W. H. Kossebau (kossebau@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sectioneditor.h"
#include "KoReportDesigner.h"
#include "reportsection.h"
#include "reportsectiondetail.h"
#include "detailgroupsectiondialog.h"
#include "reportsectiondetailgroup.h"
#include <KoIcon.h>
// KDE
#include <kpushbutton.h>
#include <klocalizedstring.h>
// Qt
#include <QVariant>
#include <QComboBox>
#include <QLineEdit>
#include <QMessageBox>

enum {
    KeyRole = Qt::UserRole
};

// KoReportDesigner currently prepends an empty key/fieldname pair to the list
// of fields, possibly to offer the option to have report elements not yet
// bound to fields
static inline bool isEditorHelperField(const QString &key)
{
    return key.isEmpty();
}

/*
 *  Constructs a SectionEditor as a child of 'parent'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
SectionEditor::SectionEditor(QWidget* parent)
  : KDialog(parent)
{
    setButtons(Close);
    setCaption(i18n("Section Editor"));

    QWidget *widget = new QWidget(this);
    m_ui.setupUi(widget);
    m_btnAdd = new KPushButton(koIcon("list-add"), i18n("Add..."), this);
    m_ui.lGroupSectionsButtons->addWidget(m_btnAdd);
    m_btnEdit = new KPushButton(koIcon("document-edit"), i18n("Edit..."), this);
    m_ui.lGroupSectionsButtons->addWidget(m_btnEdit);
    m_btnRemove = new KPushButton(koIcon("list-remove"), i18n("Delete"), this);
    m_ui.lGroupSectionsButtons->addWidget(m_btnRemove);
    m_btnMoveUp = new KPushButton(koIcon("arrow-up"), i18n("Move Up"), this);
    m_ui.lGroupSectionsButtons->addWidget(m_btnMoveUp);
    m_btnMoveDown = new KPushButton(koIcon("arrow-down"), i18n("Move Down"), this);
    m_ui.lGroupSectionsButtons->addWidget(m_btnMoveDown);
    m_ui.lGroupSectionsButtons->addStretch();
    setMainWidget(widget);

    // signals and slots connections
    connect(m_ui.cbReportHeader, SIGNAL(toggled(bool)), this, SLOT(cbReportHeader_toggled(bool)));
    connect(m_ui.cbReportFooter, SIGNAL(toggled(bool)), this, SLOT(cbReportFooter_toggled(bool)));
    connect(m_ui.cbHeadFirst, SIGNAL(toggled(bool)), this, SLOT(cbHeadFirst_toggled(bool)));
    connect(m_ui.cbHeadLast, SIGNAL(toggled(bool)), this, SLOT(cbHeadLast_toggled(bool)));
    connect(m_ui.cbHeadEven, SIGNAL(toggled(bool)), this, SLOT(cbHeadEven_toggled(bool)));
    connect(m_ui.cbHeadOdd, SIGNAL(toggled(bool)), this, SLOT(cbHeadOdd_toggled(bool)));
    connect(m_ui.cbFootFirst, SIGNAL(toggled(bool)), this, SLOT(cbFootFirst_toggled(bool)));
    connect(m_ui.cbFootLast, SIGNAL(toggled(bool)), this, SLOT(cbFootLast_toggled(bool)));
    connect(m_ui.cbFootEven, SIGNAL(toggled(bool)), this, SLOT(cbFootEven_toggled(bool)));
    connect(m_ui.cbFootOdd, SIGNAL(toggled(bool)), this, SLOT(cbFootOdd_toggled(bool)));
    connect(m_ui.cbHeadAny, SIGNAL(toggled(bool)), this, SLOT(cbHeadAny_toggled(bool)));
    connect(m_ui.cbFootAny, SIGNAL(toggled(bool)), this, SLOT(cbFootAny_toggled(bool)));
    connect(m_ui.lbGroups, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(updateButtonsForItem(QListWidgetItem*)));
    connect(m_ui.lbGroups, SIGNAL(currentRowChanged(int)),
            this, SLOT(updateButtonsForRow(int)));

    connect(m_btnAdd, SIGNAL(clicked(bool)), this, SLOT(btnAdd_clicked()));
    connect(m_btnEdit, SIGNAL(clicked(bool)), this, SLOT(btnEdit_clicked()));
    connect(m_btnRemove, SIGNAL(clicked(bool)), this, SLOT(btnRemove_clicked()));
    connect(m_btnMoveUp, SIGNAL(clicked(bool)), this, SLOT(btnMoveUp_clicked()));
    connect(m_btnMoveDown, SIGNAL(clicked(bool)), this, SLOT(brnMoveDown_clicked()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
SectionEditor::~SectionEditor()
{
    // no need to delete child widgets, Qt does it all for us
}

void SectionEditor::cbReportHeader_toggled(bool yes)
{
    if (m_reportDesigner) {
        if (yes) {
            m_reportDesigner->insertSection(KRSectionData::ReportHeader);
        } else {
            m_reportDesigner->removeSection(KRSectionData::ReportHeader);
        }
    }

}

void SectionEditor::cbReportFooter_toggled(bool yes)
{
    if (m_reportDesigner) {
        if (yes) {
            m_reportDesigner->insertSection(KRSectionData::ReportFooter);
        } else {
            m_reportDesigner->removeSection(KRSectionData::ReportFooter);
        }
    }

}

void SectionEditor::cbHeadFirst_toggled(bool yes)
{
    if (m_reportDesigner) {
        if (yes) {
            m_reportDesigner->insertSection(KRSectionData::PageHeaderFirst);
        } else {
            m_reportDesigner->removeSection(KRSectionData::PageHeaderFirst);
        }
    }

}

void SectionEditor::cbHeadLast_toggled(bool yes)
{
    if (m_reportDesigner) {
        if (yes) {
            m_reportDesigner->insertSection(KRSectionData::PageHeaderLast);
        } else {
            m_reportDesigner->removeSection(KRSectionData::PageHeaderLast);
        }
    }

}

void SectionEditor::cbHeadEven_toggled(bool yes)
{
    if (m_reportDesigner) {
        if (yes) {
            m_reportDesigner->insertSection(KRSectionData::PageHeaderEven);
        } else {
            m_reportDesigner->removeSection(KRSectionData::PageHeaderEven);
        }
    }

}

void SectionEditor::cbHeadOdd_toggled(bool yes)
{
    if (m_reportDesigner) {
        if (yes) {
            m_reportDesigner->insertSection(KRSectionData::PageHeaderOdd);
        } else {
            m_reportDesigner->removeSection(KRSectionData::PageHeaderOdd);
        }
    }

}

void SectionEditor::cbFootFirst_toggled(bool yes)
{
    if (m_reportDesigner) {
        if (yes) {
            m_reportDesigner->insertSection(KRSectionData::PageFooterFirst);
        } else {
            m_reportDesigner->removeSection(KRSectionData::PageFooterFirst);
        }
    }

}

void SectionEditor::cbFootLast_toggled(bool yes)
{
    if (m_reportDesigner) {
        if (yes) {
            m_reportDesigner->insertSection(KRSectionData::PageFooterLast);
        } else {
            m_reportDesigner->removeSection(KRSectionData::PageFooterLast);
        }
    }

}

void SectionEditor::cbFootEven_toggled(bool yes)
{
    if (m_reportDesigner) {
        if (yes) {
            m_reportDesigner->insertSection(KRSectionData::PageFooterEven);
        } else {
            m_reportDesigner->removeSection(KRSectionData::PageFooterEven);
        }
    }

}

void SectionEditor::cbFootOdd_toggled(bool yes)
{
    if (m_reportDesigner) {
        if (yes) {
            m_reportDesigner->insertSection(KRSectionData::PageFooterOdd);
        } else {
            m_reportDesigner->removeSection(KRSectionData::PageFooterOdd);
        }
    }

}


void SectionEditor::init(KoReportDesigner * rw)
{
    m_reportDesigner = 0;
    // set all the properties

    m_ui.cbReportHeader->setChecked(rw->section(KRSectionData::ReportHeader));
    m_ui.cbReportFooter->setChecked(rw->section(KRSectionData::ReportFooter));

    m_ui.cbHeadFirst->setChecked(rw->section(KRSectionData::PageHeaderFirst));
    m_ui.cbHeadOdd->setChecked(rw->section(KRSectionData::PageHeaderOdd));
    m_ui.cbHeadEven->setChecked(rw->section(KRSectionData::PageHeaderEven));
    m_ui.cbHeadLast->setChecked(rw->section(KRSectionData::PageHeaderLast));
    m_ui.cbHeadAny->setChecked(rw->section(KRSectionData::PageHeaderAny));

    m_ui.cbFootFirst->setChecked(rw->section(KRSectionData::PageFooterFirst));
    m_ui.cbFootOdd->setChecked(rw->section(KRSectionData::PageFooterOdd));
    m_ui.cbFootEven->setChecked(rw->section(KRSectionData::PageFooterEven));
    m_ui.cbFootLast->setChecked(rw->section(KRSectionData::PageFooterLast));
    m_ui.cbFootAny->setChecked(rw->section(KRSectionData::PageFooterAny));

    // now set the rw value
    m_reportDesigner = rw;
    m_reportSectionDetail = rw->detailSection();

    if (m_reportSectionDetail) {
        const QStringList columnNames = m_reportDesigner->fieldNames();
        const QStringList keys = m_reportDesigner->fieldKeys();
        for (int i = 0; i < m_reportSectionDetail->groupSectionCount(); ++i) {
            const QString key = m_reportSectionDetail->groupSection(i)->column();
            const int idx = keys.indexOf(key);
            const QString columnName = columnNames.value(idx);
            QListWidgetItem *item = new QListWidgetItem(columnName);
            item->setData(KeyRole, key);
            m_ui.lbGroups->addItem(item);
        }
    }
    if (m_ui.lbGroups->count() == 0) {
    } else {
        m_ui.lbGroups->setCurrentItem(m_ui.lbGroups->item(0));
    }
    updateButtonsForItem(m_ui.lbGroups->currentItem());
    updateAddButton();
    updateButtonsForRow(m_ui.lbGroups->currentRow());
}

bool SectionEditor::editDetailGroup(ReportSectionDetailGroup * rsdg)
{
    DetailGroupSectionDialog * dgsd = new DetailGroupSectionDialog(this);

    // add the current column and all columns not yet used for groups
    const QStringList keys = m_reportDesigner->fieldKeys();
    const QStringList columnNames = m_reportDesigner->fieldNames();
    // in case of to-be-added group that column needs to be added to the used
    const QSet<QString> usedColumns = groupingColumns() << rsdg->column();
    // if the current column is not among the keys, something is broken.
    // for now just simply select no column in the combobox, achieved by -1
    int indexOfCurrentColumn = -1;
    for (int i = 0; i < keys.count(); ++i) {
        const QString &key = keys.at(i);
        // skip any editor helper fields
        if (isEditorHelperField(key)) {
            continue;
        }
        // already used?
        if (usedColumns.contains(key)) {
            // and not the one of the group?
            if (key != rsdg->column()) {
                continue;
            }
            // remember index
            indexOfCurrentColumn = dgsd->cbColumn->count();
        }
        dgsd->cbColumn->insertItem( i, columnNames.value(i), key);
    }
    dgsd->cbColumn->setCurrentIndex(indexOfCurrentColumn);

    dgsd->cbSort->addItem(i18n("Ascending"), Qt::AscendingOrder);
    dgsd->cbSort->addItem(i18n("Descending"), Qt::DescendingOrder);
    dgsd->cbSort->setCurrentIndex(dgsd->cbSort->findData(rsdg->sort()));

    dgsd->breakAfterFooter->setChecked(rsdg->pageBreak() == ReportSectionDetailGroup::BreakAfterGroupFooter);
    dgsd->cbHead->setChecked(rsdg->groupHeaderVisible());
    dgsd->cbFoot->setChecked(rsdg->groupFooterVisible());

    const bool isOkayed = (dgsd->exec() == QDialog::Accepted);

    if (isOkayed) {
        const QString newColumn =
            dgsd->cbColumn->itemData(dgsd->cbColumn->currentIndex()).toString();
        const QString oldColumn = rsdg->column();
        if (newColumn != oldColumn) {
            rsdg->setColumn(newColumn);
        }

        rsdg->setGroupHeaderVisible(dgsd->cbHead->isChecked());
        rsdg->setGroupFooterVisible(dgsd->cbFoot->isChecked());

        const ReportSectionDetailGroup::PageBreak pageBreak = dgsd->breakAfterFooter->isChecked() ?
            ReportSectionDetailGroup::BreakAfterGroupFooter : ReportSectionDetailGroup::BreakNone;
        rsdg->setPageBreak(pageBreak);

        const Qt::SortOrder sortOrder =
            static_cast<Qt::SortOrder>(dgsd->cbSort->itemData(dgsd->cbSort->currentIndex()).toInt());
        rsdg->setSort(sortOrder);
    }

    delete dgsd;

    return isOkayed;
}

QString SectionEditor::columnName(const QString &column) const
{
    const QStringList keys = m_reportDesigner->fieldKeys();
    const QStringList columnNames = m_reportDesigner->fieldNames();
    return columnNames.at(keys.indexOf(column));
}

QSet<QString> SectionEditor::groupingColumns() const
{
    QSet<QString> result;
    for (int i = 0; i < m_ui.lbGroups->count(); ++i) {
        result.insert(m_ui.lbGroups->item(i)->data(KeyRole).toString());
    }
    return result;
}

void SectionEditor::cbHeadAny_toggled(bool yes)
{
    if (m_reportDesigner) {
        if (yes) {
            m_reportDesigner->insertSection(KRSectionData::PageHeaderAny);
        } else {
            m_reportDesigner->removeSection(KRSectionData::PageHeaderAny);
        }
    }
}

void SectionEditor::cbFootAny_toggled(bool yes)
{
    if (m_reportDesigner) {
        if (yes) {
            m_reportDesigner->insertSection(KRSectionData::PageFooterAny);
        } else {
            m_reportDesigner->removeSection(KRSectionData::PageFooterAny);
        }
    }
}

void SectionEditor::btnEdit_clicked()
{
    if (m_reportSectionDetail) {
        const int idx = m_ui.lbGroups->currentRow();
        if (idx < 0) {
            return;
        }
        ReportSectionDetailGroup * rsdg = m_reportSectionDetail->groupSection(idx);
        if (editDetailGroup(rsdg)) {
            // update name in list
            m_ui.lbGroups->item(idx)->setText(columnName(rsdg->column()));
        }
    }
}

void SectionEditor::btnAdd_clicked()
{
    if (m_reportSectionDetail) {
        // lets add a new section
        // search for unused column
        QString column;
        const QStringList keys = m_reportDesigner->fieldKeys();
        const QSet<QString> columns = groupingColumns();
        foreach(const QString &key, keys) {
            // skip any editor helper fields
            if (isEditorHelperField(key)) {
                continue;
            }
            if (! columns.contains(key)) {
                column = key;
                break;
            }
        }
        // should not happen, but we do not really know if m_reportDesigner is in sync
        if (column.isEmpty()) {
            return;
        }

        // create new group, have it edited and add it, if not cancelled
        ReportSectionDetailGroup * rsdg =
            new ReportSectionDetailGroup(column, m_reportSectionDetail, m_reportSectionDetail);
        if (editDetailGroup(rsdg)) {
            // append to group sections
            m_reportSectionDetail->insertGroupSection(m_reportSectionDetail->groupSectionCount(), rsdg);
            // add to combobox
            const QString column = rsdg->column();
            QListWidgetItem *item = new QListWidgetItem(columnName(column));
            item->setData(KeyRole, column);
            m_ui.lbGroups->addItem(item);
            m_ui.lbGroups->setCurrentRow(m_ui.lbGroups->count() - 1);
            updateAddButton();
        } else {
            delete rsdg;
        }
    }
}


void SectionEditor::btnRemove_clicked()
{
    if (m_reportSectionDetail) {
        const int index = m_ui.lbGroups->currentRow();
        if (index != -1) {
            QListWidgetItem *item = m_ui.lbGroups->takeItem(index);
            delete item;
            m_reportSectionDetail->removeGroupSection(index, true);
            // a field got usable, so make sure add button is available again
            m_btnAdd->setEnabled(true);
            // workaround for at least Qt 4.8.1, which does not emit the proper
            // currentRowChanged signal on deletion of the first element
            updateButtonsForRow(m_ui.lbGroups->currentRow());
        }
    }
}


void SectionEditor::btnMoveUp_clicked()
{
    if (m_reportSectionDetail) {
        int idx = m_ui.lbGroups->currentRow();
        if (idx <= 0) return;
        QString s = m_ui.lbGroups->currentItem()->text();
        m_ui.lbGroups->takeItem(idx);
        m_ui.lbGroups->insertItem(idx - 1, s);
        m_ui.lbGroups->setCurrentRow(idx - 1, QItemSelectionModel::ClearAndSelect);
        ReportSectionDetailGroup * rsdg = m_reportSectionDetail->groupSection(idx);
        bool showgh = rsdg->groupHeaderVisible();
        bool showgf = rsdg->groupFooterVisible();
        m_reportSectionDetail->removeGroupSection(idx);
        m_reportSectionDetail->insertGroupSection(idx - 1, rsdg);
        rsdg->setGroupHeaderVisible(showgh);
        rsdg->setGroupFooterVisible(showgf);
    }
}


void SectionEditor::brnMoveDown_clicked()
{
    if (m_reportSectionDetail) {
        int idx = m_ui.lbGroups->currentRow();
        if (idx == (int)(m_ui.lbGroups->count() - 1)) return;
        QString s = m_ui.lbGroups->currentItem()->text();
        m_ui.lbGroups->takeItem(idx);
        m_ui.lbGroups->insertItem (idx + 1, s);
        m_ui.lbGroups->setCurrentRow(idx + 1, QItemSelectionModel::ClearAndSelect);
        ReportSectionDetailGroup * rsdg = m_reportSectionDetail->groupSection(idx);
        bool showgh = rsdg->groupHeaderVisible();
        bool showgf = rsdg->groupFooterVisible();
        m_reportSectionDetail->removeGroupSection(idx);
        m_reportSectionDetail->insertGroupSection(idx + 1, rsdg);
        rsdg->setGroupHeaderVisible(showgh);
        rsdg->setGroupFooterVisible(showgf);
    }
}

void SectionEditor::updateButtonsForItem(QListWidgetItem* currentItem)
{
    const bool isItemSelected = (currentItem != 0);

    m_btnEdit->setEnabled(isItemSelected);
    m_btnRemove->setEnabled(isItemSelected);

}

void SectionEditor::updateButtonsForRow(int row)
{
    const bool enableMoveUpButton = (row > 0);
    const bool enableMoveDownButton = (0 <= row) && (row+1 < m_ui.lbGroups->count());

    m_btnMoveUp->setEnabled(enableMoveUpButton);
    m_btnMoveDown->setEnabled(enableMoveDownButton);
}

void SectionEditor::updateAddButton()
{
    // search for unused column
    bool foundUnusedColumn = false;
    const QStringList keys = m_reportDesigner->fieldKeys();
    const QSet<QString> columns = groupingColumns();
    foreach(const QString &key, keys) {
        // skip any editor helper fields
        if (isEditorHelperField(key)) {
            continue;
        }
        if (! columns.contains(key)) {
            foundUnusedColumn = true;
            break;
        }
    }
    m_btnAdd->setEnabled(foundUnusedColumn);
}
