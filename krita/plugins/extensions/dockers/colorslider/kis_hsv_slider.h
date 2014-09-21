/* This file is part of the KDE project
   Copyright (C) 2006 Sven Langkamp <sven.langkamp@gmail.com>
   Copyright (C) 2014 Wolthera van Hövell <griffinvalley@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KIS_HSVSLIDER_H
#define KIS_HSVSLIDER_H

#include <kselector.h>
#include "kowidgets_export.h"
#include "KoColorDisplayRendererInterface.h"
#include "kis_canvas2.h"


class KoColor;
class KisDisplayColorConverter;

class KOWIDGETS_EXPORT KisHSVSlider : public KSelector
{
    Q_OBJECT
public:
    explicit KisHSVSlider(QWidget *parent = 0, KoColorDisplayRendererInterface *displayRenderer = KoDumbColorDisplayRenderer::instance());
    explicit KisHSVSlider(Qt::Orientation orientation, QWidget *parent = 0, KoColorDisplayRendererInterface *displayRenderer = KoDumbColorDisplayRenderer::instance(), KisCanvas2* canvas = 0);
    virtual ~KisHSVSlider();

public:
    void setColors( const KoColor& currentcolor, const int type, qreal hue_backup, qreal l_R=0.2126, qreal l_G=0.7152, qreal l_B=0.0722);
    /**
     * Return the current color
     */
    KoColor currentColor() const;
    KoColor HSXcolor(int type, qreal t) const;
    KisDisplayColorConverter* converter() const;
protected:
    virtual void drawContents( QPainter* );
    struct Private;
    Private* const d;
private:
    qreal R, G, B;
    KoColorDisplayRendererInterface *m_displayRenderer;
    KisCanvas2* m_canvas;
};

#endif
