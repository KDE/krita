/*
 *  SPDX-FileCopyrightText: 2018 Victor Wåhlström <victor.wahlstrom@initiali.se>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KritaDesignerPluginCollection.h"

#include "KisColorSpaceSelectorPlugin.h"
#include "KisGradientSliderPlugin.h"

KritaDesignerPluginCollection::KritaDesignerPluginCollection(QObject *parent)
    : QObject(parent)
{
    m_widgets.append(new KisColorSpaceSelectorPlugin(this));
    m_widgets.append(new KisGradientSliderPlugin(this));
}

QList<QDesignerCustomWidgetInterface*> KritaDesignerPluginCollection::customWidgets() const
{
    return m_widgets;
}
