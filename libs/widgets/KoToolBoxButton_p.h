/*
 * SPDX-FileCopyrightText: 2015 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KO_TOOLBOXBUTTON_H_
#define _KO_TOOLBOXBUTTON_H_

#include <QToolButton>

class KoToolAction;

class KoToolBoxButton : public QToolButton
{
    Q_OBJECT
public:
    explicit KoToolBoxButton(KoToolAction *toolAction, QWidget * parent);
    void setHighlightColor();

private Q_SLOTS:
    void setDataFromToolAction(); // Generates tooltips.
private:
    KoToolAction *m_toolAction;
};

#endif // _KO_TOOLBOXBUTTON_H_
