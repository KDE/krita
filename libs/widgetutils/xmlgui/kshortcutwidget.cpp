/* This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kshortcutwidget.h"
#include "ui_kshortcutwidget.h"

class KShortcutWidgetPrivate
{
public:
    KShortcutWidgetPrivate(KShortcutWidget *q) : q(q) {}

//private slots
    void priKeySequenceChanged(const QKeySequence &);
    void altKeySequenceChanged(const QKeySequence &);

//members
    KShortcutWidget *const q;
    Ui::KShortcutWidget ui;
    QList<QKeySequence> cut;
    bool holdChangedSignal;
};

KShortcutWidget::KShortcutWidget(QWidget *parent)
    : QWidget(parent),
      d(new KShortcutWidgetPrivate(this))
{
    d->holdChangedSignal = false;
    d->ui.setupUi(this);
    connect(d->ui.priEditor, SIGNAL(keySequenceChanged(QKeySequence)),
            this, SLOT(priKeySequenceChanged(QKeySequence)));
    connect(d->ui.altEditor, SIGNAL(keySequenceChanged(QKeySequence)),
            this, SLOT(altKeySequenceChanged(QKeySequence)));
}

KShortcutWidget::~KShortcutWidget()
{
    delete d;
}

void KShortcutWidget::setModifierlessAllowed(bool allow)
{
    d->ui.priEditor->setModifierlessAllowed(allow);
    d->ui.altEditor->setModifierlessAllowed(allow);
}

bool KShortcutWidget::isModifierlessAllowed()
{
    return d->ui.priEditor->isModifierlessAllowed();
}

void KShortcutWidget::setClearButtonsShown(bool show)
{
    d->ui.priEditor->setClearButtonShown(show);
    d->ui.altEditor->setClearButtonShown(show);
}

QList<QKeySequence> KShortcutWidget::shortcut() const
{
    QList<QKeySequence> ret;
    ret << d->ui.priEditor->keySequence()
        << d->ui.altEditor->keySequence();
    return ret;
}


void KShortcutWidget::setCheckActionCollections(const QList<KActionCollection *> &actionCollections)
{
    d->ui.priEditor->setCheckActionCollections(actionCollections);
    d->ui.altEditor->setCheckActionCollections(actionCollections);
}

//slot
void KShortcutWidget::applyStealShortcut()
{
    d->ui.priEditor->applyStealShortcut();
    d->ui.altEditor->applyStealShortcut();
}

//slot
void KShortcutWidget::setShortcut(const QList<QKeySequence> &newSc)
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

    emit shortcutChanged(d->cut);
}

//slot
void KShortcutWidget::clearShortcut()
{
    setShortcut(QList<QKeySequence>());
}

//private slot
void KShortcutWidgetPrivate::priKeySequenceChanged(const QKeySequence &seq)
{
    if (cut.isEmpty()) {
        cut << seq;
    } else {
        cut[0] = seq;
    }

    if (!holdChangedSignal) {
        emit q->shortcutChanged(cut);
    }
}

//private slot
void KShortcutWidgetPrivate::altKeySequenceChanged(const QKeySequence &seq)
{
    if (cut.size() <= 1) {
        cut << seq;
    } else {
        cut[1] = seq;
    }

    if (!holdChangedSignal) {
        emit q->shortcutChanged(cut);
    }
}

#include "moc_kshortcutwidget.cpp"
