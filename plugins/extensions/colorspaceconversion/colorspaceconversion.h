/*
 * colorspaceconversion.h -- Part of Krita
 *
 * SPDX-FileCopyrightText: 2004 Boudewijn Rempt (boud@valdyas.org)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COLORSPACECONVERSION_H
#define COLORSPACECONVERSION_H

#include <QVariant>

#include <KisActionPlugin.h>

/**
 * Dialog for converting between color models.
 */
class ColorSpaceConversion : public KisActionPlugin
{
    Q_OBJECT
public:
    ColorSpaceConversion(QObject *parent, const QVariantList &);
    ~ColorSpaceConversion() override;

private Q_SLOTS:

    void slotImageColorSpaceConversion();
    void slotLayerColorSpaceConversion();
};

#endif // COLORSPACECONVERSION_H
