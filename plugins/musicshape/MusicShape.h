/* This file is part of the KDE project
 * Copyright 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
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
#ifndef MUSIC_SHAPE
#define MUSIC_SHAPE

#include <KoShape.h>
#include <KoFrameShape.h>
#include <kurl.h>
#define MusicShapeId "MusicShape"

namespace MusicCore {
    class Sheet;
}

class MusicRenderer;
class MusicStyle;
class Engraver;

class MusicShape : public KoShape, public KoFrameShape
{
public:
    MusicShape();
    virtual ~MusicShape();
    /// reimplemented
    virtual void paint( QPainter& painter, const KoViewConverter& converter, KoShapePaintingContext &paintcontext);
    void constPaint( QPainter& painter, const KoViewConverter& converter ) const;

    /// reimplemented
    virtual void setSize( const QSizeF &newSize );

    /// reimplemented
    virtual void saveOdf( KoShapeSavingContext & context ) const;
    // reimplemented
    virtual bool loadOdf( const KoXmlElement & element, KoShapeLoadingContext &context );

    MusicCore::Sheet* sheet();
    void setSheet(MusicCore::Sheet* sheet, int firstSystem);
    int firstSystem() const;
    void setFirstSystem(int system);
    int lastSystem() const;
    int firstBar() const;
    int lastBar() const;
    MusicRenderer* renderer();
    MusicStyle* style();
    void engrave(bool engraveBars=true);

    MusicShape* successor() { return m_successor; }
    MusicShape* predecessor() { return m_predecessor; }
protected:
    // reimplemented
    virtual bool loadOdfFrameElement( const KoXmlElement & element, KoShapeLoadingContext & context );
private:
    MusicCore::Sheet* m_sheet;
    int m_firstSystem;
    int m_lastSystem;
    MusicStyle* m_style;
    Engraver* m_engraver;
    MusicRenderer* m_renderer;
    MusicShape* m_successor;
    MusicShape* m_predecessor;
};


#endif // MUSIC_SHAPE
