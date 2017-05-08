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

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    int currentIndex() const;

public Q_SLOTS:
    void setCurrentIndex(int index);

Q_SIGNALS:
    void currentIndexChanged(int index);

protected:

    void resizeEvent(QResizeEvent *e) override;
    void paintEvent(QPaintEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_COLOR_LABEL_SELECTOR_WIDGET_H */
