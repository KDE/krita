#include <QPointer>
#include <QScopedPointer>
#include <QGridLayout>

#include <kis_icon.h>

#include "KisPaletteListWidget.h"
#include "KoResourceItemChooser.h"
#include "KoResourceServer.h"
#include "KoResourceServerAdapter.h"
#include "KoResourceServerProvider.h"
#include "KoColorSet.h"
#include <ui_WdgPaletteListWidget.h>

struct KisPaletteListWidget::Private
{
    Private(KisPaletteListWidget *a_c)
        : c(a_c)
        , rAdapter(new KoResourceServerAdapter<KoColorSet>(KoResourceServerProvider::instance()->paletteServer()))
        , skeletonWidget(new KoResourceItemChooser(rAdapter, a_c))
    {
        skeletonWidget->hide();
    }
    ~Private()
    { }
    QPointer<KisPaletteListWidget> c;
    QSharedPointer<KoResourceServerAdapter<KoColorSet> > rAdapter;
    /**
     * @brief
     *  widget that holds operations
     * in contrary, palette chooser itself mainly holds the GUI
     **/
    QScopedPointer<KoResourceItemChooser> skeletonWidget;
};

KisPaletteListWidget::KisPaletteListWidget(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui_WdgPaletteListWidget)
    , m_d(new Private(this))
{
    m_ui->setupUi(this);
    m_ui->bnAdd->setIcon(KisIconUtils::loadIcon("list-add"));
    m_ui->bnRemove->setIcon(KisIconUtils::loadIcon("list-remove"));
    m_ui->bnEdit->setIcon(KisIconUtils::loadIcon("edit-rename"));
    m_ui->bnImport->setIcon(KisIconUtils::loadIcon("document-import"));
    m_ui->bnExport->setIcon(KisIconUtils::loadIcon("document-export"));
}

KisPaletteListWidget::~KisPaletteListWidget()
{ }
