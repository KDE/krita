/* This file is part of the KDE project
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2003 Andras Mantia <amantia@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>
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

#ifndef FONTSIZEACTION_H
#define FONTSIZEACTION_H

#include <kselectaction.h>

class QIcon;

/**
 * An action to allow changing of the font size.
 * This action will be shown as a combobox on a toolbar with a proper set of font sizes.
 *
 * NOTE: We do not use KFontSizeAction because it does not support font size
 * values of type qreal.
 */
class FontSizeAction : public KSelectAction
{
    Q_OBJECT
    Q_PROPERTY(qreal fontSize READ fontSize WRITE setFontSize)

public:
    explicit FontSizeAction(QObject *parent);
    FontSizeAction(const QString &text, QObject *parent);
    FontSizeAction(const QIcon &icon, const QString &text, QObject *parent);

    ~FontSizeAction() override;

    qreal fontSize() const;

    void setFontSize(qreal size);

Q_SIGNALS:
    void fontSizeChanged(qreal);

protected Q_SLOTS:
    /**
     * This function is called whenever an action from the selections is triggered.
     */
    void actionTriggered(QAction *action) override;

private:
    class Private;
    Private *const d;
};

#endif
