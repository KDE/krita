#include <QPointer>

#include "KisPaletteListView.h"
#include "KisPaletteListModel.h"
#include "KisPaletteListDelegate.h"

class KisPaletteListView::Private
{
public:
    Private(KisPaletteListView *view)
        : model(new KisPaletteListModel(view))
        , delegate(new KisPaletteListDelegate(view))
        , m_view(view)
    { }
    ~Private()
    {
        delete model;
        delete delegate;
    }
public:
    QPointer<KisPaletteListModel> model;
    QPointer<KisPaletteListDelegate> delegate;
private:
    KisPaletteListView *m_view;
};

KisPaletteListView::~KisPaletteListView()
{
    delete m_d;
}

KisPaletteListView::KisPaletteListView(QWidget *parent)
    : QAbstractItemView(parent)
    , m_d(new Private)
{
     setModel(m_d->model);
}
