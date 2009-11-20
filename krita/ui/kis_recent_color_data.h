/* This file is part of the KDE project
   Copyright 2009 Vera Lukman <shichan.karachu@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KIS_RECENT_COLOR_DATA_H
#define KIS_RECENT_COLOR_DATA_H

#include <QObject>
class QToolButton;
class QColor;
class QIcon;

class KisRecentColorData : public QObject
{
    Q_OBJECT

public:
    KisRecentColorData(QColor*); //CHANGE TO color
    ~KisRecentColorData();
    QColor* color(); //CHANGE TO color
    void setIcon (QIcon*);
    QToolButton* colorButton();

private:
    QToolButton* m_button;
    QColor* m_data; //CHANGE TO color

private slots:
    void slotColorButtonClicked();
};

#endif // KIS_RECENT_COLOR_DATA_H
