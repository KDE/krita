/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "KisAnimatedBrushAnnotation.h"

#include <QByteArray>
#include <QBuffer>

#include <kis_pipebrush_parasite.h>

KisAnimatedBrushAnnotation::KisAnimatedBrushAnnotation(const KisPipeBrushParasite &parasite)
    : KisAnnotation("ImagePipe Parasite",
                    i18n("Brush selection information for animated brushes"),
                    QByteArray())
{
    QBuffer buf(&m_annotation);
    buf.open(QBuffer::WriteOnly);
    parasite.saveToDevice(&buf);
    buf.close();
}
