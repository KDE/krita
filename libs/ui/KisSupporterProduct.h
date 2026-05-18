/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef __KISSUPPORTERPRODUCT_H_
#define __KISSUPPORTERPRODUCT_H_

#include <QPixmap>
#include <QString>

#include "kritaui_export.h"

// Keep in sync with KisSupporterProduct.java!

enum class KRITAUI_EXPORT KisSupporterProductType {
    OneTime,
    Subscription,
};

enum class KRITAUI_EXPORT KisSupporterProductAvailability {
    Missing,
    Available,
    Owned,
};

struct KRITAUI_EXPORT KisSupporterProduct {
    KisSupporterProductType type;
    KisSupporterProductAvailability availability;
    QString id;
    QString title;
    QString description;
    QPixmap icon;
    QString buttonText;
    QString offerToken;
};

#endif
