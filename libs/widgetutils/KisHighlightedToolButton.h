/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_HIGHLIGHTED_TOOL_BUTTON_H
#define __KIS_HIGHLIGHTED_TOOL_BUTTON_H

#include <QToolButton>

#include "kritawidgetutils_export.h"

class KRITAWIDGETUTILS_EXPORT KisHighlightedToolButton : public QToolButton
{
public:
    KisHighlightedToolButton(QWidget *parent = 0)
        : QToolButton(parent)
    {
    }

protected:
    void checkStateSet() override {
        QToolButton::checkStateSet();
        updatePalette();
    }

    void nextCheckState() override {
        QToolButton::nextCheckState();
        updatePalette();
    }

private:
    void updatePalette() {
        QWidget *parent = this->parentWidget();
        if (parent) {
            QPalette p = parent->palette();
            QColor color = p.color(this->isChecked() ? QPalette::Highlight : QPalette::Button);
            p.setColor(QPalette::Button, color);
            this->setPalette(p);
        }
    }
};


#endif /* __KIS_HIGHLIGHTED_BUTTON_H */
