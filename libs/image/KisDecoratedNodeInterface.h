/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISDECORATEDNODEINTERFACE_H
#define KISDECORATEDNODEINTERFACE_H

#include "kritaimage_export.h"

/**
 * A special interface for layer that have a "decorations",
 * that is, a data that is written into layer stack, but is
 * not a part of user's image.
 */
class KRITAIMAGE_EXPORT KisDecoratedNodeInterface
{
public:
    virtual ~KisDecoratedNodeInterface();

    /**
     * \return true is the layer is allowed to write
     * its decorative information into the stack. The
     * value should be "true" by default.
     */
    virtual bool decorationsVisible() const = 0;

    /**
     * Enable or disable writing decorative information into
     * layer stack.
     */
    virtual void setDecorationsVisible(bool value, bool update) = 0;

    /**
     * Convenience override for setDecorationsVisible()
     */
    void setDecorationsVisible(bool value);
};

#endif // KISDECORATEDNODEINTERFACE_H
