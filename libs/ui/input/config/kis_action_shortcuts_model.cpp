/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_action_shortcuts_model.h"

#include <kis_debug.h>

#include <KLocalizedString>
#include <QMetaClassInfo>
#include <QKeySequence>
#include <QMessageBox>

#include "kis_icon_utils.h"

#include "input/kis_abstract_input_action.h"
#include "input/kis_input_profile.h"
#include "input/kis_input_profile_manager.h"
#include "input/kis_shortcut_configuration.h"

class KisActionShortcutsModel::Private
{
public:
    Private() : action(0), profile(0), temporaryShortcut(0) { }

    int shortcutModeCount(uint mode);

    KisAbstractInputAction *action;
    KisInputProfile *profile;
    QList<KisShortcutConfiguration *> shortcuts;

    KisShortcutConfiguration *temporaryShortcut;
};

KisActionShortcutsModel::KisActionShortcutsModel(QObject *parent)
    : QAbstractListModel(parent), d(new Private)
{
    connect(KisInputProfileManager::instance(), SIGNAL(currentProfileChanged()), SLOT(currentProfileChanged()));
}

KisActionShortcutsModel::~KisActionShortcutsModel()
{
}

QVariant KisActionShortcutsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() == d->shortcuts.count() && role == Qt::DisplayRole) {
        if (index.column() == 0) {
            return i18n("Add shortcut...");
        }
        else {
            return QVariant();
        }
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:
            switch (d->shortcuts.at(index.row())->type()) {
            case KisShortcutConfiguration::KeyCombinationType:
                return i18nc("Shortcut type", "Key Combination");

            case KisShortcutConfiguration::MouseButtonType:
                return i18nc("Shortcut type", "Mouse Button");

            case KisShortcutConfiguration::MouseWheelType:
                return i18nc("Shortcut type", "Mouse Wheel");

            case KisShortcutConfiguration::GestureType:
                return i18nc("Shortcut type", "Gesture");

            default:
                return i18n("Unknown Input");
            }

            break;

        case 1: {
            KisShortcutConfiguration *s = d->shortcuts.at(index.row());
            QString output;

            switch (s->type()) {
            case KisShortcutConfiguration::KeyCombinationType:
                output = KisShortcutConfiguration::keysToText(s->keys());
                break;

            case KisShortcutConfiguration::MouseButtonType:
                output = KisShortcutConfiguration::buttonsInputToText(
                    s->keys(), s->buttons());
                break;

            case KisShortcutConfiguration::MouseWheelType:
                output = KisShortcutConfiguration::wheelInputToText(
                    s->keys(), s->wheel());
                break;

            default:
                break;
            }

            return output;
        }

        case 2:
            return d->action->shortcutIndexes().key(d->shortcuts.at(index.row())->mode());

        case 3:
            return KisIconUtils::loadIcon("edit-delete");

        default:
            break;
        }
    }
    else if (role == Qt::EditRole) {
        KisShortcutConfiguration *s;

        if (index.row() == d->shortcuts.count()) {
            if (!d->temporaryShortcut) {
                d->temporaryShortcut = new KisShortcutConfiguration;
            }

            s = d->temporaryShortcut;
        }
        else {
            s = d->shortcuts.at(index.row());
        }

        switch (index.column()) {
        case 0:
            return s->type();

        case 1:
            return QVariant::fromValue(s);

        case 2:
            return s->mode();

        default:
            break;
        }
    }

    return QVariant();
}

int KisActionShortcutsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->shortcuts.count() + 1;
}

int KisActionShortcutsModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}

QVariant KisActionShortcutsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (section) {
    case 0:
        return i18nc("Type of shortcut", "Type");

    case 1:
        return i18nc("Input for shortcut", "Input");

    case 2:
        return i18nc("Action to trigger with shortcut", "Action");

    default:
        break;
    }

    return QVariant();
}

