/*
 * Copyright (C) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kra_converter.h"

#include <QApplication>

#include <QFileInfo>

#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KoColorSpaceRegistry.h>
#include <KisDocument.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include "kis_png_converter.h"

KraConverter::KraConverter(KisDocument *doc)
    : m_doc(doc)
    , m_stop(false)
{
}

KraConverter::~KraConverter()
{
}

KisImageBuilder_Result KraConverter::buildImage(QIODevice *io)
{
    return KisImageBuilder_RESULT_OK;
}

KisImageWSP KraConverter::image()
{
    return m_image;
}

vKisNodeSP KraConverter::activeNodes()
{
    return m_activeNodes;
}

KisImageBuilder_Result KraConverter::buildFile(QIODevice *io, KisImageWSP image, vKisNodeSP activeNodes)
{
    return KisImageBuilder_RESULT_OK;
}

void KraConverter::cancel()
{
    m_stop = true;
}


