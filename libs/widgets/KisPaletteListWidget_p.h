#ifndef KISPALETTELISTWIDGET_P_H
#define KISPALETTELISTWIDGET_P_H

#include <QAbstractItemDelegate>
#include <QListView>
#include <QAbstractListModel>
#include <QPointer>
#include <QCheckBox>
#include <QAction>

#include "KisPaletteListWidget.h"
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

    bool allowModification;

    QPointer<KisPaletteListWidget> c;

    QSharedPointer<KoResourceServerAdapter<KoColorSet> > rAdapter;
    QSharedPointer<KoResourceItemChooser> itemChooser;

    QScopedPointer<Delegate> delegate;

    QScopedPointer<QAction> actAdd;
    QScopedPointer<QAction> actImport;
    QScopedPointer<QAction> actExport;
    QScopedPointer<QAction> actModify;
    QScopedPointer<QAction> actRemove;
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
#endif // KISPALETTELISTWIDGET_P_H
