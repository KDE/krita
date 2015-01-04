/*
 * This file is part of the KDE project
 * Copyright (C) 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_input_type_delegate.h"

#include <kcombobox.h>
#include <klocalizedstring.h>

class KisInputTypeDelegate::Private
{
public:
    Private() { }
};

KisInputTypeDelegate::KisInputTypeDelegate(QObject *parent)
    : QStyledItemDelegate(parent), d(new Private)
{

}

KisInputTypeDelegate::~KisInputTypeDelegate()
{
    delete d;

}

QWidget *KisInputTypeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
    KComboBox *combo = new KComboBox(parent);
    combo->addItems(QStringList()
                    << i18n("Key Combination")
                    << i18n("Mouse Button")
                    << i18n("Mouse Wheel")
                    //<< i18n("Gesture")
                   );
    combo->setCurrentIndex(0);

    return combo;
}

void KisInputTypeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    KComboBox *combo = qobject_cast<KComboBox *>(editor);
    Q_ASSERT(combo);

    combo->setCurrentIndex(index.data(Qt::EditRole).toUInt() - 1);
}

void KisInputTypeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    KComboBox *combo = qobject_cast<KComboBox *>(editor);
    Q_ASSERT(combo);
    model->setData(index, combo->currentIndex() + 1, Qt::EditRole);
}

void KisInputTypeDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}
