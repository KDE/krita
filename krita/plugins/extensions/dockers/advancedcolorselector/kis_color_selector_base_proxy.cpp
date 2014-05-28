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

#include "kis_color_selector_base_proxy.h"

#include "KoColorSpaceRegistry.h"

#include "kis_color_selector_base.h"
#include "kis_display_color_converter.h"


/************* KisColorSelectorBaseProxy *******************************/

KisColorSelectorBaseProxy::~KisColorSelectorBaseProxy()
{
}


/************* KisColorSelectorBaseProxyNoop ***************************/

KisColorSelectorBaseProxyNoop::KisColorSelectorBaseProxyNoop()
{
}

KisColorSelectorBaseProxyNoop::~KisColorSelectorBaseProxyNoop()
{
}

const KoColorSpace* KisColorSelectorBaseProxyNoop::colorSpace() const
{
    return KoColorSpaceRegistry::instance()->rgb8();
}

KisDisplayColorConverter* KisColorSelectorBaseProxyNoop::converter() const
{
    return KisDisplayColorConverter::dumbConverterInstance();
}


/************* KisColorSelectorBaseProxyObject *************************/

KisColorSelectorBaseProxyObject::KisColorSelectorBaseProxyObject(KisColorSelectorBase *parent)
    : m_parent(parent)
{
}

const KoColorSpace* KisColorSelectorBaseProxyObject::colorSpace() const
{
    return m_parent->colorSpace();
}

void KisColorSelectorBaseProxyObject::showColorPreview()
{
    m_parent->showColorPreview();
}

void KisColorSelectorBaseProxyObject::updateColorPreview(const KoColor &color)
{
    m_parent->updateColorPreview(color);
}

void KisColorSelectorBaseProxyObject::updateColor(const KoColor &color, Acs::ColorRole role, bool needsExplicitColorReset)
{
    m_parent->updateColor(color, role, needsExplicitColorReset);
}

KisDisplayColorConverter* KisColorSelectorBaseProxyObject::converter() const
{
    return m_parent->converter();
}
