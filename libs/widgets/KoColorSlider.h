/* This file is part of the KDE project
   Copyright (C) 2006 Sven Langkamp <sven.langkamp@gmail.com>

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

#ifndef KO_COLORSLIDER_H
#define KO_COLORSLIDER_H

#include <kselector.h>
#include "kritawidgets_export.h"
#include "KoColorDisplayRendererInterface.h"

class KoColor;

class KRITAWIDGETS_EXPORT KoColorSlider : public KSelector
{
    Q_OBJECT
public:
    explicit KoColorSlider(QWidget *parent = 0, KoColorDisplayRendererInterface *displayRenderer = KoDumbColorDisplayRenderer::instance());
    explicit KoColorSlider(Qt::Orientation orientation, QWidget *parent = 0, KoColorDisplayRendererInterface *displayRenderer = KoDumbColorDisplayRenderer::instance());
    ~KoColorSlider() override;

public:
    void setColors( const KoColor& minColor, const KoColor& maxColor);
    /**
     * Return the current color
     */
    KoColor currentColor() const;
protected:
    void drawContents( QPainter* ) override;
    void drawArrow(QPainter *painter, const QPoint &pos) override;

protected:
    struct Private;
    Private* const d;
};

#endif
