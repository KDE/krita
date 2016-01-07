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

#include "ui_BibliographyConfigureDialog.h"
#include "BibliographyConfigureDialog.h"

#include <KoTextDocument.h>
#include <KoStyleManager.h>

#include <QAbstractButton>
#include <QHBoxLayout>
#include <QComboBox>
#include <QRadioButton>
#include <QGroupBox>

BibliographyConfigureDialog::BibliographyConfigureDialog(const QTextDocument *document, QWidget *parent) 
    : QDialog(parent)
    ,  m_document(document)
    , m_bibConfiguration(KoTextDocument(m_document).styleManager()->bibliographyConfiguration())
{
    dialog.setupUi(this);
    dialog.prefix->setText(m_bibConfiguration->prefix());
    dialog.suffix->setText(m_bibConfiguration->suffix());
    dialog.numberedEntries->setChecked(m_bibConfiguration->numberedEntries());
    dialog.sortAlgorithm->setCurrentIndex(
        dialog.sortAlgorithm->findText(m_bibConfiguration->sortAlgorithm(), Qt::MatchFixedString));

    dialog.sortByPosition->setChecked(m_bibConfiguration->sortByPosition());

    connect(dialog.buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(save(QAbstractButton*)));
    connect(dialog.addSortKeyButton, SIGNAL(clicked()), this, SLOT(addSortKey()));
    connect(dialog.sortByPosition, SIGNAL(clicked(bool)), this, SLOT(sortMethodChanged(bool)));

    dialog.sortKeyGroupBox->setDisabled(m_bibConfiguration->sortByPosition());

    if (m_bibConfiguration->sortKeys().isEmpty()) {
        m_bibConfiguration->setSortKeys(m_bibConfiguration->sortKeys()
                                        << QPair<QString, Qt::SortOrder>("identifier", Qt::AscendingOrder));
    }

    foreach (const SortKeyPair &key, m_bibConfiguration->sortKeys()) {
        dialog.sortKeyGroupBox->layout()->addWidget(
            new SortKeyWidget(key.first, key.second, dialog.sortKeyGroupBox));
    }

    show();
}

void BibliographyConfigureDialog::save(QAbstractButton *button)
{
    if (dialog.buttonBox->standardButton(button) == dialog.buttonBox->Apply) {

        m_bibConfiguration->setPrefix(dialog.prefix->text());
        m_bibConfiguration->setSuffix(dialog.suffix->text());
        m_bibConfiguration->setSortAlgorithm(dialog.sortAlgorithm->currentText());
        m_bibConfiguration->setSortByPosition(dialog.sortByPosition->isChecked());
        m_bibConfiguration->setNumberedEntries(dialog.numberedEntries->isChecked());

        QList<SortKeyPair> sortKeys;

        foreach (QObject *o, dialog.sortKeyGroupBox->children()) {
            SortKeyWidget *widget = dynamic_cast<SortKeyWidget *>(o);
            if (widget) {
                sortKeys << SortKeyPair(widget->sortKey(), widget->sortOrder());
            }
        }
        m_bibConfiguration->setSortKeys(sortKeys);

        KoTextDocument(m_document).styleManager()->setBibliographyConfiguration(m_bibConfiguration);
    }
    emit accept();
}

void BibliographyConfigureDialog::addSortKey()
{
    dialog.sortKeyGroupBox->layout()->addWidget(
        new SortKeyWidget("identifier", Qt::AscendingOrder, dialog.sortKeyGroupBox));
}

void BibliographyConfigureDialog::sortMethodChanged(bool sortByPosition)
{
    m_bibConfiguration->setSortByPosition(sortByPosition);

    if (!sortByPosition && m_bibConfiguration->sortKeys().isEmpty()) {
        m_bibConfiguration->setSortKeys(m_bibConfiguration->sortKeys()
                                        << QPair<QString, Qt::SortOrder>("identifier", Qt::AscendingOrder));
    }
}

SortKeyWidget::SortKeyWidget(const QString &sortKey, Qt::SortOrder order, QWidget *parent) :
    QWidget(parent),
    m_dataFields(new QComboBox),
    m_ascButton(new QRadioButton(i18n("Ascending"))),
    m_dscButton(new QRadioButton(i18n("Descending"))),
    m_layout(new QHBoxLayout)
{
    setLayout(m_layout);
    m_dataFields->addItems(KoOdfBibliographyConfiguration::bibDataFields);
    setSortKey(sortKey);
    setSortOrder(order);

    m_layout->addWidget(m_dataFields);
    m_layout->addWidget(m_ascButton);
    m_layout->addWidget(m_dscButton);
}

void SortKeyWidget::setSortKey(const QString &sortKey)
{
    int sortKeyIndex = KoOdfBibliographyConfiguration::bibDataFields.indexOf(sortKey);
    if (sortKeyIndex != -1) {
        m_dataFields->setCurrentIndex(sortKeyIndex);
    }
}

void SortKeyWidget::setSortOrder(Qt::SortOrder order)
{
    if (order == Qt::DescendingOrder) {
        m_dscButton->setChecked(true);
    } else {
        m_ascButton->setChecked(true);
    }
}

QString SortKeyWidget::sortKey() const
{
    return m_dataFields->currentText();
}

Qt::SortOrder SortKeyWidget::sortOrder() const
{
    return (m_ascButton->isChecked()) ? Qt::AscendingOrder : Qt::DescendingOrder;
}
