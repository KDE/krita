/* This file is part of the KDE project
 * Copyright (C) 2002 Montel Laurent <lmontel@mandrakesoft.com>
 * Copyright (C) 2002 Thomas Zander <zander@kde.org>
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
#ifndef DATEFORMATWIDGET_H
#define DATEFORMATWIDGET_H

#include <QWidget>
class Ui_TimeDateFormatWidgetPrototype;

class DateFormatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DateFormatWidget( QWidget* parent );
    ~DateFormatWidget();
    QString resultString();
    int correctValue();
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

#endif // DATEFORMATWIDGET_H
