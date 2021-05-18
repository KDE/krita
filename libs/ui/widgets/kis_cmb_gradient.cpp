/*
 *  SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_cmb_gradient.h"

#include <QPainter>
#include <QResizeEvent>
#include <QStyleOptionComboBox>
#include <QApplication>

#include <KoCheckerBoardPainter.h>
#include <KoResource.h>
#include <resources/KoAbstractGradient.h>
#include <KisGradientChooser.h>

KisCmbGradient::KisCmbGradient(QWidget *parent)
    : KisPopupButton(parent)
    , m_gradientChooser(new KisGradientChooser(this))
    , m_checkersPainter(4)
{
    m_gradientChooser->setContentsMargins(10, 10, 10, 10);
    connect(m_gradientChooser, SIGNAL(resourceSelected(KoResourceSP )), SLOT(gradientSelected(KoResourceSP )));
    setPopupWidget(m_gradientChooser);
}

void KisCmbGradient::setGradient(KoAbstractGradientSP gradient)
{
    m_gradientChooser->setCurrentResource(gradient);
}

KoAbstractGradientSP KisCmbGradient::gradient() const
{
    return m_gradientChooser->currentResource().dynamicCast<KoAbstractGradient>();
}

void KisCmbGradient::setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    m_gradientChooser->setCanvasResourcesInterface(canvasResourcesInterface);
}

void KisCmbGradient::gradientSelected(KoResourceSP resource)
{
    KoAbstractGradientSP gradient = resource.dynamicCast<KoAbstractGradient>();
    if (!gradient) return;

    QImage preview = gradient->generatePreview(iconSize().width(), iconSize().height(), m_gradientChooser->canvasResourcesInterface());

    QImage thumbnail(preview.size(), QImage::Format_ARGB32);

    {
        QPainter gc(&thumbnail);
        m_checkersPainter.paint(gc, preview.rect());
        gc.drawImage(QPoint(), preview);
    }

    setIcon(QIcon(QPixmap::fromImage(thumbnail)));

    emit gradientChanged(gradient);
}

QSize KisCmbGradient::sizeHint() const
{
    ensurePolished();
    QFontMetrics fm = fontMetrics();
#if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
    int maxW = 7 * fm.horizontalAdvance(QChar('x')) + 18;
#else
    int maxW = 7 * fm.width(QChar('x')) + 18;
#endif
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
