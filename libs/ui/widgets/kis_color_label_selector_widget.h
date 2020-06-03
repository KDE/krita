/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_COLOR_LABEL_SELECTOR_WIDGET_H
#define __KIS_COLOR_LABEL_SELECTOR_WIDGET_H

#include <QScopedPointer>
#include <QWidget>

#include "kritaui_export.h"


class KRITAUI_EXPORT KisColorLabelSelectorWidget : public QWidget
{
    Q_OBJECT

public:
    KisColorLabelSelectorWidget(QWidget *parent);
    ~KisColorLabelSelectorWidget() override;

    int currentIndex() const;

    QSize sizeHint() const;
    void resizeEvent(QResizeEvent* e) override;

    int calculateMenuOffset() const;

public Q_SLOTS:
    void groupButtonChecked(int index, bool state);
    void setCurrentIndex(int index);

Q_SIGNALS:
    void currentIndexChanged(int index);

private:
    class Private* m_d;
};

#endif /* __KIS_COLOR_LABEL_SELECTOR_WIDGET_H */
