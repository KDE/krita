/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2021 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CANVAS_ONLY_ACTION_H
#define KIS_CANVAS_ONLY_ACTION_H

#include "kis_abstract_input_action.h"

class KisCanvasOnlyAction : public KisAbstractInputAction
{
    public:
    explicit KisCanvasOnlyAction();
    ~KisCanvasOnlyAction() override;

    int priority() const override;
    void begin(int shortcut, QEvent *event = 0) override;
};

#endif // KIS_CANVAS_ONLY_ACTION_H
