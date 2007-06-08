#ifndef KOBOOKMARK_H
#define KOBOOKMARK_H

#include "KoInlineObject.h"
#include "kotext_export.h"

class KoShape;

class KOTEXT_EXPORT KoBookmark : public KoInlineObject
{
public:
    /// constructor
    KoBookmark(KoShape *s);
    virtual ~KoBookmark();

    /// reimplemented from super
    virtual void updatePosition(const QTextDocument *document, QTextInlineObject object,
            int posInDocument, const QTextCharFormat &format);
    /// reimplemented from super
    virtual void resize(const QTextDocument *document, QTextInlineObject object,
            int posInDocument, const QTextCharFormat &format, QPaintDevice *pd);
    /// reimplemented from super
    virtual void paint (QPainter &painter, QPaintDevice *pd, const QTextDocument *document,
           const QRectF &rect, QTextInlineObject object, int posInDocument, const QTextCharFormat &format);

    void setEndBookmark(KoBookmark *bookmark);
    KoBookmark *endBookmark();
    KoShape *shape();
    int position();
    bool hasSelection();
private:
    class Private;
    Private *const d;
};

#endif

