/* This file is part of the KDE libraries SPDX-FileCopyrightText: 1998 Mark Donohoe <donohoe@kde.org>
    SPDX-FileCopyrightText: 1997 Nicolas Hadacek <hadacek@kde.org>
    SPDX-FileCopyrightText: 1998 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 2001 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KisShortcutsDialog_p.h"
#include "KisShortcutsEditor_p.h"

#include <QAction>
#include <QTreeWidgetItem>
#include <kis_debug.h>


KisShortcutsEditorItem::KisShortcutsEditorItem(QTreeWidgetItem *parent, QAction *action)
    : QTreeWidgetItem(parent, ActionItem)
    , m_action(action)
{
    // Filtering message requested by translators (scripting).
    m_id = m_action->objectName();
    m_actionNameInTable = i18nc("@item:intable Action name in shortcuts configuration",
                                "%1", KLocalizedString::removeAcceleratorMarker(m_action->text()));
    if (m_actionNameInTable.isEmpty()) {
        warnKrita << "Action without text!" << m_action->objectName();
        m_actionNameInTable = m_id;
    }

    m_collator.setNumericMode(true);
    m_collator.setCaseSensitivity(Qt::CaseSensitive);

    // qDebug() << "Adding new action" << m_id << "with shortcut" << keySequence(LocalPrimary).toString();
}

KisShortcutsEditorItem::~KisShortcutsEditorItem()
{
    delete m_oldLocalShortcut;
}

bool KisShortcutsEditorItem::isModified() const
{
    return m_oldLocalShortcut;
}

QVariant KisShortcutsEditorItem::data(int column, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        switch (column) {
        case Name:
            return m_actionNameInTable;
        case Id:
            return m_id;
        case LocalPrimary:
        case LocalAlternate:
            return QVariant::fromValue(keySequence(column));
        default:
            break;
        }
        break;
    case Qt::DecorationRole:
        if (column == Name) {
            return m_action->icon();
        } else {
            return QIcon();
        }
        break;
    case Qt::WhatsThisRole:
        return m_action->whatsThis();
    case Qt::ToolTipRole:
        // TODO: show command descriptions/tooltips in the shortcut editor
        return QVariant();
    case Qt::FontRole:
        if (column == Name && m_isNameBold) {
            QFont modifiedFont = treeWidget()->font();
            modifiedFont.setBold(true);
            return modifiedFont;
        }
        break;
    case KExtendableItemDelegate::ShowExtensionIndicatorRole:
        switch (column) {
        case Name:
            return false;
        case LocalPrimary:
        case LocalAlternate:
            return !m_action->property("isShortcutConfigurable").isValid()
                   || m_action->property("isShortcutConfigurable").toBool();
        default:
            return false;
        }
    //the following are custom roles, defined in this source file only
    case ShortcutRole:
        switch (column) {
        case LocalPrimary:
        case LocalAlternate:
            return QVariant::fromValue(keySequence(column));
        default:
            // Column not valid for this role
            Q_ASSERT(false);
            return QVariant();
        }

    case DefaultShortcutRole: {

        // Note: we are using the QMetaObject system to store this property.
        QList<QKeySequence> defaultShortcuts = m_action->property("defaultShortcuts").value<QList<QKeySequence> >();

        switch (column) {
        case LocalPrimary:
            return QVariant::fromValue(primarySequence(defaultShortcuts));
        case LocalAlternate:
            return QVariant::fromValue(alternateSequence(defaultShortcuts));
        default:
            // Column not valid for this role
            Q_ASSERT(false);
            return QVariant();
        }
    }
    case ObjectRole:
        return QVariant::fromValue((QObject *)m_action);

    default:
        break;
    }

    return QVariant();
}

bool KisShortcutsEditorItem::operator<(const QTreeWidgetItem &other) const
{
    const int column = treeWidget() ? treeWidget()->sortColumn() : 0;
    return m_collator.compare(text(column), other.text(column)) < 0;
}

QKeySequence KisShortcutsEditorItem::keySequence(uint column) const
{
    QList<QKeySequence> shortcuts = m_action->shortcuts();

    switch (column) {
    case LocalPrimary:
        return primarySequence(shortcuts);
    case LocalAlternate:
        return alternateSequence(shortcuts);
    default:
        return QKeySequence();
    }
}

void KisShortcutsEditorItem::setKeySequence(uint column, const QKeySequence &seq)
{
    QList<QKeySequence> ks;
    ks = m_action->shortcuts();
    if (!m_oldLocalShortcut) {
        m_oldLocalShortcut = new QList<QKeySequence>(ks);
    }

    if (column == LocalAlternate) {
        if (ks.isEmpty()) {
            ks << QKeySequence();
        }

        if (ks.size() <= 1) {
            ks << seq;
        } else {
            ks[1] = seq;
        }
    } else {
        if (ks.isEmpty()) {
            ks << seq;
        } else {
            ks[0] = seq;
        }
    }

    m_action->setShortcuts(ks);

    updateModified();
}

//our definition of modified is "modified since the chooser was shown".
void KisShortcutsEditorItem::updateModified()
{
    if (m_oldLocalShortcut && *m_oldLocalShortcut == m_action->shortcuts()) {
        delete m_oldLocalShortcut;
        m_oldLocalShortcut = 0;
    }
}

bool KisShortcutsEditorItem::isModified(uint column) const
{
    switch (column) {
    case Name:
        return false;
    case LocalPrimary:
    case LocalAlternate:
        if (!m_oldLocalShortcut) {
            return false;
        }
        if (column == LocalPrimary) {
            return primarySequence(*m_oldLocalShortcut) != primarySequence(m_action->shortcuts());
        } else {
            return alternateSequence(*m_oldLocalShortcut) != alternateSequence(m_action->shortcuts());
        }
    default:
        return false;
    }
}

void KisShortcutsEditorItem::undo()
{
    //dbgKrita << "Undoing changes for " << data(Name, Qt::DisplayRole).toString();

    if (m_oldLocalShortcut) {
        // We only ever reset the active Shortcut
        m_action->setShortcuts(*m_oldLocalShortcut);
    }

    updateModified();
}

void KisShortcutsEditorItem::commit()
{
    if (m_oldLocalShortcut) { // || m_oldShapeGesture || m_oldRockerGesture) {
       dbgUI << "Committing changes for " << data(Name, Qt::DisplayRole).toString();
    }

    delete m_oldLocalShortcut;
    m_oldLocalShortcut = 0;
}
