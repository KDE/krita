/* This file is part of the KDE libraries
    Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1997 Nicolas Hadacek <hadacek@kde.org>
    Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1998 Matthias Ettrich <ettrich@kde.org>
    Copyright (C) 1999 Espen Sand <espen@kde.org>
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
    Copyright (C) 2007 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>
    Copyright (C) 2008 Alexander Dymo <adymo@kdevelop.org>
    Copyright (C) 2009 Chani Armitage <chani@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "KisShortcutsDialog.h"
#include "KisShortcutsDialog_p.h"
// #include "kshortcutschemeshelper_p.h" diff to KF5 version: seems not needed, thankfully

#include <QApplication>
#include <QDialogButtonBox>
#include <QDomDocument>

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <kmessagebox.h>
#include <ksharedconfig.h>
#include "kxmlguiclient.h"
#include "kxmlguifactory.h"
#include "kactioncollection.h"

KisShortcutsDialog::KisShortcutsDialog(KShortcutsEditor::ActionTypes types,
                                       KShortcutsEditor::LetterShortcuts allowLetterShortcuts,
                                       QWidget *parent)
    : QDialog(parent)
    , d(new KisShortcutsDialogPrivate(this))
{
    /*  Set up UI layout  */
    setWindowTitle(i18n("Configure Shortcuts"));
    setModal(true);

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    d->m_keyChooser = new KShortcutsEditor(this, types, allowLetterShortcuts);
    layout->addWidget(d->m_keyChooser);

#ifndef NOSCHEMESPLEASEFORKRITA
    d->m_schemeEditor = new KShortcutSchemesEditor(this);
    connect(d->m_schemeEditor, SIGNAL(shortcutsSchemeChanged(QString)),
            this, SLOT(changeShortcutScheme(QString)));
    d->m_schemeEditor->hide();
    layout->addWidget(d->m_schemeEditor);
    d->m_detailsButton = new QPushButton;
    d->m_detailsButton->setText(i18n("&Details") + QStringLiteral(" >>"));
#endif

    QPushButton *printButton = new QPushButton;
    KGuiItem::assign(printButton, KStandardGuiItem::print());

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
#ifndef NOSCHEMESPLEASEFORKRITA
    buttonBox->addButton(d->m_detailsButton, QDialogButtonBox::ActionRole);
#endif
    buttonBox->addButton(printButton, QDialogButtonBox::ActionRole);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Ok), KStandardGuiItem::ok());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::RestoreDefaults), KStandardGuiItem::defaults());
    layout->addWidget(buttonBox);

    connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), SIGNAL(clicked()),
            d->m_keyChooser, SLOT(allDefault()));
    connect(printButton, SIGNAL(clicked()), d->m_keyChooser, SLOT(printShortcuts()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(undoChanges()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

#ifndef NOSCHEMESPLEASEFORKRITA
    // connect(d->m_detailsButton, &clicked(), this, &toggleDetails());
#endif

    KConfigGroup group(KSharedConfig::openConfig(), "KisShortcutsDialog Settings");
    resize(group.readEntry("Dialog Size", sizeHint()));
}

KisShortcutsDialog::~KisShortcutsDialog()
{
    KConfigGroup group(KSharedConfig::openConfig(), "KisShortcutsDialog Settings");
    group.writeEntry("Dialog Size", size(), KConfigGroup::Persistent | KConfigGroup::Global);
    delete d;
}

void KisShortcutsDialog::addCollection(KActionCollection *collection, const QString &title)
{
    d->m_keyChooser->addCollection(collection, title);
    d->m_collections << collection;
}

QList<KActionCollection *> KisShortcutsDialog::actionCollections() const
{
    return d->m_collections;
}

//FIXME should there be a setSaveSettings method?
bool KisShortcutsDialog::configure(bool saveSettings)
{
    d->m_saveSettings = saveSettings;
    if (isModal()) {
        int retcode = exec();
        return retcode;
    } else {
        show();
        return false;
    }
}

void KisShortcutsDialog::accept()
{
    if (d->m_saveSettings) {
        d->save();
    }
    QDialog::accept();
}

QSize KisShortcutsDialog::sizeHint() const
{
    return QSize(600, 480);
}

int KisShortcutsDialog::configure(KActionCollection *collection, KShortcutsEditor::LetterShortcuts allowLetterShortcuts,
                                QWidget *parent, bool saveSettings)
{
    //qDebug(125) << "KisShortcutsDialog::configureKeys( KActionCollection*, " << saveSettings << " )";
    KisShortcutsDialog dlg(KShortcutsEditor::AllActions, allowLetterShortcuts, parent);
    dlg.d->m_keyChooser->addCollection(collection);
    return dlg.configure(saveSettings);
}

#include "moc_KisShortcutsDialog.cpp"
