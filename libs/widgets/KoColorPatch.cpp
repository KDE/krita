/**
 * SPDX-FileCopyrightText: 2006 C. Boemann (cbo@boemann.dk)
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
