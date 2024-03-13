/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoSvgTextPropertyData.h"
#include <QDebug>

#include <KisStaticInitializer.h>

KIS_DECLARE_STATIC_INITIALIZER {
    qRegisterMetaType<KoSvgTextPropertyData>("KoSvgTextPropertyData");
    QMetaType::registerEqualsComparator<KoSvgTextPropertyData>();
    QMetaType::registerDebugStreamOperator<KoSvgTextPropertyData>();
}

QDebug operator<<(QDebug dbg, const KoSvgTextPropertyData &prop)
{
    dbg.nospace() << "TextPropertyData(";
    dbg.nospace() << " Common properties:" << prop.commonProperties.convertParagraphProperties() << prop.commonProperties.convertToSvgTextAttributes();
    dbg.nospace() << " Tristate:" << prop.tristate;
    dbg.nospace() << " )";
    return dbg.space();
}
