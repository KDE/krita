/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
