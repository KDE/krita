// -*- c++ -*-
/* This file is part of the KDE project

   Original QTextCustomItem is
     Copyright (C) 1999-2000 Trolltech AS.  All rights reserved.
   KoText modifications
     Copyright (C) 2001-2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// This file isn't standalone at this point; it's included by KoRichText.h

/**
 * KoTextCustomItem is the base class for custom items (i.e. special chars)
 * Custom items include:
 * - variables ( KoVariable, kovariable.h )
 * - in kword: inline images ( KWTextImage, kwtextimage.h ) (to be removed)
 * - in kword: anchors, i.e. floating frames ( KWAnchor, kwanchor.h )
 */
class KOTEXT_EXPORT KoTextCustomItem
{
public:
    KoTextCustomItem( KoTextDocument *p );
    virtual ~KoTextCustomItem();
    virtual void draw(QPainter* p, int x, int y, int cx, int cy, int cw, int ch, const QColorGroup& cg, bool selected ) /* = 0*/;

    // Called after the item's paragraph has been formatted
    virtual void finalize() {}

    void move( int x, int y ) { xpos = x; ypos = y; }
    int x() const { return xpos; }
    int y() const { return ypos; }

    // Called when the format of the character is being changed, see KoTextStringChar::setFormat
    virtual void setFormat( KoTextFormat * ) { }

    //virtual void setPainter( QPainter*, bool adjust );

    enum Placement { PlaceInline = 0, PlaceLeft, PlaceRight };
    virtual Placement placement() const { return PlaceInline; }
    bool placeInline() { return placement() == PlaceInline; }

    virtual bool ownLine() const { return false; }
    // Called for "ownline" items
    virtual void resize( int nwidth ) { width = nwidth; }
    virtual void invalidate() {};

    virtual bool isNested() const { return false; }
    virtual int minimumWidth() const { return 0; }
    virtual int widthHint() const { return 0; }
    virtual int ascent() const { return height; }

    virtual QString richText() const { return QString::null; }

    int width;
    int height;

    QRect geometry() const { return QRect( xpos, ypos, width, height ); }

    virtual bool enter( KoTextCursor *, KoTextDocument *&doc, KoTextParag *&parag, int &idx, int &ox, int &oy, bool atEnd = false );
    virtual bool enterAt( KoTextCursor *, KoTextDocument *&doc, KoTextParag *&parag, int &idx, int &ox, int &oy, const QPoint & );
    virtual bool next( KoTextCursor *, KoTextDocument *&doc, KoTextParag *&parag, int &idx, int &ox, int &oy );
    virtual bool prev( KoTextCursor *, KoTextDocument *&doc, KoTextParag *&parag, int &idx, int &ox, int &oy );
    virtual bool down( KoTextCursor *, KoTextDocument *&doc, KoTextParag *&parag, int &idx, int &ox, int &oy );
    virtual bool up( KoTextCursor *, KoTextDocument *&doc, KoTextParag *&parag, int &idx, int &ox, int &oy );

    void setParagraph( KoTextParag * p ) { parag = p; }
    KoTextParag *paragraph() const { return parag; }

    virtual void pageBreak( int /*y*/, KoTextFlow* /*flow*/ ) {}

    KoTextDocument *parent;



    /** The text document in which this customitem is */
    KoTextDocument * textDocument() const { return parent; }

    /** When the user deletes a custom item, it isn't destroyed but
     * moved into the undo/redo history - setDeleted( true )
     * and it can be then copied back from there into the real world - setDeleted( false ). */
    virtual void setDeleted( bool b ) { m_deleted = b; }

    bool isDeleted() const { return m_deleted; }

    /** Called when the item is created or 'deleted' by the user
     * Most custom items don't need to reimplement those, since
     * the custom item is simply moved into the undo/redo history
     * when deleting (or undoing a creation).
     * It is not deleted and re-created later. */
    virtual KCommand * createCommand() { return 0L; }
    virtual KCommand * deleteCommand() { return 0L; }

    /** Save to XML */
    virtual void save( QDomElement& formatElem ) = 0;
    /** Save to Oasis XML */
    virtual void saveOasis( KoXmlWriter& writer, KoSavingContext& context ) const = 0;
    /** Return type of custom item. See DTD for VARIABLE.id docu. */
    virtual int typeId() const = 0;

    /** Reimplement this to calculate the item width
     * It is important to start with "if ( m_deleted ) return;" */
    virtual void resize() {}

    /** Reimplemented by KoVariable to recalculate the value.
     * It exists at the KoTextCustomItem level so that KoTextParag::setCustomItem
     * can call it to set the initial value.
     * This should call always resize(). */
    virtual void recalc() { resize(); }

    /** The index in paragraph(), where this anchor is
     * Slightly slow (does a linear search in the paragraph) */
    int index() const;

    /** The formatting given to this 'special' character
     * Slightly slow (does a linear search in the paragraph) */
    KoTextFormat * format() const;

    /**
     * All coordinates are in pixels.
     */
    virtual void drawCustomItem(QPainter* p, int x, int y, int wpix, int hpix, int ascentpix, int cx, int cy, int cw, int ch, const QColorGroup& cg, bool selected, int offset,  bool drawingShadow) = 0;

protected:
    bool m_deleted;

protected:
    int xpos;
    int ypos;
private:
    KoTextParag *parag;
};
