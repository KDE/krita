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
#ifndef KIS_CMB_GRADIENT_H
#define KIS_CMB_GRADIENT_H

#include <KisPopupButton.h>

#include <KoAbstractGradient.h>

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

    QSize sizeHint() const override;

protected:
    void resizeEvent(QResizeEvent *event) override;

Q_SIGNALS:

    void gradientChanged(KoAbstractGradientSP);

private Q_SLOTS:

    void gradientSelected(KoResourceSP resource);

private:
    KisGradientChooser *m_gradientChooser;
};

#endif // KIS_CMB_GRADIENT_H
