#ifndef KISPALETTELISTWIDGET_P_H
#define KISPALETTELISTWIDGET_P_H

#include <QAbstractItemDelegate>
#include <QListView>
#include <QAbstractListModel>
#include <QPointer>

#include "KisPaletteListWidget.h"
#include "KoResourceModel.h"
#include "KoResourceItemView.h"
#include "KoResourceItemChooser.h"
#include "KoResourceServer.h"
#include "KoResourceServerAdapter.h"
#include "KoResourceServerProvider.h"
#include "KoColorSet.h"

struct KisPaletteListWidget::Private
{
    class Delegate;
    class View;
    class Model;
    Private(KisPaletteListWidget *);
    virtual ~Private();

    QPointer<KisPaletteListWidget> c;
    QSharedPointer<KoResourceServerAdapter<KoColorSet> > rAdapter;
    QSharedPointer<KoResourceItemChooser> itemChooser;

    QScopedPointer<KoResourceModel> model;
    QScopedPointer<Delegate> delegate;
};

class KisPaletteListWidget::Private::Delegate : public QAbstractItemDelegate
{
public:
    Delegate(QObject *);
    virtual ~Delegate();
    void paint(QPainter * painter,
               const QStyleOptionViewItem & option,
               const QModelIndex & index) const override;
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex &) const override;
};

KisPaletteListWidget::Private::Private(KisPaletteListWidget *a_c)
    : c(a_c)
    , rAdapter(new KoResourceServerAdapter<KoColorSet>(KoResourceServerProvider::instance()->paletteServer()))
    , itemChooser(new KoResourceItemChooser(rAdapter, a_c))
    , model(new KoResourceModel(rAdapter, a_c))
    , delegate(new Delegate(a_c))
{
    itemChooser->showButtons(false);
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
    painter->drawText(option.rect.x() + previewRect.width() + 10,
                      option.rect.y() + painter->fontMetrics().ascent() + 5,
                      colorSet->name());

    painter->restore();
}

inline QSize KisPaletteListWidget::Private::Delegate::sizeHint(const QStyleOptionViewItem & option,
                                                               const QModelIndex &) const
{
    return option.decorationSize;
}

#endif // KISPALETTELISTWIDGET_P_H
