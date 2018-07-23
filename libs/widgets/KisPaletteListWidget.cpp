#include <QPointer>
#include <QScopedPointer>
#include <QGridLayout>

#include <kis_icon.h>

#include <ui_WdgPaletteListWidget.h>
#include "KisPaletteListWidget.h"
#include "KisPaletteListWidget_p.h"

/*
bool KisPaletteView::addGroupWithDialog()
{
    KoDialog *window = new KoDialog();
    window->setWindowTitle(i18nc("@title:window","Add a new group"));
    QFormLayout *editableItems = new QFormLayout();
    window->mainWidget()->setLayout(editableItems);
    QLineEdit *lnName = new QLineEdit();
    editableItems->addRow(i18nc("Name for a group", "Name"), lnName);
    lnName->setText(i18nc("Part of default name for a new group", "Color Group")+""+QString::number(m_d->model->colorSet()->getGroupNames().size()+1));
    if (window->exec() == KoDialog::Accepted) {
        QString groupName = lnName->text();
        m_d->model->addGroup(groupName);
        m_d->model->colorSet()->save();
        return true;
    }
    return false;
}
*/


KisPaletteListWidget::KisPaletteListWidget(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui_WdgPaletteListWidget)
    , m_d(new KisPaletteListWidgetPrivate(this))
{
    m_d->actAdd.reset(new QAction(KisIconUtils::loadIcon("list-add"),
                                  i18n("Add a new palette")));
    m_d->actRemove.reset(new QAction(KisIconUtils::loadIcon("list-remove"),
                                     i18n("Remove current palette")));
    m_d->actModify.reset(new QAction(KisIconUtils::loadIcon("edit-rename"),
                                     i18n("Rename choosen palette")));
    m_d->actImport.reset(new QAction(KisIconUtils::loadIcon("document-import"),
                                     i18n("Import a new palette from file")));
    m_d->actExport.reset(new QAction(KisIconUtils::loadIcon("document-export"),
                                     i18n("Export current palette to file")));
    m_d->model->setColumnCount(1);

    m_ui->setupUi(this);
    m_ui->bnAdd->setDefaultAction(m_d->actAdd.data());
    m_ui->bnRemove->setDefaultAction(m_d->actRemove.data());
    m_ui->bnEdit->setDefaultAction(m_d->actModify.data());
    m_ui->bnImport->setDefaultAction(m_d->actImport.data());
    m_ui->bnExport->setDefaultAction(m_d->actExport.data());

    connect(m_d->actAdd.data(), SIGNAL(triggered()), SLOT(slotAdd()));
    connect(m_d->actRemove.data(), SIGNAL(triggered()), SLOT(slotRemove()));
    connect(m_d->actModify.data(), SIGNAL(triggered()), SLOT(slotModify()));
    connect(m_d->actImport.data(), SIGNAL(triggered()), SLOT(slotImport()));
    connect(m_d->actExport.data(), SIGNAL(triggered()), SLOT(slotExport()));

    m_d->itemChooser->setItemDelegate(m_d->delegate.data());
    m_d->itemChooser->setRowHeight(40);
    m_d->itemChooser->setColumnCount(1);
    m_d->itemChooser->showButtons(false);
    m_ui->viewPalette->setLayout(new QHBoxLayout(m_ui->viewPalette));
    m_ui->viewPalette->layout()->addWidget(m_d->itemChooser.data());

    connect(m_d->itemChooser.data(), SIGNAL(resourceSelected(KoResource *)), SLOT(slotPaletteResourceSelected(KoResource*)));
}

KisPaletteListWidget::~KisPaletteListWidget()
{ }

void KisPaletteListWidget::slotPaletteResourceSelected(KoResource *r)
{
    emit sigPaletteSelected(static_cast<KoColorSet*>(r));
}

void KisPaletteListWidget::slotAdd()
{
    KoColorSet *newColorSet = new KoColorSet("New palette");
    newColorSet->setPaletteType(KoColorSet::GPL);
    newColorSet->setIsGlobal(false);
    m_d->rAdapter->addResource(newColorSet);
    m_d->itemChooser->setCurrentResource(newColorSet);

    emit sigPaletteListChanged();
}

void KisPaletteListWidget::slotRemove()
{
    if (m_d->itemChooser->currentResource()) {
        m_d->rAdapter->removeResource(m_d->itemChooser->currentResource());
    }

    emit sigPaletteListChanged();
}

void KisPaletteListWidget::slotModify()
{
    emit sigPaletteListChanged();
}

void KisPaletteListWidget::slotImport()
{
    emit sigPaletteListChanged();
}

void KisPaletteListWidget::slotExport()
{

}

/************************* KisPaletteListWidgetPrivate **********************/

KisPaletteListWidgetPrivate::KisPaletteListWidgetPrivate(KisPaletteListWidget *a_c)
    : c(a_c)
    , rAdapter(new KoResourceServerAdapter<KoColorSet>(KoResourceServerProvider::instance()->paletteServer()))
    , itemChooser(new KoResourceItemChooser(rAdapter, a_c))
    , model(new Model(rAdapter, a_c))
    , delegate(new Delegate(a_c))
{  }

KisPaletteListWidgetPrivate::~KisPaletteListWidgetPrivate()
{  }

/******************* KisPaletteListWidgetPrivate::Delegate ******************/

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
