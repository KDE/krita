/**
 * Copyright (c) 2006 C. Boemann (cbo@boemann.dk)
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
#ifndef KOCOLORPATCH_H
#define KOCOLORPATCH_H

#include <QFrame>

#include <KoColor.h>
#include "kritawidgets_export.h"
#include <KoColorDisplayRendererInterface.h>

/**
 *  The small widget showing the selected color
 */
class KRITAWIDGETS_EXPORT KoColorPatch : public QFrame
{
  Q_OBJECT
public:
    explicit KoColorPatch( QWidget *parent );
    ~KoColorPatch() override;

    /**
     * Set the color of this color patch
     * @param c the new color
     */
    void setColor( const KoColor &c );

    /**
     * @brief setDisplayRenderer
     * Set the display renderer of this object.
     * @param displayRenderer
     */
    void setDisplayRenderer(const KoColorDisplayRendererInterface *displayRenderer);

    /**
     * @brief getColorFromDisplayRenderer
     * Get QColor from the display renderers
     * @param c
     */
    QColor getColorFromDisplayRenderer(KoColor c);

    /**
     * @return current color shown by this patch
     */
    KoColor color() const;

protected:
    void mousePressEvent(QMouseEvent *e ) override; ///< reimplemented from QFrame
    void paintEvent(QPaintEvent *e) override; ///< reimplemented from QFrame
    QSize sizeHint() const override; ///< reimplemented from QFrame

Q_SIGNALS:

    /**
     * Emitted when the mouse is released.
     * @param widget a pointer to this widget
     */
    void triggered(KoColorPatch *widget);

private:
  KoColor m_color;
  const KoColorDisplayRendererInterface *m_displayRenderer;
};

#endif
