/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISLAYERSTYLEKNOCKOUTBLOWER_H
#define KISLAYERSTYLEKNOCKOUTBLOWER_H

#include "kis_selection.h"
#include <QReadWriteLock>

class KisPainter;


class KRITAIMAGE_EXPORT KisLayerStyleKnockoutBlower
{
public:
    KisLayerStyleKnockoutBlower();
    KisLayerStyleKnockoutBlower(const KisLayerStyleKnockoutBlower &rhs);

    KisSelectionSP knockoutSelectionLazy();

    void setKnockoutSelection(KisSelectionSP selection);
    void resetKnockoutSelection();


    void apply(KisPainter *painter, KisPaintDeviceSP mergedStyle, const QRect &rect) const;
    bool isEmpty() const;

private:
    mutable QReadWriteLock m_lock;
    KisSelectionSP m_knockoutSelection;
};

#endif // KISLAYERSTYLEKNOCKOUTBLOWER_H
