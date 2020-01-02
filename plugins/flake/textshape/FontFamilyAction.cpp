/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>
              (C) 2003 Andras Mantia <amantia@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>
              (C) 2007 Clarence Dang <dang@kde.org>
              (C) 2014 Dan Leinir Turthra Jensen <admin@leinir.dk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// This is a minorly modified version of the KFontAction class. It exists
// entirely because there's a hang bug on windows at the moment.

#include "FontFamilyAction.h"

#include <QToolBar>

#include <QDebug>
#include <QIcon>
#include <klocalizedstring.h>
#include <kfontchooser.h>
#include <QFontComboBox>

class KoFontFamilyAction::KoFontFamilyActionPrivate
{
public:
    KoFontFamilyActionPrivate(KoFontFamilyAction *parent)
        : q(parent)
        , settingFont(0)
    {
    }

    void _ko_slotFontChanged(const QFont &font)
    {
        qDebug() << "QFontComboBox - slotFontChanged("
                 << font.family() << ") settingFont=" << settingFont;
        if (settingFont) {
            return;
        }

        q->setFont(font.family());
        q->triggered(font.family());

        qDebug() << "\tslotFontChanged done";
    }

    KoFontFamilyAction *q;
    int settingFont;
};

KoFontFamilyAction::KoFontFamilyAction(uint fontListCriteria, QObject *parent)
    : KSelectAction(parent)
    , d(new KoFontFamilyActionPrivate(this))
{
    QStringList list;
    KFontChooser::getFontList(list, fontListCriteria);
    KSelectAction::setItems(list);
    setEditable(true);
}

KoFontFamilyAction::KoFontFamilyAction(QObject *parent)
    : KSelectAction(parent)
    , d(new KoFontFamilyActionPrivate(this))
{
    QStringList list;
    KFontChooser::getFontList(list, 0);
    KSelectAction::setItems(list);
    setEditable(true);
}

KoFontFamilyAction::KoFontFamilyAction(const QString &text, QObject *parent)
    : KSelectAction(text, parent)
    , d(new KoFontFamilyActionPrivate(this))
{
    QStringList list;
    KFontChooser::getFontList(list, 0);
    KSelectAction::setItems(list);
    setEditable(true);
}

KoFontFamilyAction::KoFontFamilyAction(const QIcon &icon, const QString &text, QObject *parent)
    : KSelectAction(icon, text, parent)
    , d(new KoFontFamilyActionPrivate(this))
{
    QStringList list;
    KFontChooser::getFontList(list, 0);
    KSelectAction::setItems(list);
    setEditable(true);
}

KoFontFamilyAction::~KoFontFamilyAction()
{
    delete d;
}

QString KoFontFamilyAction::font() const
{
    return currentText();
}

QWidget *KoFontFamilyAction::createWidget(QWidget *parent)
{
    qDebug() << "KoFontFamilyAction::createWidget()";
// silence unclear warning from original kfontaction.acpp
// #ifdef __GNUC__
// #warning FIXME: items need to be converted
// #endif
    // This is the visual element on the screen.  This method overrides
    // the KSelectAction one, preventing KSelectAction from creating its
    // regular KComboBox.
    QFontComboBox *cb = new QFontComboBox(parent);
    //cb->setFontList(items());

    qDebug() << "\tset=" << font();
    // Do this before connecting the signal so that nothing will fire.
    cb->setCurrentFont(QFont(font().toLower()));
    qDebug() << "\tspit back=" << cb->currentFont().family();

    connect(cb, SIGNAL(currentFontChanged(QFont)), SLOT(_ko_slotFontChanged(QFont)));
    cb->setMinimumWidth(cb->sizeHint().width());
    return cb;
}

/*
 * Maintenance note: Keep in sync with KFontComboBox::setCurrentFont()
 */
void KoFontFamilyAction::setFont(const QString &family)
{
    qDebug() << "KoFontFamilyAction::setFont(" << family << ")";

    // Suppress triggered(QString) signal and prevent recursive call to ourself.
    d->settingFont++;

    Q_FOREACH (QWidget *w, createdWidgets()) {
        QFontComboBox *cb = qobject_cast<QFontComboBox *>(w);
        qDebug() << "\tw=" << w << "cb=" << cb;

        if (!cb) {
            continue;
        }

        cb->setCurrentFont(QFont(family.toLower()));
        qDebug() << "\t\tw spit back=" << cb->currentFont().family();
    }

    d->settingFont--;

    qDebug() << "\tcalling setCurrentAction()";

    QString lowerName = family.toLower();
    if (setCurrentAction(lowerName, Qt::CaseInsensitive)) {
        return;
    }

    int i = lowerName.indexOf(" [");
    if (i > -1) {
        lowerName = lowerName.left(i);
        i = 0;
        if (setCurrentAction(lowerName, Qt::CaseInsensitive)) {
            return;
        }
    }

    lowerName += " [";
    if (setCurrentAction(lowerName, Qt::CaseInsensitive)) {
        return;
    }

    // TODO: Inconsistent state if KFontComboBox::setCurrentFont() succeeded
    //       but setCurrentAction() did not and vice-versa.
    qDebug() << "Font not found " << family.toLower();
}

// have to include this because of Q_PRIVATE_SLOT
#include "moc_FontFamilyAction.cpp"
