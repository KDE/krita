/* This file is part of the KDE project
 * Copyright (C) 2010 Adam Celarek <kdedev at xibo dot at>
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

#ifndef KIS_CURVE_WIDGET_H
#define KIS_CURVE_WIDGET_H

#include <QtGui/QWidget>

class KisCurveWidgetBase;

class KisCurveWidget : public QWidget
{
    Q_OBJECT

public:
    KisCurveWidget(QWidget *parent = 0);
    ~KisCurveWidget();

public slots:
    void switchToFunction() {switchTo(m_functionLikeWidget);}
    void switchToCubic() {switchTo(m_splineWidget);}
    void switchToLinear() {switchTo(m_lineWidget);}
    void switchToFreehand() {switchTo(m_freehandWidget);}
    void reset();

protected:
    void switchTo(KisCurveWidgetBase* newWidget);

private:
    KisCurveWidgetBase* m_currentCurve;
    KisCurveWidgetBase* m_functionLikeWidget;
    KisCurveWidgetBase* m_splineWidget;
    KisCurveWidgetBase* m_lineWidget;
    KisCurveWidgetBase* m_freehandWidget;
};

#endif // KIS_CURVE_WIDGET_H
