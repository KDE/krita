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
#include "kshortcutschemeshelper_p.h"
#include "kshortcutschemeseditor.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QDomDocument>
#include <QVBoxLayout>

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <kmessagebox.h>
#include <ksharedconfig.h>
#include "kxmlguiclient.h"
#include "kxmlguifactory.h"
#include "kactioncollection.h"

KisShortcutsDialog::KisShortcutsDialog(KisShortcutsEditor::ActionTypes types,
                                       KisShortcutsEditor::LetterShortcuts allowLetterShortcuts,
                                       QWidget *parent)
    : QDialog(parent)
    , d(new KisShortcutsDialogPrivate(this))
{

    d->m_shortcutsEditor = new KisShortcutsEditor(this, types, allowLetterShortcuts);



    /*  Construct & Connect UI  */
    setWindowTitle(i18n("Configure Shortcuts"));
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(d->m_shortcutsEditor);


    QHBoxLayout *bottomLayout = new QHBoxLayout;
    d->m_schemeEditor = new KShortcutSchemesEditor(this);
    connect(d->m_schemeEditor, SIGNAL(shortcutsSchemeChanged(QString)),
            this, SLOT(changeShortcutScheme(QString)));
    bottomLayout->addLayout(d->m_schemeEditor);

    QPushButton *printButton = new QPushButton;
    KGuiItem::assign(printButton, KStandardGuiItem::print());

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(printButton, QDialogButtonBox::ActionRole);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok |
                                  QDialogButtonBox::Cancel);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Ok),
                     KStandardGuiItem::ok());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel),
                     KStandardGuiItem::cancel());
    bottomLayout->addWidget(buttonBox);


    mainLayout->addLayout(bottomLayout);

    connect(printButton, SIGNAL(clicked()), d->m_shortcutsEditor, SLOT(printShortcuts()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(undo()));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    KConfigGroup group(KSharedConfig::openConfig(), "KisShortcutsDialog Settings");
    resize(group.readEntry("Dialog Size", sizeHint()));
}

KisShortcutsDialog::~KisShortcutsDialog()
{
    KConfigGroup group(KSharedConfig::openConfig(), "KisShortcutsDialog Settings");
    group.writeEntry("Dialog Size", size());
    delete d;
}

void KisShortcutsDialog::addCollection(KActionCollection *collection, const QString &title)
{
    d->m_shortcutsEditor->addCollection(collection, title);
    d->m_collections.insert(title, collection);
}

QList<KActionCollection *> KisShortcutsDialog::actionCollections() const
{
    return d->m_collections.values();
}

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

int KisShortcutsDialog::configure(KActionCollection *collection,
                                  KisShortcutsEditor::LetterShortcuts allowLetterShortcuts,
                                  QWidget *parent, bool saveSettings)
{
    KisShortcutsDialog dlg(KisShortcutsEditor::AllActions, allowLetterShortcuts, parent);
    dlg.d->m_shortcutsEditor->addCollection(collection);
    return dlg.configure(saveSettings);
}

void KisShortcutsDialog::allDefault()
{
    d->m_shortcutsEditor->allDefault();
}

void KisShortcutsDialog::importConfiguration(const QString &path)
{
    KConfig config(path);
    d->m_shortcutsEditor->importConfiguration(static_cast<KConfigBase *>(&config), true);
}

void KisShortcutsDialog::exportConfiguration(const QString &path) const
{
    KConfig config(path);
    d->m_shortcutsEditor->exportConfiguration(static_cast<KConfigBase *>(&config));
}

void KisShortcutsDialog::saveCustomShortcuts(const QString &path) const
{
  KConfig config(path);
  KConfigGroup cg(&config, QStringLiteral("Shortcuts"));
  d->m_shortcutsEditor->saveShortcuts(&cg);
}

void KisShortcutsDialog::loadCustomShortcuts(const QString &path)
{
    KConfig config(path);
    d->m_shortcutsEditor->importConfiguration(static_cast<KConfigBase *>(&config), false);
}

#include "moc_KisShortcutsDialog.cpp"
