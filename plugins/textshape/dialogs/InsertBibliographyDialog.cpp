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
#include <ToCBibGeneratorInfo.h>

#include <QMessageBox>

InsertBibliographyDialog::InsertBibliographyDialog(KoTextEditor *editor, QWidget *parent) :
    QDialog(parent),
    m_editor(editor),
    m_bibInfo(new KoBibliographyInfo)
{
    dialog.setupUi(this);

    connect(dialog.bibTypes,SIGNAL(currentTextChanged(QString)),this,SLOT(updateFields()));
    connect(dialog.buttonBox,SIGNAL(accepted()),this,SLOT(insert()));
    connect(dialog.add,SIGNAL(clicked()),this,SLOT(addField()));
    connect(dialog.remove,SIGNAL(clicked()),this,SLOT(removeField()));
    /*  To do : handle tab stops
    */
    //connect(dialog.addTabStop,SIGNAL(clicked()),this,SLOT(insertTabStop()));
    //connect(dialog.removeTabStop,SIGNAL(clicked()),this,SLOT(removeTabStop()));

    setDefaultIndexEntries();
}

void InsertBibliographyDialog::insert()
{
    m_editor->insertBibliography();
    KoBibliographyInfo *bibInfo =
            m_editor->cursor()->block().blockFormat().property(KoParagraphStyle::BibliographyData).value<KoBibliographyInfo*>();
    QTextDocument *bibDocument =
            m_editor->cursor()->block().blockFormat().property(KoParagraphStyle::BibliographyDocument).value<QTextDocument*>();

    bibInfo->m_entryTemplate = m_bibInfo->m_entryTemplate;
    bibInfo->m_indexTitleTemplate.text = dialog.title->text();

    bool *autoUpdate = m_editor->cursor()->block().blockFormat().property(KoParagraphStyle::AutoUpdateBibliography).value<bool *>();
    *autoUpdate = dialog.autoupdate->isChecked();

    BibliographyGenerator *generator =
            new BibliographyGenerator(bibDocument, m_editor->cursor()->block(), bibInfo);

    if (!(*autoUpdate)) {          //if autoUpdate is disabled then do a forced generate on insertion
        generator->generate();
    }
}

void InsertBibliographyDialog::updateFields()
{
    dialog.availableFields->clear();
    dialog.addedFields->clear();

    QString bibType(dialog.bibTypes->currentItem()->text().remove(" ").toLower());

    QSet<QString> addedFields;
    foreach(IndexEntry *entry,m_bibInfo->m_entryTemplate[bibType].indexEntries) {
        if (entry->name == IndexEntry::BIBLIOGRAPHY) {
            IndexEntryBibliography *bibEntry = static_cast<IndexEntryBibliography *>(entry);
            new QListWidgetItem(bibEntry->dataField,dialog.addedFields);
            addedFields.insert(bibEntry->dataField);
        }
    }
    QSet<QString> availableFields = QSet<QString>::fromList(KoBibliographyInfo::bibDataFields) - addedFields;

    foreach (QString field, availableFields) {
        new QListWidgetItem(field,dialog.availableFields);
    }
    dialog.availableFields->sortItems();
}

void InsertBibliographyDialog::addField()
{
    int row = dialog.availableFields->row(dialog.availableFields->currentItem());

    if (row != -1) {
        QString newDataField = dialog.availableFields->takeItem(row)->text();
        new QListWidgetItem(newDataField,dialog.addedFields);

        QString bibType(dialog.bibTypes->currentItem()->text().remove(" ").toLower());

        //creating IndexEntries
        IndexEntryBibliography *newEntry = new IndexEntryBibliography(QString());
        newEntry->dataField = newDataField;
        IndexEntrySpan *span = new IndexEntrySpan(QString());
        if (dialog.addedFields->count() == 1) {
            span->text = ":";
        }
        else span->text = ",";

        m_bibInfo->m_entryTemplate[bibType].indexEntries.append(newEntry);
        m_bibInfo->m_entryTemplate[bibType].indexEntries.append(span);
    }
}

void InsertBibliographyDialog::removeField()
{
    int row = dialog.addedFields->row(dialog.addedFields->currentItem());

    if (row != -1) {
        new QListWidgetItem(dialog.addedFields->takeItem(row)->text(),dialog.availableFields);
        dialog.availableFields->sortItems();

        QString bibType(dialog.bibTypes->currentItem()->text().remove(" ").toLower());

        //Removing IndexEntries
        m_bibInfo->m_entryTemplate[bibType].indexEntries.removeAt(2*row);       //to remove IndexEntry
        m_bibInfo->m_entryTemplate[bibType].indexEntries.removeAt(2*row);       //to remove span text
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

void InsertBibliographyDialog::setDefaultIndexEntries()
{
    dialog.addedFields->clear();
    dialog.availableFields->clear();

    foreach (QString bibType, KoBibliographyInfo::bibTypes) {
        BibliographyEntryTemplate bibEntryTemplate;

        //Now creating default IndexEntries for all BibliographyEntryTemplates
        IndexEntryBibliography *identifier = new IndexEntryBibliography(QString());
        IndexEntryBibliography *author = new IndexEntryBibliography(QString());
        IndexEntryBibliography *title = new IndexEntryBibliography(QString());
        IndexEntryBibliography *year = new IndexEntryBibliography(QString());
        IndexEntrySpan *firstSpan = new IndexEntrySpan(QString());
        IndexEntrySpan *otherSpan = new IndexEntrySpan(QString());

        identifier->dataField = "identifier";
        author->dataField = "author";
        title->dataField = "title";
        year->dataField = "year";
        firstSpan->text = ":";
        otherSpan->text = ",";

        bibEntryTemplate.bibliographyType = bibType;
        bibEntryTemplate.indexEntries.append(static_cast<IndexEntry *>(identifier));
        bibEntryTemplate.indexEntries.append(static_cast<IndexEntry *>(firstSpan));
        bibEntryTemplate.indexEntries.append(static_cast<IndexEntry *>(author));
        bibEntryTemplate.indexEntries.append(static_cast<IndexEntry *>(otherSpan));
        bibEntryTemplate.indexEntries.append(static_cast<IndexEntry *>(title));
        bibEntryTemplate.indexEntries.append(static_cast<IndexEntry *>(otherSpan));
        bibEntryTemplate.indexEntries.append(static_cast<IndexEntry *>(year));

        m_bibInfo->m_entryTemplate[bibType] = bibEntryTemplate;
    }
    dialog.bibTypes->setCurrentRow(0,QItemSelectionModel::Select);
}
