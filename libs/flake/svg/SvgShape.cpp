/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "SvgShape.h"


SvgShape::~SvgShape()
{
}

bool SvgShape::saveSvg(SvgSavingContext &/*context*/)
{
    return false;
}

bool SvgShape::loadSvg(const KoXmlElement &/*element*/, SvgLoadingContext &/*context*/)
{
    return false;
}
