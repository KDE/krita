/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2006 Sven Langkamp <sven.langkamp@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
