/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
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

#include <QPen>
#include <QPainter>

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
    virtual qreal beamLineWidth();
    virtual void renderNoteHead(QPainter& painter, qreal x, qreal y, MusicCore::Duration duration, const QColor& color = Qt::black);
    virtual void renderRest(QPainter& painter, qreal x, qreal y, MusicCore::Duration duration, const QColor& color = Qt::black);
    virtual void renderClef(QPainter& painter, qreal x, qreal y, MusicCore::Clef::ClefShape shape, const QColor& color = Qt::black);
    virtual void renderAccidental(QPainter& painter, qreal x, qreal y, int accidental, const QColor& color = Qt::black);
    virtual void renderTimeSignatureNumber(QPainter& painter, qreal x, qreal y, qreal w, int number, const QColor& color = Qt::black);
    virtual void renderNoteFlags(QPainter& painter, qreal x, qreal y, MusicCore::Duration duration, bool stemsUp, const QColor& color = Qt::black);
    /**
     * Render text either as text or as path, as specified by the textAsPath value
     */
    virtual void renderText(QPainter& painter, qreal x, qreal y, const QString& text);
    /**
     * Whether to render text as paths. Default value is false (render text as text)
     */
    virtual void setTextAsPath(bool drawTextAsPath);
    /**
     * Whether to render text as paths
     */
    virtual bool textAsPath() const;
private:
    QPen m_staffLinePen, m_stemPen, m_noteDotPen;
    QFont m_font;
    bool m_textAsPath;
};

#endif // MUSIC_STYLE_H
