/* This file is part of the KDE libraries Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1997 Nicolas Hadacek <hadacek@kde.org>
    Copyright (C) 1998 Matthias Ettrich <ettrich@kde.org>
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
    Copyright (C) 2007 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>

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

#include "KisShortcutsDialog_p.h"

#include <QPainter>
#include <QPen>
#include <QGridLayout>
#include <QRadioButton>
#include <QLabel>

#include <klocalizedstring.h>
#include <kglobalaccel.h>

#include "kkeysequencewidget.h"

void TabConnectedWidget::paintEvent(QPaintEvent *e)
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
    : TabConnectedWidget(viewport),
      m_defaultKeySequence(defaultSeq),
      m_isUpdating(false),
      m_action(Q_NULLPTR)
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
    connect(KGlobalAccel::self(), &KGlobalAccel::globalShortcutChanged,
        [this](QAction *action, const QKeySequence &seq) {
            if (action != m_action) {
                return;
            }
            setKeySequence(seq);
        }
    );
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
        // The default key sequence should be activated. We check first if this is
        // possible.
        if (m_customEditor->isKeySequenceAvailable(m_defaultKeySequence)) {
            // Clear the customs widget
            m_customEditor->clearKeySequence();
            emit keySequenceChanged(m_defaultKeySequence);
        } else {
            // We tried to switch to the default key sequence and failed. Go
            // back.
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
    if (activeSeq.toString(QKeySequence::NativeText) == m_defaultLabel->text()) {
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

