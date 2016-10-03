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
#include "KoColorPatch.h"

#include <QPainter>

KoColorPatch::KoColorPatch( QWidget *parent ) : QFrame( parent )
{
    m_displayRenderer = KoDumbColorDisplayRenderer::instance();
    connect(m_displayRenderer, SIGNAL(displayConfigurationChanged()),
            SLOT(update()), Qt::UniqueConnection);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

KoColorPatch::~KoColorPatch()
{
}

QSize KoColorPatch::sizeHint() const
{
    return QSize(12,12);
}

void KoColorPatch::setColor(const KoColor& c)
{
    m_color = c;

    update();
}
void KoColorPatch::setDisplayRenderer(const KoColorDisplayRendererInterface *displayRenderer)
{
    if (displayRenderer) {
        if (m_displayRenderer) {
            m_displayRenderer->disconnect(this);
        }
        m_displayRenderer = displayRenderer;
    } else {
        m_displayRenderer = KoDumbColorDisplayRenderer::instance();
    }
    connect(m_displayRenderer, SIGNAL(displayConfigurationChanged()),
            SLOT(update()), Qt::UniqueConnection);

}

QColor KoColorPatch::getColorFromDisplayRenderer(KoColor c)
{
    QColor col;
    if (m_displayRenderer) {
        c.convertTo(m_displayRenderer->getPaintingColorSpace());
        col = m_displayRenderer->toQColor(c);
    } else {
        col = c.toQColor();
    }
    return col;
}

KoColor KoColorPatch::color() const
{
    return m_color;
}

void KoColorPatch::mousePressEvent (QMouseEvent *e )
{
    Q_UNUSED( e );

    emit triggered(this);
}

void KoColorPatch::paintEvent(QPaintEvent *pe)
{
    QColor qc = getColorFromDisplayRenderer(m_color);
    QFrame::paintEvent(pe);
    QPainter painter( this );
    painter.setPen(QPen(qc, 0));
    painter.setBrush(QBrush(qc));
    painter.drawRect(contentsRect());
}
