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

    QRect previewRect(option.rect.x() + 2,
                      option.rect.y() + 2,
                      option.rect.height() - 4,
                      option.rect.height() - 4);

    painter->drawImage(previewRect, colorSet->image());

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
        painter->drawImage(previewRect, colorSet->image());
        painter->setPen(option.palette.highlightedText().color());
    } else {
        painter->setBrush(option.palette.text().color());
    }
    painter->drawText(option.rect.x() + previewRect.width() + 5,
                      option.rect.y() + painter->fontMetrics().ascent() + 5,
                      colorSet->name());

    painter->restore();
}

inline QSize KisPaletteListWidget::Private::Delegate::sizeHint(const QStyleOptionViewItem & option,
                                                               const QModelIndex &) const
{
    return option.decorationSize;
}
