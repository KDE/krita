/*
 *  Copyright (c) 2015 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_cmb_gradient.h"

#include <QPainter>
#include <QResizeEvent>
#include <QStyleOptionComboBox>
#include <QApplication>

#include <KoCheckerBoardPainter.h>
#include <resources/KoResource.h>
#include <resources/KoAbstractGradient.h>
#include "kis_gradient_chooser.h"

KisCmbGradient::KisCmbGradient(QWidget *parent)
    : KisPopupButton(parent)
    , m_gradientChooser(new KisGradientChooser(this))
{
    connect(m_gradientChooser, SIGNAL(resourceSelected(KoResource*)), SLOT(gradientSelected(KoResource*)));
    setPopupWidget(m_gradientChooser);
}

void KisCmbGradient::setGradient(KoAbstractGradient *gradient)
{
    m_gradientChooser->setCurrentResource(gradient);
}

KoAbstractGradient *KisCmbGradient::gradient() const
{
    return dynamic_cast<KoAbstractGradient*>(m_gradientChooser->currentResource());
}

void KisCmbGradient::gradientSelected(KoResource *resource)
{
    KoAbstractGradient *gradient = dynamic_cast<KoAbstractGradient*>(resource);
    if (!gradient) return;

    QImage pm = gradient->generatePreview(iconSize().width(), iconSize().height());
    setIcon(QIcon(QPixmap::fromImage(pm)));

    emit gradientChanged(gradient);
}

QSize KisCmbGradient::sizeHint() const
{
    ensurePolished();
    QFontMetrics fm = fontMetrics();

    int maxW = 7 * fm.width(QChar('x')) + 18;
    int maxH = qMax(fm.lineSpacing(), 14) + 2;

    QStyleOptionComboBox options;
    options.initFrom(this);

    return style()->sizeFromContents(QStyle::CT_ComboBox, &options, QSize(maxW, maxH), this);
}

void KisCmbGradient::resizeEvent(QResizeEvent *event)
{
    setIconSize(QSize(event->size().width() - 30, event->size().height() - 4));
    KisPopupButton::resizeEvent(event);
}
