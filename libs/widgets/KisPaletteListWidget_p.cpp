#include "KisPaletteListWidget_p.h"

KisPaletteListWidget::Private::Private(KisPaletteListWidget *a_c)
    : c(a_c)
    , rAdapter(new KoResourceServerAdapter<KoColorSet>(KoResourceServerProvider::instance()->paletteServer()))
    , model(new KoResourceModel(rAdapter, a_c))
    , delegate(new Delegate(a_c))
{
    model->setColumnCount(1);
}

KisPaletteListWidget::Private::~Private()
{  }

KisPaletteListWidget::Private::Delegate::Delegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{  }

KisPaletteListWidget::Private::Delegate::~Delegate()
{  }

void KisPaletteListWidget::Private::Delegate::paint(QPainter * painter,
                                                    const QStyleOptionViewItem & option,
                                                    const QModelIndex & index) const
{
    painter->save();
    if (!index.isValid())
        return;

    KoResource* resource = static_cast<KoResource*>(index.internalPointer());
    KoColorSet* colorSet = static_cast<KoColorSet*>(resource);

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
        painter->setPen(option.palette.highlightedText().color());
    }
    else {
        painter->setBrush(option.palette.text().color());
    }
    painter->drawText(option.rect.x() + 5, option.rect.y() + painter->fontMetrics().ascent() + 5, colorSet->name());

    int size = 7;
    for (quint32 i = 0; i < colorSet->nColors() && i*size < (quint32)option.rect.width(); i++) {
        QRect rect(option.rect.x() + i*size, option.rect.y() + option.rect.height() - size, size, size);
        painter->fillRect(rect, colorSet->getColorGlobal(i).color().toQColor());
    }

    painter->restore();
}

inline QSize KisPaletteListWidget::Private::Delegate::sizeHint(const QStyleOptionViewItem & option,
                                                               const QModelIndex &) const
{
    return option.decorationSize;
}
