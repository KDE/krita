#ifndef KISPALETTELISTWIDGET_P_H
#define KISPALETTELISTWIDGET_P_H

#include <QAbstractItemDelegate>
#include <QListView>
#include <QAbstractListModel>
#include <QPointer>
#include <QCheckBox>

#include "KisPaletteListWidget.h"
#include "KoResourceModel.h"
#include "KoResourceItemView.h"
#include "KoResourceItemChooser.h"
#include "KoResourceServer.h"
#include "KoResourceServerAdapter.h"
#include "KoResourceServerProvider.h"
#include "KoColorSet.h"

struct KisPaletteListWidgetPrivate
{
    class View;
    class Delegate;
    class Model;
    KisPaletteListWidgetPrivate(KisPaletteListWidget *);
    virtual ~KisPaletteListWidgetPrivate();

    QPointer<KisPaletteListWidget> c;
    QSharedPointer<KoResourceServerAdapter<KoColorSet> > rAdapter;
    QSharedPointer<KoResourceItemChooser> itemChooser;

    QScopedPointer<Model> model;
    QScopedPointer<Delegate> delegate;
};

class KisPaletteListWidgetPrivate::Delegate : public QAbstractItemDelegate
{
public:
    Delegate(QObject *);
    virtual ~Delegate();
    void paint(QPainter * painter,
               const QStyleOptionViewItem & option,
               const QModelIndex & index) const override;
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex &) const override;
};

class KisPaletteListWidgetPrivate::Model : public KoResourceModel
{
public:
    Model(const QSharedPointer<KoResourceServerAdapter<KoColorSet> > &rAdapter, QObject *parent = Q_NULLPTR)
        : KoResourceModel(rAdapter, parent)
    { }
    ~Model() override { }

    Qt::ItemFlags flags(const QModelIndex &index) const override
    { return KoResourceModel::flags(index) | Qt::ItemIsUserCheckable; }
};

KisPaletteListWidgetPrivate::KisPaletteListWidgetPrivate(KisPaletteListWidget *a_c)
    : c(a_c)
    , rAdapter(new KoResourceServerAdapter<KoColorSet>(KoResourceServerProvider::instance()->paletteServer()))
    , itemChooser(new KoResourceItemChooser(rAdapter, a_c))
    , model(new Model(rAdapter, a_c))
    , delegate(new Delegate(a_c))
{
    itemChooser->showButtons(false);
    model->setColumnCount(1);
}

KisPaletteListWidgetPrivate::~KisPaletteListWidgetPrivate()
{  }

KisPaletteListWidgetPrivate::Delegate::Delegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{  }

KisPaletteListWidgetPrivate::Delegate::~Delegate()
{  }

void KisPaletteListWidgetPrivate::Delegate::paint(QPainter * painter,
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

inline QSize KisPaletteListWidgetPrivate::Delegate::sizeHint(const QStyleOptionViewItem & option,
                                                               const QModelIndex &) const
{
    return option.decorationSize;
}



#endif // KISPALETTELISTWIDGET_P_H
