/* This file is part of the KDE project
 * Copyright (C) 2011 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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

#include "TableOfContentsEntryDelegate.h"

#include <klocalizedstring.h>
#include <KoStyleManager.h>
#include <KoParagraphStyle.h>
#include <KoStyleThumbnailer.h>

#include <QComboBox>
#include <QPainter>

TableOfContentsEntryDelegate::TableOfContentsEntryDelegate(KoStyleManager *manager)
    : QStyledItemDelegate()
    , m_styleManager(manager)
{
    Q_ASSERT(manager);
}

QSize TableOfContentsEntryDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(250, 48);
}

QWidget *TableOfContentsEntryDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &/* option */,
        const QModelIndex &/* index */) const
{
    QComboBox *editor = new QComboBox(parent);
    return editor;
}

void TableOfContentsEntryDelegate::setEditorData(QWidget *editor,
        const QModelIndex &index) const
{
    int value = index.model()->data(index, Qt::EditRole).toInt();
    QComboBox *comboBox = static_cast<QComboBox *>(editor);

    QList<KoParagraphStyle *> paragraphStyles = m_styleManager->paragraphStyles();
    int count = 0;
    int indexCount = 0;
    foreach (const KoParagraphStyle *style, paragraphStyles) {
        comboBox->addItem(style->name());
        comboBox->setItemData(count, style->styleId());

        if (style->styleId() == value) {
            indexCount = count;
        }

        count++;
    }

    comboBox->setCurrentIndex(indexCount);
}

void TableOfContentsEntryDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox *>(editor);
    int value = comboBox->itemData(comboBox->currentIndex()).toInt();

    model->setData(index, value, Qt::EditRole);
}

void TableOfContentsEntryDelegate::updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
