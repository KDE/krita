#include "KoBookmark.h"

#include <KoShape.h>

#include <QTextDocument>
#include <QTextInlineObject>
#include <QTextList>
#include <QTextBlock>
#include <QTextCursor>

#include <KDebug>

class KoBookmark::Private {
public:
    Private(KoShape *s) : document(0), shape(s) /*, cursorPosition(0), chapterPosition(-1), pageNumber(1)*/ { }
    const QTextDocument *document;
    int posInDocument;
    KoShape *shape;
    KoBookmark *endBookmark;
    bool selection;
};

KoBookmark::KoBookmark(KoShape *shape)
    : KoInlineObject(false),
    d(new Private(shape))
{
    d->selection = false;
    d->endBookmark = 0;
}

KoBookmark::~KoBookmark()
{
    delete d;
}

void KoBookmark::updatePosition(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format) {
    Q_UNUSED(object);
    Q_UNUSED(format);
    d->document = document;
    d->posInDocument = posInDocument;
}

void KoBookmark::resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd) {
    Q_UNUSED(object);
    Q_UNUSED(pd);
    Q_UNUSED(format);
    Q_UNUSED(document);
    Q_UNUSED(posInDocument);
}

void KoBookmark::paint (QPainter &, QPaintDevice *, const QTextDocument *, const QRectF &, QTextInlineObject , int , const QTextCharFormat &) {
    // nothing to paint.
}

void KoBookmark::setEndBookmark(KoBookmark *bookmark) {
    d->endBookmark = bookmark;
    d->selection = true;
}

KoBookmark *KoBookmark::endBookmark() {
    return d->endBookmark;
}

KoShape *KoBookmark::shape() {
    return d->shape;
}

int KoBookmark::position() {
    return d->posInDocument;
}

bool KoBookmark::hasSelection() {
    return d->selection;
}

