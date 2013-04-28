/* This file is part of the KDE project
 * Copyright (C) 2010 C. Boemann <cbo@boemann.dk>
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
#ifndef SPECIALBUTTON_H
#define SPECIALBUTTON_H

#include <QFrame>

class QPixmap;
class QLabel;
class StylesWidget;

class SpecialButton : public QFrame
{
    Q_OBJECT
public:
    explicit SpecialButton(QWidget *parent);
    ~SpecialButton();

    void setStylesWidget(StylesWidget *stylesWidget);
    void setStylePreview(const QPixmap &pm);

    void showPopup();
    void hidePopup();

protected:
    virtual void mousePressEvent(QMouseEvent *event);

    StylesWidget *m_stylesWidget;
    QLabel *m_preview;
    bool isPopupVisible;
};

#endif //SPECIALBUTTON_H
