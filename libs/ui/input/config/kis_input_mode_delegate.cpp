/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_input_mode_delegate.h"
#include "../kis_abstract_input_action.h"

#include <kcombobox.h>
#include <klocalizedstring.h>

class KisInputModeDelegate::Private
{
public:
    Private() { }

    KisAbstractInputAction *action;
};

KisInputModeDelegate::KisInputModeDelegate(QObject *parent)
    : QStyledItemDelegate(parent), d(new Private)
{
}

KisInputModeDelegate::~KisInputModeDelegate()
{
    delete d;

}

QWidget *KisInputModeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
    KComboBox *combo = new KComboBox(parent);
    QStringList sorted = d->action->shortcutIndexes().keys();
    std::sort(sorted.begin(), sorted.end());
    combo->addItems(sorted);
    return combo;
}

void KisInputModeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    KComboBox *combo = qobject_cast<KComboBox *>(editor);
    Q_ASSERT(combo);

    int i = combo->findText(d->action->shortcutIndexes().key(index.data(Qt::EditRole).toUInt()));
    combo->setCurrentIndex(i);
}

void KisInputModeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    KComboBox *combo = qobject_cast<KComboBox *>(editor);
    Q_ASSERT(combo);

    int i = d->action->shortcutIndexes().value(combo->currentText());
    model->setData(index, i, Qt::EditRole);
}

void KisInputModeDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}

void KisInputModeDelegate::setAction(KisAbstractInputAction *action)
{
    d->action = action;
}
