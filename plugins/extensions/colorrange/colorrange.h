/*
 * colorrange.h -- Part of Krita
 *
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt (boud@valdyas.org)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COLORRANGE_H
#define COLORRANGE_H

#include <QVariant>

#include <KisActionPlugin.h>

class ColorRange : public KisActionPlugin
{
    Q_OBJECT
public:
    ColorRange(QObject *parent, const QVariantList &);
    ~ColorRange() override;

private Q_SLOTS:
    void slotActivated();
    void selectOpaque(int id);
};

#endif // COLORRANGE_H
