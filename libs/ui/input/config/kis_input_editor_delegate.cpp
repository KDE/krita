/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_input_editor_delegate.h"

#include <QApplication>
#include <QPushButton>

#include <KLocalizedString>

#include "input/kis_input_profile.h"
#include "input/kis_shortcut_configuration.h"
#include "kis_input_button.h"
#include "kis_mouse_input_editor.h"
#include "kis_wheel_input_editor.h"
#include "kis_key_input_editor.h"

class KisInputEditorDelegate::Private
{
public:
    Private() { }
};

KisInputEditorDelegate::KisInputEditorDelegate(QObject *parent)
    : QStyledItemDelegate(parent), d(new Private())
{
}

KisInputEditorDelegate::~KisInputEditorDelegate()
{
    delete d;

}

QWidget *KisInputEditorDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    QPushButton *editor = 0;
    KisShortcutConfiguration *s = index.data(Qt::EditRole).value<KisShortcutConfiguration *>();

    switch (s->type()) {
    case KisShortcutConfiguration::KeyCombinationType:
        editor = new KisKeyInputEditor(parent);
        break;

    case KisShortcutConfiguration::MouseButtonType:
        editor = new KisMouseInputEditor(parent);
        break;

    case KisShortcutConfiguration::MouseWheelType:
        editor = new KisWheelInputEditor(parent);
        break;

    default:
        break;
    }

    return editor;
}

void KisInputEditorDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    KisShortcutConfiguration *s = index.model()->data(index, Qt::EditRole).value<KisShortcutConfiguration *>();

    switch (s->type()) {
    case KisShortcutConfiguration::KeyCombinationType: {
        KisKeyInputEditor *e = qobject_cast<KisKeyInputEditor *>(editor);
        e->setKeys(s->keys());
        break;
    }

    case KisShortcutConfiguration::MouseButtonType: {
        KisMouseInputEditor *e = qobject_cast<KisMouseInputEditor *>(editor);
        e->setKeys(s->keys());
        e->setButtons(s->buttons());
        break;
    }

    case KisShortcutConfiguration::MouseWheelType: {
        KisWheelInputEditor *e = qobject_cast<KisWheelInputEditor *>(editor);
        e->setKeys(s->keys());
        e->setWheel(s->wheel());
        break;
    }

    default:
        break;
    }
}

void KisInputEditorDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    KisShortcutConfiguration *s = model->data(index, Qt::EditRole).value<KisShortcutConfiguration *>();

    switch (s->type()) {
    case KisShortcutConfiguration::KeyCombinationType: {
        KisKeyInputEditor *e = qobject_cast<KisKeyInputEditor *>(editor);
        s->setKeys(e->keys());
        break;
    }

    case KisShortcutConfiguration::MouseButtonType: {
        KisMouseInputEditor *e = qobject_cast<KisMouseInputEditor *>(editor);
        s->setKeys(e->keys());
        s->setButtons(e->buttons());
        break;
    }

    case KisShortcutConfiguration::MouseWheelType: {
        KisWheelInputEditor *e = qobject_cast<KisWheelInputEditor *>(editor);
        s->setKeys(e->keys());
        s->setWheel(e->wheel());
        break;
    }
    break;

    default:
        break;
    }

    model->setData(index, QVariant::fromValue(s), Qt::EditRole);
}

void KisInputEditorDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}

QSize KisInputEditorDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index) + QSize(6, 6);
}
