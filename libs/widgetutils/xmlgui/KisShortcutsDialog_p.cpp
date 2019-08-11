/*
 *  Copyright (c) 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "KisShortcutsDialog_p.h"
#include "kshortcutschemeshelper_p.h"
#include "kxmlguiclient.h"
#include <QDomDocument>
#include "kactioncollection.h"
#include "kxmlguifactory.h"
#include <QAction>
#include <QApplication>
#include <QDebug>
#include "kis_action_registry.h"
#include <KSharedConfig>
#include <KConfigGroup>



QKeySequence primarySequence(const QList<QKeySequence> &sequences)
{
    return sequences.isEmpty() ? QKeySequence() : sequences.at(0);
}

QKeySequence alternateSequence(const QList<QKeySequence> &sequences)
{
    return sequences.size() <= 1 ? QKeySequence() : sequences.at(1);
}


KisShortcutsDialog::KisShortcutsDialogPrivate::KisShortcutsDialogPrivate(KisShortcutsDialog *q)
    : q(q)
{ }

void KisShortcutsDialog::KisShortcutsDialogPrivate::changeShortcutScheme(const QString &schemeName)
{
    // KTreeWidgetSearchLine is unhappy if the contents of the tree change
    m_shortcutsEditor->clearSearch();

    QString dialogText = i18n("The current shortcut scheme is modified. Save before switching to the new one?");
    if (m_shortcutsEditor->isModified() &&
        KMessageBox::questionYesNo( q,dialogText ) == KMessageBox::Yes) {
        m_shortcutsEditor->save();
    } else {
        m_shortcutsEditor->undo();
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    m_shortcutsEditor->clearCollections();

    KConfigGroup cg = KSharedConfig::openConfig()->group("Shortcut Schemes");
    cg.writeEntry("Current Scheme", schemeName);
    KisActionRegistry::instance()->loadShortcutScheme(schemeName);


    // Update actions themselves, and re-add to dialog box to refresh
    auto it = m_collections.constBegin();
    while (it != m_collections.constEnd()) {
        it.value()->updateShortcuts();
        // TODO: BAD
        m_shortcutsEditor->addCollection(it.value(), it.key());
        it++;
    }

    QApplication::restoreOverrideCursor();
}

void KisShortcutsDialog::KisShortcutsDialogPrivate::undo()
{
    m_shortcutsEditor->undo();
}

void KisShortcutsDialog::KisShortcutsDialogPrivate::save()
{
    m_shortcutsEditor->save();
}

#include "moc_KisShortcutsDialog_p.cpp"
