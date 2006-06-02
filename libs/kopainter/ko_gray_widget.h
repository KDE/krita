/*
 *  Copyright (c) 1999 Matthias Elter (me@kde.org)
 *  Copyright (c) 2001-2002 Igor Jansen (rm@kde.org)
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
#ifndef KO_GRAY_WIDGET_H
#define KO_GRAY_WIDGET_H

#include "qwidget.h"

#include <koffice_export.h>
#include <kdualcolorbutton.h>

class Q3GridLayout;
class QColor;
class QLabel;
class QSpinBox;
class KDualColorButton;
class KGradientSelector;

class KOPAINTER_EXPORT KoGrayWidget
     : public QWidget
{
    Q_OBJECT
    typedef QWidget super;

public:
    KoGrayWidget(QWidget *parent = 0L, const char *name = 0);
    virtual ~KoGrayWidget() {}

public slots:
    /**
     * Set the current color to c. Do not emit the color changed signals
     */
    virtual void setFgColor(const QColor & c);
    virtual void setBgColor(const QColor & c);

signals:

    /**
     * Emitted when the current color is changed.
     */
    void sigFgColorChanged(const QColor & c);
    void sigBgColorChanged(const QColor & c);

protected slots:
    virtual void slotChanged(int v);

    void slotFGColorSelected(const QColor& c);
    void slotBGColorSelected(const QColor& c);
    void currentChanged(KDualColorButton::DualColor);

private:

    void update(const QColor & fgColor, const QColor & bgColor);

    KGradientSelector *mSlider;
    QLabel *mLabel;
    QSpinBox *mIn;
    KDualColorButton *m_ColorButton;

    QColor m_fgColor;
    QColor m_bgColor;
};

#endif
