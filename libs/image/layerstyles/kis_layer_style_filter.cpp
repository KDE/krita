/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_layer_style_filter.h"

#include <KoID.h>


struct Q_DECL_HIDDEN KisLayerStyleFilter::Private
{
    KoID id;
};

KisLayerStyleFilter::KisLayerStyleFilter(const KoID &id)
    : m_d(new Private)
{
    m_d->id = id;
}

KisLayerStyleFilter::KisLayerStyleFilter(const KisLayerStyleFilter &rhs)
    : KisShared(),
      m_d(new Private)
{
    m_d->id = rhs.m_d->id;
}

KisLayerStyleFilter::~KisLayerStyleFilter()
{
}

QString KisLayerStyleFilter::id() const
{
    return m_d->id.id();
}
