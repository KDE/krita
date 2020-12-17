/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    ~KisColorSelectorBaseProxyNoop() override;

    const KoColorSpace* colorSpace() const override;

    void showColorPreview() override {}

    void updateColorPreview(const KoColor &color) override {
        Q_UNUSED(color);
    }

    void updateColor(const KoColor &color, Acs::ColorRole role, bool needsExplicitColorReset) override {
        Q_UNUSED(color);
        Q_UNUSED(role);
        Q_UNUSED(needsExplicitColorReset);
    }

    KisDisplayColorConverter* converter() const override;
};


class KisColorSelectorBase;

class KisColorSelectorBaseProxyObject : public KisColorSelectorBaseProxy
{
public:
    KisColorSelectorBaseProxyObject(KisColorSelectorBase *parent);

    const KoColorSpace* colorSpace() const override;
    void showColorPreview() override;
    void updateColorPreview(const KoColor &color) override;
    void updateColor(const KoColor &color, Acs::ColorRole role, bool needsExplicitColorReset) override;
    KisDisplayColorConverter* converter() const override;

private:
    KisColorSelectorBase *m_parent;
};

#endif /* __KIS_COLOR_SELECTOR_BASE_PROXY_H */
