/*
 *  SPDX-FileCopyrightText: 2018 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef H_GAMUT_MASK_PLUGIN_H
#define H_GAMUT_MASK_PLUGIN_H

#include <QObject>
#include <QVariant>

class GamutMaskPlugin: public QObject
{
public:
    GamutMaskPlugin(QObject *parent, const QVariantList &);
};

#endif // H_GAMUT_MASK_PLUGIN_H
