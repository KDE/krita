/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
