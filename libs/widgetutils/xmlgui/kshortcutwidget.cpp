/* This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kshortcutwidget.h"
#include "ui_kshortcutwidget.h"

class KisKShortcutWidgetPrivate
{
public:
    KisKShortcutWidgetPrivate(KisKShortcutWidget *q) : q(q) {}

//private slots
    void priKeySequenceChanged(const QKeySequence &);
    void altKeySequenceChanged(const QKeySequence &);

//members
    KisKShortcutWidget *const q;
    Ui::KisKShortcutWidget ui;
    QList<QKeySequence> cut;
    bool holdChangedSignal {false};
};

KisKShortcutWidget::KisKShortcutWidget(QWidget *parent)
    : QWidget(parent),
      d(new KisKShortcutWidgetPrivate(this))
{
    d->holdChangedSignal = false;
    d->ui.setupUi(this);
    connect(d->ui.priEditor, SIGNAL(keySequenceChanged(QKeySequence)),
            this, SLOT(priKeySequenceChanged(QKeySequence)));
    connect(d->ui.altEditor, SIGNAL(keySequenceChanged(QKeySequence)),
            this, SLOT(altKeySequenceChanged(QKeySequence)));
}

KisKShortcutWidget::~KisKShortcutWidget()
{
    delete d;
}

void KisKShortcutWidget::setModifierlessAllowed(bool allow)
{
    d->ui.priEditor->setModifierlessAllowed(allow);
    d->ui.altEditor->setModifierlessAllowed(allow);
}

bool KisKShortcutWidget::isModifierlessAllowed()
{
    return d->ui.priEditor->isModifierlessAllowed();
}

void KisKShortcutWidget::setClearButtonsShown(bool show)
{
    d->ui.priEditor->setClearButtonShown(show);
    d->ui.altEditor->setClearButtonShown(show);
}

QList<QKeySequence> KisKShortcutWidget::shortcut() const
{
    QList<QKeySequence> ret;
    ret << d->ui.priEditor->keySequence()
        << d->ui.altEditor->keySequence();
    return ret;
}


void KisKShortcutWidget::setCheckActionCollections(const QList<KisKActionCollection *> &actionCollections)
{
    d->ui.priEditor->setCheckActionCollections(actionCollections);
    d->ui.altEditor->setCheckActionCollections(actionCollections);
}

//slot
void KisKShortcutWidget::applyStealShortcut()
{
    d->ui.priEditor->applyStealShortcut();
    d->ui.altEditor->applyStealShortcut();
}

//slot
void KisKShortcutWidget::setShortcut(const QList<QKeySequence> &newSc)
{
    if (newSc == d->cut) {
        return;
    }

    d->holdChangedSignal = true;

    if (!newSc.isEmpty()) {
        d->ui.priEditor->setKeySequence(newSc.first());
    }

    if (newSc.size() > 1) {
        d->ui.altEditor->setKeySequence(newSc.at(1));
    }

    d->holdChangedSignal = false;

    Q_EMIT shortcutChanged(d->cut);
}

//slot
void KisKShortcutWidget::clearShortcut()
{
    setShortcut(QList<QKeySequence>());
}

//private slot
void KisKShortcutWidgetPrivate::priKeySequenceChanged(const QKeySequence &seq)
{
    if (cut.isEmpty()) {
        cut << seq;
    } else {
        cut[0] = seq;
    }

    if (!holdChangedSignal) {
        Q_EMIT q->shortcutChanged(cut);
    }
}

//private slot
void KisKShortcutWidgetPrivate::altKeySequenceChanged(const QKeySequence &seq)
{
    if (cut.size() <= 1) {
        cut << seq;
    } else {
        cut[1] = seq;
    }

    if (!holdChangedSignal) {
        Q_EMIT q->shortcutChanged(cut);
    }
}

#include "moc_kshortcutwidget.cpp"
