/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
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