Qt::ItemFlags KisActionShortcutsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::ItemIsEnabled;
    }

    if (index.row() == d->shortcuts.count() && index.column() != 0) {
        return Qt::ItemIsEnabled;
    }

    if (index.row() >= d->shortcuts.count()) {
        return Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }

    KisShortcutConfiguration* config = d->shortcuts.at(index.row());
    if (index.column() == 2 && d->action->isShortcutRequired(config->mode()) && d->shortcutModeCount(config->mode()) < 2) {
        return Qt::ItemIsSelectable;
    }

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool KisActionShortcutsModel::canRemoveRow(int row) const
{
    if (row >= d->shortcuts.size()) {
        return false;
    }
    KisShortcutConfiguration* config = d->shortcuts.at(row);
    return !(d->action->isShortcutRequired(config->mode()) && d->shortcutModeCount(config->mode()) < 2);
}

bool KisActionShortcutsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole) {
        return false;
    }

    if (index.row() == d->shortcuts.count()) {
        if (!d->temporaryShortcut || (index.column() == 0 && value.toUInt() == 0)) {
            return false;
        }

        beginInsertRows(QModelIndex(), d->shortcuts.count(), d->shortcuts.count());
        d->temporaryShortcut->setAction(d->action);
        d->profile->addShortcut(d->temporaryShortcut);
        d->shortcuts.append(d->temporaryShortcut);
        d->temporaryShortcut = 0;
        endInsertRows();
    }

    switch (index.column()) {
    case 0:
        d->shortcuts.at(index.row())->setType(static_cast<KisShortcutConfiguration::ShortcutType>(value.toUInt()));
        break;

    case 1: {
        KisShortcutConfiguration *newData = value.value<KisShortcutConfiguration *>();
        KisShortcutConfiguration *oldData = d->shortcuts.at(index.row());

        if (newData == oldData)
            return true;

        oldData->setKeys(newData->keys());
        oldData->setButtons(newData->buttons());
        oldData->setWheel(newData->wheel());
        oldData->setGesture(newData->gesture());

        break;
    }

    case 2:
        d->shortcuts.at(index.row())->setMode(value.toUInt());
        break;
    }

    return true;
}

KisAbstractInputAction *KisActionShortcutsModel::action() const
{
    return d->action;
}

void KisActionShortcutsModel::setAction(KisAbstractInputAction *action)
{
    if (action != d->action) {
        if (d->action) {
            beginRemoveRows(QModelIndex(), 0, d->shortcuts.count() - 1);
            endRemoveRows();
        }

        d->action = action;

        if (d->action && d->profile) {
            d->shortcuts = d->profile->shortcutsForAction(d->action);
            beginInsertRows(QModelIndex(), 0, d->shortcuts.count() - 1);
            endInsertRows();
        }
    }
}

KisInputProfile *KisActionShortcutsModel::profile() const
{
    return d->profile;
}

void KisActionShortcutsModel::setProfile(KisInputProfile *profile)
{
    if (profile != d->profile) {
        if (d->profile) {
            beginRemoveRows(QModelIndex(), 0, d->shortcuts.count() - 1);
            endRemoveRows();
        }

        d->profile = profile;

        if (d->action && d->profile) {
            d->shortcuts = d->profile->shortcutsForAction(d->action);
            beginInsertRows(QModelIndex(), 0, d->shortcuts.count() - 1);
            endInsertRows();
        }
    }
}

void KisActionShortcutsModel::currentProfileChanged()
{
    setProfile(KisInputProfileManager::instance()->currentProfile());
}

bool KisActionShortcutsModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || row >= d->shortcuts.count() || count == 0) {
        return false;
    }

    beginRemoveRows(parent, row, row + count - 1);

    for (int i = row; i < d->shortcuts.count() && count > 0; ++i, count--) {
        KisShortcutConfiguration *s = d->shortcuts.at(i);

        if (!canRemoveRow(i)) {
            QMessageBox shortcutMessage;
            shortcutMessage.setText(i18n("Deleting last shortcut for this action!"));
            shortcutMessage.setInformativeText(i18n("It is not allowed to erase some default shortcuts. Modify it instead."));
            shortcutMessage.exec();
            continue;
        }

        d->profile->removeShortcut(s);
        d->shortcuts.removeOne(s);
        delete s;
    }

    endRemoveRows();

    return true;
}

int KisActionShortcutsModel::Private::shortcutModeCount(uint mode)
{
    int count = 0;
    Q_FOREACH (KisShortcutConfiguration* s, shortcuts) {
        if(s->mode() == mode) {
            count++;
        }
    }

    return count;
}
