/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef SWATCH_H
#define SWATCH_H

#include "ManagedColor.h"

#include "kritalibkis_export.h"
#include "libkis.h"

class KisSwatch;

/**
 * @brief The Swatch class is a thin wrapper around the KisSwatch class.
 *
 * A Swatch is a single color that is part of a palette, that has a name
 * and an id. A Swatch color can be a spot color.
 */
class KRITALIBKIS_EXPORT Swatch
{
private:
    friend class Palette;
    friend class PaletteView;
    Swatch(const KisSwatch &kisSwatch);
public:
    Swatch();
    virtual ~Swatch();
    Swatch(const Swatch &rhs);
    Swatch &operator=(const Swatch &rhs);

    QString name() const;
    void setName(const QString &name);

    QString id() const;
    void setId(const QString &id);

    ManagedColor *color() const;
    void setColor(ManagedColor *color);

    bool spotColor() const;
    void setSpotColor(bool spotColor);

    bool isValid() const;

private:
    friend class Palette;
    KisSwatch kisSwatch() const;

    struct Private;
    Private *const d;
};

#endif // SWATCH_H
