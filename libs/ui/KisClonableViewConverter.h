/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCLONABLEVIEWCONVERTER_H
#define KISCLONABLEVIEWCONVERTER_H

#include "kritaui_export.h"
#include <KoViewConverter.h>

class KisClonableViewConverter : public KoViewConverter
{
public:
    virtual KisClonableViewConverter* clone() const = 0;
};

#endif // KISCLONABLEVIEWCONVERTER_H
