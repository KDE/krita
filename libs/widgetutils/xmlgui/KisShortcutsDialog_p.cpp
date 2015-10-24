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
#include "kxmlguiclient.h"
#include <QDomDocument>
#include "kactioncollection.h"
#include "kxmlguifactory.h"
#include <QAction>
#include <QApplication>




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

void KisShortcutsDialog::KisShortcutsDialogPrivate::changeShortcutScheme(const QString &scheme)
{
    QString dialogText = i18n("The current shortcut scheme is modified. Save before switching to the new one?");
    if (m_shortcutsEditor->isModified() &&
        KMessageBox::questionYesNo( q,dialogText ) == KMessageBox::Yes) {
        m_shortcutsEditor->save();
    } else {
        m_shortcutsEditor->undo();
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    m_shortcutsEditor->clearCollections();

    foreach (KActionCollection *collection, m_collections) {
        // passing an empty stream forces the clients to reread the XML
        KXMLGUIClient *client = const_cast<KXMLGUIClient *>(collection->parentGUIClient());
        if (client) {
            client->setXMLGUIBuildDocument(QDomDocument());
        }
    }

    //get xmlguifactory
    if (!m_collections.isEmpty()) {
        const KXMLGUIClient *client = m_collections.first()->parentGUIClient();
        if (client) {
            KXMLGUIFactory *factory = client->factory();
            if (factory) {
                factory->changeShortcutScheme(scheme);
            }
        }
    }

    foreach (KActionCollection *collection, m_collections) {
        m_shortcutsEditor->addCollection(collection);
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
    emit q->saved();
};


#include "moc_KisShortcutsDialog_p.cpp"
