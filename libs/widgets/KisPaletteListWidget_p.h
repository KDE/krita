#ifndef KISPALETTELISTWIDGET_P_H
#define KISPALETTELISTWIDGET_P_H

#include <QAbstractItemDelegate>
#include <QListView>
#include <QAbstractListModel>
#include <QPointer>

#include "KisPaletteListWidget.h"
#include "KoResourceModel.h"
#include "KoResourceItemView.h"
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

#endif // KISPALETTELISTWIDGET_P_H
