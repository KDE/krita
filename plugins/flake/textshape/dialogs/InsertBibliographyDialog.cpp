/* This file is part of the KDE project
 * Copyright (C) 2011 Smit Patel <smitpatel24@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "InsertBibliographyDialog.h"

#include <ToCBibGeneratorInfo.h>
#include <BibliographyGenerator.h>
#include <KoParagraphStyle.h>
#include <KoOdfBibliographyConfiguration.h>
#include <KoBibliographyInfo.h>

#include <QMessageBox>
#include <QListWidgetItem>
#include <QDebug>

InsertBibliographyDialog::InsertBibliographyDialog(KoTextEditor *editor, QWidget *parent) 
    : QDialog(parent)
    , m_editor(editor)
    , m_bibInfo(new KoBibliographyInfo())
{
    dialog.setupUi(this);

    connect(dialog.bibTypes, SIGNAL(currentTextChanged(QString)), this, SLOT(updateFields()));
    connect(dialog.buttonBox, SIGNAL(accepted()), this, SLOT(insert()));
    connect(dialog.add, SIGNAL(clicked()), this, SLOT(addField()));
    connect(dialog.remove, SIGNAL(clicked()), this, SLOT(removeField()));
    connect(dialog.span, SIGNAL(clicked()), this, SLOT(addSpan()));
    connect(dialog.addedFields, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(spanChanged(QListWidgetItem*)));

    /*  To do : handle tab stops
    */
    //connect(dialog.addTabStop,SIGNAL(clicked()),this,SLOT(insertTabStop()));
    //connect(dialog.removeTabStop,SIGNAL(clicked()),this,SLOT(removeTabStop()));

    dialog.addedFields->clear();
    dialog.availableFields->clear();
    m_bibInfo->m_entryTemplate = BibliographyGenerator::defaultBibliographyEntryTemplates();
    dialog.bibTypes->setCurrentRow(0, QItemSelectionModel::Select);
    show();
}

QString InsertBibliographyDialog::bibliographyType()
{
    return dialog.bibTypes->currentItem()->text().remove(' ').toLower();
}

void InsertBibliographyDialog::insert()
{
    m_bibInfo->m_indexTitleTemplate.text = dialog.title->text();
    m_editor->insertBibliography(m_bibInfo);
}

void InsertBibliographyDialog::updateFields()
{
    dialog.availableFields->clear();
    dialog.addedFields->clear();

    QSet<QString> addedFields;
    Q_FOREACH (IndexEntry *entry, m_bibInfo->m_entryTemplate[bibliographyType()].indexEntries) {
        if (entry->name == IndexEntry::BIBLIOGRAPHY) {
            IndexEntryBibliography *bibEntry = static_cast<IndexEntryBibliography *>(entry);
            QListWidgetItem *bibItem = new QListWidgetItem(bibEntry->dataField, dialog.addedFields);
            addedFields.insert(bibEntry->dataField);
            bibItem->setData(Qt::UserRole, QVariant::fromValue<IndexEntry::IndexEntryName>(IndexEntry::BIBLIOGRAPHY));
        } else if (entry->name == IndexEntry::SPAN) {
            IndexEntrySpan *span = static_cast<IndexEntrySpan *>(entry);
            QListWidgetItem *spanField = new QListWidgetItem(span->text, dialog.addedFields);
            addedFields.insert(span->text);
            spanField->setData(Qt::UserRole, QVariant::fromValue<IndexEntry::IndexEntryName>(IndexEntry::SPAN));
            spanField->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        }
    }
    QSet<QString> availableFields = QSet<QString>::fromList(KoOdfBibliographyConfiguration::bibDataFields) - addedFields;

    foreach (const QString &field, availableFields) {
        new QListWidgetItem(field, dialog.availableFields);
    }
    dialog.availableFields->sortItems();
}

void InsertBibliographyDialog::addField()
{
    int row = dialog.availableFields->currentRow();

    if (row != -1) {

        disconnect(dialog.addedFields, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(spanChanged(QListWidgetItem*)));

        QString newDataField = dialog.availableFields->takeItem(row)->text();
        QListWidgetItem *bibField = new QListWidgetItem(newDataField, dialog.addedFields);
        bibField->setData(Qt::UserRole, QVariant::fromValue<IndexEntry::IndexEntryName>(IndexEntry::BIBLIOGRAPHY));

        IndexEntryBibliography *newEntry = new IndexEntryBibliography(QString());
        newEntry->dataField = newDataField;

        m_bibInfo->m_entryTemplate[bibliographyType()].indexEntries.append(static_cast<IndexEntry *>(newEntry));
        connect(dialog.addedFields, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(spanChanged(QListWidgetItem*)));
    }
}

void InsertBibliographyDialog::removeField()
{
    int row = dialog.addedFields->currentRow();

    if (row != -1) {
        if (dialog.addedFields->currentItem()->data(Qt::UserRole).value<IndexEntry::IndexEntryName>() == IndexEntry::BIBLIOGRAPHY) {
            new QListWidgetItem(dialog.addedFields->takeItem(row)->text(), dialog.availableFields);
            dialog.availableFields->sortItems();
        } else {
            dialog.availableFields->removeItemWidget(dialog.addedFields->takeItem(row));
        }

        m_bibInfo->m_entryTemplate[bibliographyType()].indexEntries.removeAt(row);
    }
}

void InsertBibliographyDialog::addSpan()
{
    QString spanText = (dialog.addedFields->count() == 1) ? QString(":") : QString(",");
    QListWidgetItem *spanField = new QListWidgetItem(spanText, dialog.addedFields);
    spanField->setData(Qt::UserRole, QVariant::fromValue<IndexEntry::IndexEntryName>(IndexEntry::SPAN));
    spanField->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    IndexEntrySpan *span = new IndexEntrySpan(QString());
    span->text = spanText;

    m_bibInfo->m_entryTemplate[bibliographyType()].indexEntries.append(static_cast<IndexEntry *>(span));
}

void InsertBibliographyDialog::spanChanged(QListWidgetItem *item)
{
    int row = dialog.addedFields->currentRow();

    if (row != -1) {
        IndexEntrySpan *span = static_cast<IndexEntrySpan *>(m_bibInfo->m_entryTemplate[bibliographyType()].indexEntries.at(row));
        span->text = item->text();
    }
}

void InsertBibliographyDialog::insertTabStop()
{
    /*QListWidgetItem *tabStopItem = new QListWidgetItem(QString("Tab stop"),dialog.availableFields);

    IndexEntryTabStop *tabStop = new IndexEntryTabStop(QString());*/
}

void InsertBibliographyDialog::removeTabStop()
{
    /*int row = dialog.addedFields->row(dialog.addedFields->currentItem());

    if (row != -1 && dialog.addedFields->takeItem(row)->text() == "Tab stop") {
        dialog.addedFields->removeItemWidget(dialog.addedFields->takeItem(row));
    }*/
}
