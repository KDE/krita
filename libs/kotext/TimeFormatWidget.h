/* This file is part of the KDE project
 * Copyright (C) 2001 Tomasz Grobelny <grotk@poczta.onet.pl>
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
#ifndef TIMEFORMATWIDGET_H
#define TIMEFORMATWIDGET_H

#include <QWidget>
class QComboBox;
class Ui_TimeDateFormatWidgetPrototype;

class TimeFormatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimeFormatWidget( QWidget* parent );
    ~TimeFormatWidget();
    QString resultString() const;
    int correctValue() const;
    QComboBox *combo1();
    QComboBox *combo2();
public slots:
    void updateLabel();
    void comboActivated();
    void slotPersonalizeChanged(bool b);
    void slotDefaultValueChanged(const QString & );
    void slotOffsetChanged(int);

private:
    Ui_TimeDateFormatWidgetPrototype* m_ui;
};

#endif // TIMEFORMATWIDGET_H
