/*
 *  SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_CMB_GRADIENT_H
#define KIS_CMB_GRADIENT_H

#include <KisPopupButton.h>

#include <KoAbstractGradient.h>
#include <KoCheckerBoardPainter.h>

class KoResource;
class KisGradientChooser;

/**
 * @brief The KisCmbGradient class allows the user to select a gradient.
 */
class KisCmbGradient : public KisPopupButton
{
    Q_OBJECT
public:
    explicit KisCmbGradient(QWidget *parent = 0);

    void setGradient(KoAbstractGradientSP gradient);
    KoAbstractGradientSP gradient() const;

    void setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface);

    QSize sizeHint() const override;

protected:
    void resizeEvent(QResizeEvent *event) override;

Q_SIGNALS:

    void gradientChanged(KoAbstractGradientSP);

private Q_SLOTS:

    void gradientSelected(KoResourceSP resource);

private:
    KisGradientChooser *m_gradientChooser;
    KoCheckerBoardPainter m_checkersPainter;
};

#endif // KIS_CMB_GRADIENT_H
