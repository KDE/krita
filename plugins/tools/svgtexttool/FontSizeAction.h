/* This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2000 Michael Koch <koch@kde.org>
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2003 Andras Mantia <amantia@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
