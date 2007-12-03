/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
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
#ifndef MUSIC_STYLE_H
#define MUSIC_STYLE_H

#include "core/Global.h"
#include "core/Clef.h"

#include <QtGui/QPen>
#include <QtGui/QPainter>

/**
 * This class contains various methods that define how music is rendered. Currently all hardcoded
 * implementations, but in the future this class would become pure virtual, with various implementations.
 */
class MusicStyle {
public:
    MusicStyle();
    virtual ~MusicStyle();
    virtual QPen staffLinePen(const QColor& color = Qt::black);
    virtual QPen stemPen(const QColor& color = Qt::black);
    virtual QPen noteDotPen(const QColor& color = Qt::black);
    virtual double beamLineWidth();
    virtual void renderNoteHead(QPainter& painter, double x, double y, MusicCore::Duration duration, const QColor& color = Qt::black);
    virtual void renderRest(QPainter& painter, double x, double y, MusicCore::Duration duration, const QColor& color = Qt::black);
    virtual void renderClef(QPainter& painter, double x, double y, MusicCore::Clef::ClefShape shape, const QColor& color = Qt::black);
    virtual void renderAccidental(QPainter& painter, double x, double y, int accidental, const QColor& color = Qt::black);
    virtual void renderTimeSignatureNumber(QPainter& painter, double x, double y, double w, int number, const QColor& color = Qt::black);
    virtual void renderNoteFlags(QPainter& painter, double x, double y, MusicCore::Duration duration, bool stemsUp, const QColor& color = Qt::black);
private:
    QPen m_staffLinePen, m_stemPen, m_noteDotPen;
    QFont m_font;
};

#endif // MUSIC_STYLE_H
