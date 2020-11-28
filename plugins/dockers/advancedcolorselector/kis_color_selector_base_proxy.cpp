/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
