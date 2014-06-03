/* This file is part of the KDE project
 *
 * Copyright (c) 2010 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KoFindToolbar_p_h
#define KoFindToolbar_p_h

#include "KoFindToolbar.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QMenu>
#include <QTimer>
#include <QApplication>
#include <QDebug>

#include <klocalizedstring.h>
#include <klineedit.h>
#include <ksqueezedtextlabel.h>
#include <khistorycombobox.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kcolorscheme.h>

#include <KoIcon.h>

#include "KoFindBase.h"
#include "KoFindOptionSet.h"
#include "KoFindOption.h"

class KoFindToolbar::Private
{
public:
    Private(KoFindToolbar *qq) : q(qq) { }

    void matchFound();
    void noMatchFound();
    void searchWrapped(bool direction);
    void addToHistory();
    void find(const QString &pattern);
    void optionChanged();
    void replace();
    void replaceAll();
    void inputTimeout();

    KoFindToolbar *q;

    KoFindBase *finder;

    QToolButton *closeButton;
    KHistoryComboBox *searchLine;
    KHistoryComboBox *replaceLine;
    QToolButton *previousButton;
    QToolButton *nextButton;
    QToolButton *optionsButton;
    QToolButton *replaceButton;
    QToolButton *replaceAllButton;
    QLabel *replaceLabel;
    KSqueezedTextLabel *information;
    QLabel *matchCounter;
    QTimer *textTimeout;

    static QStringList searchCompletionItems;
    static QStringList replaceCompletionItems;
};

#endif