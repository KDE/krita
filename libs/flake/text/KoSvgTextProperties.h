/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KOSVGTEXTPROPERTIES_H
#define KOSVGTEXTPROPERTIES_H

#include "kritaflake_export.h"

#include <QScopedPointer>
#include <QVariant>
#include <QList>

class SvgLoadingContext;



/**
 * KoSvgTextProperties represents the text attributes defined in SVG DOM tree
 *
 * There is a limitation in flake: it doesn't support the inheritance of shape
 * properties: every shape stores all the properties that were defined at the
 * loading/creation stage. KoSvgTextProperties allows the user to compare
 * the properties of the two shapes and distinguish, which properties were
 * inherited by text shape, and which are its own. It is needed to generate a
 * correct and clean SVG/markup code that can be edited by the user easily.
 * Otherwise, every \<tspan\> block will contain the full list of 20+ attributes,
 * which are not interesting for the user, since they are inherited or default.
 *
 * To achieve the goal, KoSvgTextProperties wraps all the SVG attributes into a
 * map of QVariants. When the user need to find a set of unique properties
 * of the shape, it iterates through the map and compares values with standard
 * QVariant-based comparison operator. If the property value in a child and a
 * parent is not the same, then it is not inherited.
 */
class KRITAFLAKE_EXPORT KoSvgTextProperties
{
public:
    /**
     * Defines a set of supported properties. See SVG 1.1 for details.
     */
    enum PropertyId {
        WritingModeId,
        DirectionId,
        UnicodeBidiId,
        TextAnchorId,
        DominantBaselineId,
        AlignmentBaselineId,
        BaselineShiftModeId,
        BaselineShiftValueId,
        KerningId,
        GlyphOrientationVerticalId,
        GlyphOrientationHorizontalId,
        LetterSpacingId,
        WordSpacingId,

        FontFamiliesId,
        FontStyleId,
        FontIsSmallCapsId,
        FontStretchId,
        FontWeightId,
        FontSizeId,
        FontSizeAdjustId,
        TextDecorationId,

        FillId,
        StrokeId
    };

public:

    KoSvgTextProperties();
    ~KoSvgTextProperties();

    KoSvgTextProperties(const KoSvgTextProperties &rhs);
    KoSvgTextProperties& operator=(const KoSvgTextProperties &rhs);

    /**
     * Set the property \p id to \p value
     */
    void setProperty(PropertyId id, const QVariant &value);

    /**
     * Check if property \p id is present in this properties set
     */
    bool hasProperty(PropertyId id) const;

    /**
     * Return the value of property \p id. If the property doesn't exist in
     * the shape, return \p defaultValue instead.
     */
    QVariant property(PropertyId id, const QVariant &defaultValue = QVariant()) const;

    /**
     * Remove property \p id from the set
     */
    void removeProperty(PropertyId id);

    /**
     * Return the value of property \p id. If the property doesn't exist in the
     * shape, return the default value define in SVG 1.1.
     */
    QVariant propertyOrDefault(PropertyId id) const;

    /**
     * Return a list of properties contained in this set
     */
    QList<PropertyId> properties() const;

    /**
     * Return true if the set contains no properties
     */
    bool isEmpty() const;

    /**
     * Reset all non-inheritable properties to default values. The set of
     * non-inheritable properties is define by SVG 1.1. Used by the loading
     * code for resetting state automata's properties on entering a \<tspan\>.
     */
    void resetNonInheritableToDefault();


    /**
     * Apply properties from the parent shape. The property is set **iff** the
     * property is inheritable according to SVG and this set does not define
     * it.
     */
    void inheritFrom(const KoSvgTextProperties &parentProperties);

    /**
     * Return true if the property \p id is inherited from \p parentProperties.
     * The property is considered "inherited" **iff* it is inheritable
     * according to SVG and the parent defined the same property with the same
     * value.
     */
    bool inheritsProperty(PropertyId id, const KoSvgTextProperties &parentProperties) const;

    /**
     * Return a set of properties that ar **not** inherited from \p
     * parentProperties. The property is considered "inherited" **iff* it is
     * inheritable according to SVG and the parent defined the same property
     * with the same value.
     */
    KoSvgTextProperties ownProperties(const KoSvgTextProperties &parentProperties) const;

    /**
     * @brief parseSvgTextAttribute add a property according to an XML attribute value.
     * @param context shared loading context
     * @param command XML attribute name
     * @param value attribute value
     *
     * @see supportedXmlAttributes for a list of supported attributes
     */
    void parseSvgTextAttribute(const SvgLoadingContext &context, const QString &command, const QString &value);

    /**
     * Convert all the properties of the set into a map of XML attribute/value
     * pairs.
     */
    QMap<QString,QString> convertToSvgTextAttributes() const;

    /**
     * Return a list of supported XML attribute names (defined in SVG)
     */
    static QStringList supportedXmlAttributes();

    /**
     * Return a static object that defines default values for all the supported
     * properties according to SVG
     */
    static const KoSvgTextProperties& defaultProperties();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KOSVGTEXTPROPERTIES_H
