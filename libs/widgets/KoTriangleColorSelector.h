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

#include "kritawidgets_export.h"

#include <KisColorSelectorInterface.h>

class KoColor;
class KoColorDisplayRendererInterface;


class KRITAWIDGETS_EXPORT KoTriangleColorSelector : public KisColorSelectorInterface {
    Q_OBJECT
    public:
        explicit KoTriangleColorSelector(QWidget *parent);
        explicit KoTriangleColorSelector(const KoColorDisplayRendererInterface *displayRenderer, QWidget *parent);
        ~KoTriangleColorSelector() override;
    protected: // events
        void paintEvent( QPaintEvent * event ) override;
        void resizeEvent( QResizeEvent * event ) override;
        void mouseReleaseEvent( QMouseEvent * event ) override;
        void mousePressEvent( QMouseEvent * event ) override;
        void mouseMoveEvent( QMouseEvent * event ) override;
    public:
        int hue() const;
        int value() const;
        int saturation() const;
        KoColor getCurrentColor() const override;
    public Q_SLOTS:
        void setHue(int h);
        void setValue(int v);
        void setSaturation(int s);
        void setHSV(int h, int s, int v);
        void slotSetColor(const KoColor& ) override;
    Q_SIGNALS:
        void colorChanged(const QColor& );
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
