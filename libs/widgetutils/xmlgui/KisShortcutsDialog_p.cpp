/*
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
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
