/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KO_TRIANGLE_COLOR_SELECTOR_H_
#define _KO_TRIANGLE_COLOR_SELECTOR_H_

#include <QWidget>

#include "kowidgets_export.h"

class KOWIDGETS_EXPORT KoTriangleColorSelector : public QWidget {
    Q_OBJECT
    public:
        KoTriangleColorSelector(QWidget* parent);
        ~KoTriangleColorSelector();
    protected: // events
        void paintEvent( QPaintEvent * event );
        void resizeEvent( QResizeEvent * event );
        void mouseReleaseEvent( QMouseEvent * event );
        void mousePressEvent( QMouseEvent * event );
        void mouseMoveEvent( QMouseEvent * event );
    public:
        int hue() const;
        int value() const;
        int saturation() const;
        QColor color() const;
    public slots:
        void setHue(int h);
        void setValue(int v);
        void setSaturation(int s);
        void setHSV(int h, int s, int v);
        void setQColor(const QColor& );
    signals:
        void colorChanged(const QColor& );
    private:
        void tellColorChanged();
        void generateTriangle();
        void generateWheel();
        void updateTriangleCircleParameters();
        void selectColorAt(int x, int y, bool checkInWheel = true);
    private:
        struct Private;
        Private* const d;
};

#endif
