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

#ifndef __KIS_HIGHLIGHTED_BUTTON_H
#define __KIS_HIGHLIGHTED_BUTTON_H

template <class BaseButton>
class HighlightedButtonBase : public BaseButton
{
public:
    HighlightedButtonBase(QWidget *parent = 0) : BaseButton(parent) {}
protected:
    void checkStateSet() override {
        BaseButton::checkStateSet();
        updatePalette();
    }

    void nextCheckState() override {
        BaseButton::nextCheckState();
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


class QPushButton;
class QToolButton;
typedef HighlightedButtonBase<QPushButton> KisHighlightedButton;
typedef HighlightedButtonBase<QToolButton> KisHighlightedToolButton;

#endif /* __KIS_HIGHLIGHTED_BUTTON_H */
