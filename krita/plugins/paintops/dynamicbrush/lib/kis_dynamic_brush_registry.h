/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_DYNAMIC_BRUSH_REGISTRY_H_
#define _KIS_DYNAMIC_BRUSH_REGISTRY_H_

#include <KoGenericRegistry.h>

class KisDynamicBrush;

class KisDynamicBrushRegistry : public KoGenericRegistry<KisDynamicBrush *> {

  protected:
    void init();
  public:
    /**
     * @return an instance of the KisDynamicBrushRegistry
     * Creates an instance if that has never happened before and returns the singleton instance.
     */
    static KisDynamicBrushRegistry* instance();
    /**
     * @return the currently selected dynamic brush
     */
    inline KisDynamicBrush* current() { return m_current; }
    inline void setCurrent(KisDynamicBrush* c) { m_current = c; }
  private:
    static KisDynamicBrushRegistry *singleton;
    KisDynamicBrush* m_current;
};

#endif
