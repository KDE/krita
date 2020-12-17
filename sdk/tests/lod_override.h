/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __LOD_OVERRIDE_H
#define __LOD_OVERRIDE_H

#include "kis_default_bounds_base.h"


namespace TestUtil {

class LodOverride
{
private:

    class LodDefaultBounds : public KisDefaultBoundsBase
    {
    public:
        LodDefaultBounds(int lod, KisDefaultBoundsBaseSP parent)
            : m_lod(lod), m_parent(parent)
        {
        }

        QRect bounds() const override {
            return m_parent->bounds();
        }

        virtual QRect imageBorderRect() const override {
            return m_parent->imageBorderRect();
        }

        bool wrapAroundMode() const override {
            return m_parent->wrapAroundMode();
        }

        int currentLevelOfDetail() const override {
            return m_lod;
        }

        int currentTime() const override {
            return m_parent->currentTime();
        }

        bool externalFrameActive() const override {
            return m_parent->externalFrameActive();
        }

        KisDefaultBoundsBaseSP parent() const {
            return m_parent;
        }

        void * sourceCookie() const override {
            return 0;
        }

    private:
        int m_lod;
        KisDefaultBoundsBaseSP m_parent;
    };

public:
    explicit LodOverride(int lod, KisImageSP image)
        : m_lod(lod), m_image(image)
    {
        overrideBounds(m_image->root(), OverrideDevice(m_lod));
    }

    ~LodOverride()
    {
        overrideBounds(m_image->root(), RestoreDevice());
    }

private:

    template <class OverrideOp>
    void overrideBounds(KisNodeSP root, OverrideOp op) {
        op(root->paintDevice());

        if (root->original() != root->paintDevice()) {
            op(root->original());
        }

        if (root->projection() != root->original()) {
            op(root->projection());
        }

        KisNodeSP node = root->firstChild();
        while (node) {
            overrideBounds(node, op);
            node = node->nextSibling();
        }
    }

    struct OverrideDevice {
        OverrideDevice(int lod) : m_lod(lod) {}

        void operator() (KisPaintDeviceSP device) {
            if (!device) return;

            LodDefaultBounds *bounds = dynamic_cast<LodDefaultBounds*>(device->defaultBounds().data());
            if (bounds) return;

            device->setDefaultBounds(new LodDefaultBounds(m_lod, device->defaultBounds()));
        }

        int m_lod;
    };

    struct RestoreDevice {
        void operator() (KisPaintDeviceSP device) {
            if (!device) return;

            LodDefaultBounds *bounds = dynamic_cast<LodDefaultBounds*>(device->defaultBounds().data());
            if (!bounds) return;

            device->setDefaultBounds(bounds->parent());
        }
    };

private:
    int m_lod;
    KisImageSP m_image;
};

}

#endif /* __LOD_OVERRIDE_H */
