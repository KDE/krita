/* This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1998 Mark Donohoe <donohoe@kde.org>
    SPDX-FileCopyrightText: 1997 Nicolas Hadacek <hadacek@kde.org>
    SPDX-FileCopyrightText: 1998 Mark Donohoe <donohoe@kde.org>
    SPDX-FileCopyrightText: 1998 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 1999 Espen Sand <espen@kde.org>
    SPDX-FileCopyrightText: 2001 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>
    SPDX-FileCopyrightText: 2008 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    : QWidget(parent)
    , d(new KisShortcutsDialogPrivate(this))
{

    d->m_shortcutsEditor = new KisShortcutsEditor(this, types, allowLetterShortcuts);

    /*  Construct & Connect UI  */
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
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
    bottomLayout->addWidget(buttonBox);

    mainLayout->addLayout(bottomLayout);

    connect(printButton, SIGNAL(clicked()), d->m_shortcutsEditor, SLOT(printShortcuts()));

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


void KisShortcutsDialog::save()
{
    d->save();
}

QList<KActionCollection *> KisShortcutsDialog::actionCollections() const
{
    return d->m_collections.values();
}

QSize KisShortcutsDialog::sizeHint() const
{
    return QSize(600, 480);
}

void KisShortcutsDialog::allDefault()
{
    d->m_shortcutsEditor->allDefault();
}

void KisShortcutsDialog::undo()
{
    d->undo();
}

void KisShortcutsDialog::importConfiguration(const QString &path)
{
    auto config = KSharedConfig::openConfig(path);
    d->m_shortcutsEditor->importConfiguration(config.data(), true);
}

void KisShortcutsDialog::exportConfiguration(const QString &path) const
{
    auto config = KSharedConfig::openConfig(path);
    d->m_shortcutsEditor->exportConfiguration(config.data());
}

void KisShortcutsDialog::saveCustomShortcuts(const QString &path) const
{
    auto cg = KSharedConfig::openConfig(path)->group(QStringLiteral("Shortcuts"));
    d->m_shortcutsEditor->saveShortcuts(&cg);
    d->m_shortcutsEditor->commit();
}

void KisShortcutsDialog::loadCustomShortcuts(const QString &path)
{
    auto config = KSharedConfig::openConfig(path);
    d->m_shortcutsEditor->importConfiguration(config.data(), false);
}

#include "moc_KisShortcutsDialog.cpp"
