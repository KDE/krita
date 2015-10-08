/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License,
 *  or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
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

class KoColor;
class KoColorDisplayRendererInterface;


class KOWIDGETS_EXPORT KoTriangleColorSelector : public QWidget {
    Q_OBJECT
    public:
        explicit KoTriangleColorSelector(QWidget *parent);
        explicit KoTriangleColorSelector(const KoColorDisplayRendererInterface *displayRenderer, QWidget *parent);
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
        KoColor realColor() const;

        // please use realColor() instead!
        Q_DECL_DEPRECATED QColor color() const;

    public Q_SLOTS:
        void setHue(int h);
        void setValue(int v);
        void setSaturation(int s);
        void setHSV(int h, int s, int v);

        // please use setRealColor() instead!
        Q_DECL_DEPRECATED void setQColor(const QColor& );

        void setRealColor(const KoColor& );
    Q_SIGNALS:
        void colorChanged(const QColor& );
        void realColorChanged(const KoColor& );
    private Q_SLOTS:
        void configurationChanged();
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
