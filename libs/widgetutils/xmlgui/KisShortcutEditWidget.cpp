/* This file is part of the KDE libraries SPDX-FileCopyrightText: 1998 Mark Donohoe <donohoe@kde.org>
    SPDX-FileCopyrightText: 1997 Nicolas Hadacek <hadacek@kde.org>
    SPDX-FileCopyrightText: 1998 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 2001 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KisShortcutsDialog_p.h"

#include <QPainter>
#include <QPen>
#include <QGridLayout>
#include <QRadioButton>
#include <QLabel>
#include <QApplication>

#include <klocalizedstring.h>
//#include <kglobalaccel.h>

#include "kkeysequencewidget.h"

void ShortcutEditWidget::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
    QPainter p(this);
    QPen pen(QPalette().highlight().color());
    pen.setWidth(6);
    p.setPen(pen);
    p.drawLine(0, 0, width(), 0);
    if (qApp->isLeftToRight()) {
        p.drawLine(0, 0, 0, height());
    } else {
        p.drawLine(width(), 0, width(), height());
    }
}

ShortcutEditWidget::ShortcutEditWidget(QWidget *viewport, const QKeySequence &defaultSeq,
                                       const QKeySequence &activeSeq, bool allowLetterShortcuts)
    : QWidget(viewport),
      m_defaultKeySequence(defaultSeq),
      m_isUpdating(false),
      m_action(0)
{
    QGridLayout *layout = new QGridLayout(this);

    m_defaultRadio = new QRadioButton(i18n("Default:"), this);
    m_defaultLabel = new QLabel(i18nc("No shortcut defined", "None"), this);
    QString defaultText = defaultSeq.toString(QKeySequence::NativeText);
    if (defaultText.isEmpty()) {
        defaultText = i18nc("No shortcut defined", "None");
    }
    m_defaultLabel->setText(defaultText);

    m_customRadio = new QRadioButton(i18n("Custom:"), this);
    m_customEditor = new KKeySequenceWidget(this);
    m_customEditor->setModifierlessAllowed(allowLetterShortcuts);

    layout->addWidget(m_defaultRadio, 0, 0);
    layout->addWidget(m_defaultLabel, 0, 1);
    layout->addWidget(m_customRadio, 1, 0);
    layout->addWidget(m_customEditor, 1, 1);
    layout->setColumnStretch(2, 1);

    setKeySequence(activeSeq);

    connect(m_defaultRadio, SIGNAL(toggled(bool)),
            this, SLOT(defaultToggled(bool)));
    connect(m_customEditor, SIGNAL(keySequenceChanged(QKeySequence)),
            this, SLOT(setCustom(QKeySequence)));
    connect(m_customEditor, SIGNAL(stealShortcut(QKeySequence,QAction*)),
            this, SIGNAL(stealShortcut(QKeySequence,QAction*)));
}

KKeySequenceWidget::ShortcutTypes ShortcutEditWidget::checkForConflictsAgainst() const
{
    return m_customEditor->checkForConflictsAgainst();
}

//slot
void ShortcutEditWidget::defaultToggled(bool checked)
{
    if (m_isUpdating) {
        return;
    }

    m_isUpdating = true;
    if (checked) {
        // The default key sequence should be activated. We check first if this
        // is possible.
        if (m_customEditor->isKeySequenceAvailable(m_defaultKeySequence)) {
            // Clear the customs widget
            m_customEditor->clearKeySequence();
            emit keySequenceChanged(m_defaultKeySequence);
        } else {
            // We tried to switch to the default key sequence and failed.
            // Go back.
            m_customRadio->setChecked(true);
        }
    } else {
        // The empty key sequence is always valid
        emit keySequenceChanged(QKeySequence());
    }
    m_isUpdating = false;
}

void ShortcutEditWidget::setCheckActionCollections(
    const QList<KActionCollection *> checkActionCollections)
{
    // We just forward them to out KKeySequenceWidget.
    m_customEditor->setCheckActionCollections(checkActionCollections);
}

void ShortcutEditWidget::setCheckForConflictsAgainst(KKeySequenceWidget::ShortcutTypes types)
{
    m_customEditor->setCheckForConflictsAgainst(types);
}

void ShortcutEditWidget::setComponentName(const QString componentName)
{
    m_customEditor->setComponentName(componentName);
}

void ShortcutEditWidget::setMultiKeyShortcutsAllowed(bool allowed)
{
    // We just forward them to out KKeySequenceWidget.
    m_customEditor->setMultiKeyShortcutsAllowed(allowed);
}

bool ShortcutEditWidget::multiKeyShortcutsAllowed() const
{
    return m_customEditor->multiKeyShortcutsAllowed();
}

void ShortcutEditWidget::setAction(QObject *action)
{
    m_action = action;
}

//slot
void ShortcutEditWidget::setCustom(const QKeySequence &seq)
{
    if (m_isUpdating) {
        return;
    }

    // seq is a const reference to a private variable of KKeySequenceWidget.
    // Somewhere below we possible change that one. But we want to emit seq
    // whatever happens. So we make a copy.
    QKeySequence original = seq;

    m_isUpdating = true;

    // Check if the user typed in the default sequence into the custom field.
    // We do this by calling setKeySequence which will do the right thing.
    setKeySequence(original);

    emit keySequenceChanged(original);
    m_isUpdating = false;
}

void ShortcutEditWidget::setKeySequence(const QKeySequence &activeSeq)
{
    if (activeSeq.toString(QKeySequence::NativeText) == m_defaultKeySequence.toString(QKeySequence::NativeText)) {
        m_defaultRadio->setChecked(true);
        m_customEditor->clearKeySequence();
    } else {
        m_customRadio->setChecked(true);
        // m_customEditor->setKeySequence does some stuff we only want to
        // execute when the sequence really changes.
        if (activeSeq != m_customEditor->keySequence()) {
            m_customEditor->setKeySequence(activeSeq);
        }
    }
}

