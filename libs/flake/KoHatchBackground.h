/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2012 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOHATCHBACKGROUND_H
#define KOHATCHBACKGROUND_H

#include "KoColorBackground.h"

/**
 * A hatch shape background
 */
class KoHatchBackground : public KoColorBackground
{
public:
    enum HatchStyle {
        Single,
        Double,
        Triple
    };

    KoHatchBackground();
    ~KoHatchBackground() override;

    // reimplemented
    void paint(QPainter &painter, KoShapePaintingContext &context, const QPainterPath &fillPath) const override;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

#endif /* KOHATCHBACKGROUND_H */
