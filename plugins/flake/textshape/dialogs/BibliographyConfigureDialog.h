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
#ifndef BIBLIOGRAPHYCONFIGUREDIALOG_H
#define BIBLIOGRAPHYCONFIGUREDIALOG_H

#include "ui_BibliographyConfigureDialog.h"

#include <QDialog>
#include <QTextDocument>
#include <QComboBox>
#include <QRadioButton>
#include <QHBoxLayout>

#include "KoOdfBibliographyConfiguration.h"

class BibliographyConfigureDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BibliographyConfigureDialog(const QTextDocument *document, QWidget *parent = 0);

public Q_SLOTS:
    void addSortKey();
    void save(QAbstractButton *);
    void sortMethodChanged(bool);

private:
    Ui::BibliographyConfigureDialog dialog;
    const QTextDocument *m_document;
    KoOdfBibliographyConfiguration *m_bibConfiguration;
};

class SortKeyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SortKeyWidget(const QString &sortKey, Qt::SortOrder order, QWidget *parent);

    QString sortKey() const;
    Qt::SortOrder sortOrder() const;

    void setSortKey(const QString &sortKey);
    void setSortOrder(Qt::SortOrder order);

private:
    QComboBox *m_dataFields;
    QRadioButton *m_ascButton;
    QRadioButton *m_dscButton;
    QHBoxLayout *m_layout;

};

#endif // BIBLIOGRAPHYCONFIGUREDIALOG_H
