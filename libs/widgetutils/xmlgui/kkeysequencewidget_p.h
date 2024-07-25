/* This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2001, 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KKEYSEQUENCEWIDGET_P_H
#define KKEYSEQUENCEWIDGET_P_H

#include <QPushButton>

class KKeySequenceButton: public QPushButton
{
    Q_OBJECT

public:
    explicit KKeySequenceButton(KisKKeySequenceWidgetPrivate *d, QWidget *parent)
        : QPushButton(parent),
          d(d) {}

    ~KKeySequenceButton() override;

protected:
    /**
    * Reimplemented for internal reasons.
    */
    bool event(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    KisKKeySequenceWidgetPrivate *const d;
};

#endif //KKEYSEQUENCEWIDGET_P_H
