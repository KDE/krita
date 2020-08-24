/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISWARNINGWIDGET_H
#define KISWARNINGWIDGET_H

#include "kritaui_export.h"
#include <QWidget>

class QLabel;

class KRITAUI_EXPORT KisWarningWidget : public QWidget
{
    Q_OBJECT
public:
    KisWarningWidget(QWidget *parent);

    void setText(const QString &text);

    /**
     * The default warning message for a case when the user
     * tries to change color profile for a multilayered image
     */
    static QString changeImageProfileWarningText();

private:
    QLabel *m_warningIcon = 0;
    QLabel *m_warningText = 0;
};

#endif // KISWARNINGWIDGET_H
