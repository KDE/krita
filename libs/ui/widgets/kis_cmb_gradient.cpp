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

#include <KoResource.h>
#include <resources/KoAbstractGradient.h>

#include <kis_signals_blocker.h>
#include <KisGenericGradientEditor.h>

KisCmbGradient::KisCmbGradient(QWidget *parent)
    : KisPopupButton(parent)
    , m_checkersPainter(4)
    , m_gradientEditor(new KisGenericGradientEditor())
{
    m_gradientEditor->setContentsMargins(10, 10, 10, 10);
    m_gradientEditor->loadUISettings();
    connect(m_gradientEditor, &KisGenericGradientEditor::sigGradientChanged,
            this, &KisCmbGradient::gradientSelected);
    setPopupWidget(m_gradientEditor);
}

KisCmbGradient::~KisCmbGradient()
{
    m_gradientEditor->saveUISettings();
}

void KisCmbGradient::setGradient(KoAbstractGradientSP gradient)
{
    KisSignalsBlocker b(m_gradientEditor);
    m_gradientEditor->setGradient(gradient);
    updateGradientPreview();
}

KoAbstractGradientSP KisCmbGradient::gradient() const
{
    return m_gradientEditor->gradient();
}

void KisCmbGradient::setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    m_gradientEditor->setCanvasResourcesInterface(canvasResourcesInterface);
}

void KisCmbGradient::gradientSelected()
{
    updateGradientPreview();

    Q_EMIT gradientChanged(this->gradient());
}

void KisCmbGradient::updateGradientPreview()
{
    const QSize previewSize = iconSize();
    QImage thumbnail(previewSize, QImage::Format_ARGB32);

    {
        QPainter gc(&thumbnail);
        m_checkersPainter.paint(gc, QRect(QPoint(), previewSize));

        KoAbstractGradientSP gradient = this->gradient();
        if (gradient) {
            QImage preview = gradient->generatePreview(previewSize.width(), previewSize.height(), m_gradientEditor->canvasResourcesInterface());
            gc.drawImage(QPoint(), preview);
        }
    }

    setIcon(QIcon(QPixmap::fromImage(thumbnail)));
}

QSize KisCmbGradient::sizeHint() const
{
    ensurePolished();
    QFontMetrics fm = fontMetrics();
    int maxW = 7 * fm.horizontalAdvance(QChar('x')) + 18;
    int maxH = qMax(fm.lineSpacing(), 14) + 2;

    QStyleOptionComboBox options;
    options.initFrom(this);

    return style()->sizeFromContents(QStyle::CT_ComboBox, &options, QSize(maxW, maxH), this);
}

void KisCmbGradient::resizeEvent(QResizeEvent *event)
{
    setIconSize(QSize(event->size().width() - 30, event->size().height() - 4));
    updateGradientPreview();
    KisPopupButton::resizeEvent(event);
}
