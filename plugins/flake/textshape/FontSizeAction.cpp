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

#include <QFontDatabase>
#include <QToolBar>
#include <QToolButton>

#include <QDebug>
#include <klocalizedstring.h>
#include <QIcon>

#include "FontSizeAction.h"

QString format(qreal v)
{
    static const QString f("%1");
    static const QString e("");
    static const QRegExp r("\\.?0+$");
    return f.arg(v, 0, 'f').replace(r, e);
}

class FontSizeAction::Private
{
public:
    Private(FontSizeAction *parent)
        : q(parent)
    {
    }

    void init();

    FontSizeAction *q;
};

// BEGIN FontSizeAction
FontSizeAction::FontSizeAction(QObject *parent)
    : KSelectAction(parent)
    , d(new Private(this))
{
    d->init();
}

FontSizeAction::FontSizeAction(const QString &text, QObject *parent)
    : KSelectAction(text, parent)
    , d(new Private(this))
{
    d->init();
}

FontSizeAction::FontSizeAction(const QIcon &icon, const QString &text, QObject *parent)
    : KSelectAction(icon, text, parent)
    , d(new Private(this))
{
    d->init();
}

FontSizeAction::~FontSizeAction()
{
    delete d;
}

void FontSizeAction::Private::init()
{
    q->setEditable(true);
    QFontDatabase fontDB;
    const QList<int> sizes = fontDB.standardSizes();
    QStringList lst;
    for (QList<int>::ConstIterator it = sizes.begin(); it != sizes.end(); ++it) {
        lst.append(format(*it));
    }
    q->setItems(lst);
}

void FontSizeAction::setFontSize(qreal size)
{
    if (size == fontSize()) {
        const QString test = format(size);
        Q_FOREACH (QAction *action, actions()) {
            if (action->text() == test) {
                setCurrentAction(action);
                return;
            }
        }
    }

    if (size < 1) {
        qWarning() << "FontSizeAction: Size " << size << " is out of range";
        return;
    }

    QAction *a = action(format(size));
    if (!a) {
        // Insert at the correct position in the list (to keep sorting)
        QList<qreal> lst;
        // Convert to list of qreals
        QStringListIterator itemsIt(items());
        QStringList debug_lst = items();

        while (itemsIt.hasNext()) {
            lst.append(itemsIt.next().toDouble());
        }
        //add the new size
        lst.append(size);

        //remove actions
        clear();

        // Sort the list
        qSort(lst);
        Q_FOREACH (qreal it, lst) {
            QAction *const action = addAction(format(it));
            if (it == size) {
                setCurrentAction(action);
            }
        }

    } else {
        setCurrentAction(a);
    }
}

qreal FontSizeAction::fontSize() const
{
    return currentText().toDouble();
}

void FontSizeAction::actionTriggered(QAction *action)
{
    emit fontSizeChanged(action->text().toDouble());
    KSelectAction::actionTriggered(action);
}
