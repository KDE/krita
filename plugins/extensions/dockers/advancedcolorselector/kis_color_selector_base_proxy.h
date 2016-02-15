/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_COLOR_SELECTOR_BASE_PROXY_H
#define __KIS_COLOR_SELECTOR_BASE_PROXY_H

#include "kis_acs_types.h"

class KoColor;
class KoColorSpace;
class KisDisplayColorConverter;

/**
 * A proxy interface for accessing high-level KisColorSelectorBase
 * structure by lower level classes.
 *
 * This small abstractions makes the interface clear and hides all the
 * non-relevant features from the clients
 */
class KisColorSelectorBaseProxy
{
public:
    virtual ~KisColorSelectorBaseProxy();

    virtual const KoColorSpace* colorSpace() const = 0;
    virtual void showColorPreview() = 0;
    virtual void updateColorPreview(const KoColor &color) = 0;
    virtual void updateColor(const KoColor &color, Acs::ColorRole role, bool needsExplicitColorReset) = 0;
    virtual KisDisplayColorConverter* converter() const = 0;
};

class KisColorSelectorBaseProxyNoop : public KisColorSelectorBaseProxy
{
public:
    KisColorSelectorBaseProxyNoop();
    ~KisColorSelectorBaseProxyNoop();

    const KoColorSpace* colorSpace() const;

    void showColorPreview() {}

    void updateColorPreview(const KoColor &color) {
        Q_UNUSED(color);
    }

    void updateColor(const KoColor &color, Acs::ColorRole role, bool needsExplicitColorReset) {
        Q_UNUSED(color);
        Q_UNUSED(role);
        Q_UNUSED(needsExplicitColorReset);
    }

    KisDisplayColorConverter* converter() const;
};


class KisColorSelectorBase;

class KisColorSelectorBaseProxyObject : public KisColorSelectorBaseProxy
{
public:
    KisColorSelectorBaseProxyObject(KisColorSelectorBase *parent);

    const KoColorSpace* colorSpace() const;
    void showColorPreview();
    void updateColorPreview(const KoColor &color);
    void updateColor(const KoColor &color, Acs::ColorRole role, bool needsExplicitColorReset);
    KisDisplayColorConverter* converter() const;

private:
    KisColorSelectorBase *m_parent;
};

#endif /* __KIS_COLOR_SELECTOR_BASE_PROXY_H */
