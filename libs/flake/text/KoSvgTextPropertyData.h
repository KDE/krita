/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOSVGTEXTPROPERTYDATA_H
#define KOSVGTEXTPROPERTYDATA_H

#include "KoSvgTextProperties.h"
#include "kritaflake_export.h"

#include <boost/operators.hpp>
#include <QDebug>

/**
 * @brief The KoSvgTextPropertyData struct
 *
 * This struct contains the style data of a given set of text objects.
 * Within text editing it is very common to select ranges of multiple
 * styled spans, each with their own properties.
 *
 * This struct encapsulates the data to represent it inside a text property
 * widget, and is the source data for the lager model.
 */
struct KRITAFLAKE_EXPORT KoSvgTextPropertyData : public boost::equality_comparable<KoSvgTextPropertyData>
{
    /// The properties common between all the selected text.
    KoSvgTextProperties commonProperties;

    /// The properties that are inherited, so that widgets may be set correctly.
    KoSvgTextProperties inheritedProperties {KoSvgTextProperties::defaultProperties()};

    /// The properties that are not common (tri-state) between the selected text.
    QSet<KoSvgTextProperties::PropertyId> tristate;

    /// Whether we're currently selecting a span of text as opposed to editing the whole paragraph.
    bool spanSelection {false};

    bool enabled{false};

    bool operator==(const KoSvgTextPropertyData &rhs) const {
        return commonProperties == rhs.commonProperties
                && inheritedProperties == rhs.inheritedProperties
                && tristate == rhs.tristate
                && spanSelection == rhs.spanSelection
                && enabled == rhs.enabled;
    }
};

QDebug KRITAFLAKE_EXPORT operator<<(QDebug dbg, const KoSvgTextPropertyData &prop);

Q_DECLARE_METATYPE(KoSvgTextPropertyData)

#endif // KOSVGTEXTPROPERTYDATA_H
