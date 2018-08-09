#include <QPointer>
#include <QScopedPointer>
#include <QGridLayout>
#include <QSet>
#include <QStringList>
#include <QFile>
#include <QMessageBox>
#include <QInputDialog>
#include <QDir>

#include <kis_icon.h>
#include <KoFileDialog.h>

#include "KisDlgPaletteEditor.h"

#include <ui_WdgPaletteListWidget.h>
#include "KisPaletteListWidget.h"
#include "KisPaletteListWidget_p.h"

KisPaletteListWidget::KisPaletteListWidget(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui_WdgPaletteListWidget)
    , m_d(new KisPaletteListWidgetPrivate(this))
{
    m_d->allowModification = false;

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
    m_d->itemChooser->showTaggingBar(true);
    m_ui->viewPalette->setLayout(new QHBoxLayout(m_ui->viewPalette));
    m_ui->viewPalette->layout()->addWidget(m_d->itemChooser.data());

    connect(m_d->itemChooser.data(), SIGNAL(resourceSelected(KoResource *)), SLOT(slotPaletteResourceSelected(KoResource*)));
    m_d->itemChooser->setCurrentItem(0, 0);
}

KisPaletteListWidget::~KisPaletteListWidget()
{ }

void KisPaletteListWidget::slotPaletteResourceSelected(KoResource *r)
{
    KoColorSet *g = static_cast<KoColorSet*>(r);
    if (g->isEditable()) {
        m_ui->bnAdd->setEnabled(true);
        m_ui->bnRemove->setEnabled(true);
        m_ui->bnEdit->setEnabled(true);
    } else {
        m_ui->bnAdd->setEnabled(false);
        m_ui->bnRemove->setEnabled(false);
        m_ui->bnEdit->setEnabled(false);
    }
    emit sigPaletteSelected(g);
}

void KisPaletteListWidget::slotAdd()
{
    KoColorSet *newColorSet = new KoColorSet(newPaletteFileName());
    newColorSet->setPaletteType(KoColorSet::KPL);
    newColorSet->setIsGlobal(false);
    newColorSet->setIsEditable(true);
    newColorSet->setName("New Palette");
    m_d->rAdapter->addResource(newColorSet);
    m_d->itemChooser->setCurrentResource(newColorSet);

    emit sigPaletteListChanged();
}

void KisPaletteListWidget::slotRemove()
{
    if (m_d->itemChooser->currentResource()) {
        KoColorSet *cs = static_cast<KoColorSet*>(m_d->itemChooser->currentResource());
        if (!cs || !cs->isEditable()) {
            return;
        }
        if (cs->isGlobal()) {
            QFile::remove(cs->filename());
        }
        m_d->rAdapter->removeResource(cs);
    }
    m_d->itemChooser->setCurrentItem(0, 0);

    emit sigPaletteListChanged();
}

void KisPaletteListWidget::slotModify()
{
    KisDlgPaletteEditor dlg;
    KoColorSet *colorSet = static_cast<KoColorSet*>(m_d->itemChooser->currentResource());
    if (!colorSet) { return; }
    dlg.setPalette(colorSet);
    if (dlg.exec() != QDialog::Accepted){ return; }
    if (!dlg.isModified()) { return; }
    colorSet->setName(dlg.name());
    colorSet->setColumnCount(dlg.columnCount());
    if (dlg.isGlobal()) {
        setPaletteGlobal(colorSet);
    } else {
        setPaletteNonGlobal(colorSet);
    }
    Q_FOREACH (const QString &newGroupName, dlg.newGroupNames()) {
        qDebug() << "new group:" << newGroupName;
        colorSet->addGroup(newGroupName);
        colorSet->getGroup(newGroupName)->setRowCount(dlg.groupRowNumber(newGroupName));
    }
    Q_FOREACH (const QString &groupName, colorSet->getGroupNames()) {
        qDebug() << "modifying existing" << groupName;
        colorSet->getGroup(groupName)->setRowCount(dlg.groupRowNumber(groupName));
        if (groupName != KoColorSet::GLOBAL_GROUP_NAME) { continue; }
        if (dlg.groupRemoved(groupName)) {
            colorSet->removeGroup(groupName, dlg.groupKeepColors(groupName));
            continue;
        }
        if (!dlg.groupRenamedTo(groupName).isEmpty()) {
            qDebug() << "group renamed:" << groupName;
            qDebug() << "to:" << dlg.groupRenamedTo(groupName);
            colorSet->changeGroupName(groupName, dlg.groupRenamedTo(groupName));
        }
    }
    Q_FOREACH (const KoResource *r, m_d->rAdapter->resources()) {
        if (r != colorSet && r->filename() == dlg.filename()) {
            QMessageBox msgFilenameDuplicate;
            msgFilenameDuplicate.setWindowTitle(i18n("Duplicate filename"));
            msgFilenameDuplicate.setText(i18n("Duplicate filename! Palette not saved."));
            msgFilenameDuplicate.exec();
            return;
        }
    }
    colorSet->setFilename(dlg.filename());
    emit sigPaletteSelected(colorSet); // to update elements in the docker
    emit sigPaletteListChanged();
}

void KisPaletteListWidget::slotImport()
{
    KoFileDialog dialog(this, KoFileDialog::OpenFile, "Open Palette");
    dialog.setDefaultDir(QDir::homePath());
    dialog.setMimeTypeFilters(QStringList() << "krita/x-colorset" << "application/x-gimp-color-palette");
    QString fileName = dialog.filename();
    if (fileName.isEmpty()) { return; }
    KoColorSet *colorSet = new KoColorSet(fileName);
    colorSet->load();
    m_d->rAdapter->addResource(colorSet);
    m_d->itemChooser->setCurrentResource(colorSet);
    emit sigPaletteListChanged();
}

void KisPaletteListWidget::slotExport()
{
    KoColorSet *r = static_cast<KoColorSet*>(m_d->itemChooser->currentResource());
    KoFileDialog dialog(this, KoFileDialog::SaveFile, "Save Palette");
    dialog.setDefaultDir(r->filename());
    dialog.setMimeTypeFilters(QStringList() << "krita/x-colorset");
    QString newPath;
    bool isStandAlone = r->isGlobal();
    QString oriPath = r->filename();
    if ((newPath = dialog.filename()).isEmpty()) { return; }
    r->setFilename(newPath);
    r->setIsGlobal(true);
    r->save();
    r->setFilename(oriPath);
    r->setIsGlobal(isStandAlone);
}

void KisPaletteListWidget::setPaletteGlobal(KoColorSet *colorSet)
{
    if (QPointer<KoColorSet>(colorSet).isNull()) { return; }
    KoResourceServer<KoColorSet> *rserver = KoResourceServerProvider::instance()->paletteServer();

    QString saveLocation = rserver->saveLocation();
    QString name = colorSet->filename();

    QFileInfo fileInfo(saveLocation + name);

    colorSet->setFilename(fileInfo.filePath());
    colorSet->setIsGlobal(true);
}

void KisPaletteListWidget::setPaletteNonGlobal(KoColorSet *colorSet)
{
    if (QPointer<KoColorSet>(colorSet).isNull()) { return; }
    QString filename = newPaletteFileName();
    QFile::remove(colorSet->filename());
    colorSet->setFilename(filename);
    colorSet->setIsGlobal(false);
}

void KisPaletteListWidget::setAllowModification(bool allowModification)
{
    m_d->allowModification = allowModification;
    m_ui->bnImport->setEnabled(allowModification);
    m_ui->bnExport->setEnabled(allowModification);
}

QString KisPaletteListWidget::newPaletteFileName()
{
    KoColorSet tmpColorSet;
    QString result = "new_palette_";
    QSet<QString> nameSet;
    QList<KoResource*> rlist = m_d->rAdapter->resources();
    Q_FOREACH (const KoResource *r, rlist) {
        nameSet.insert(r->filename());
    }
    int i = 0;
    while (nameSet.contains(result + QString::number(i) + tmpColorSet.defaultFileExtension())) {
        i++;
    }
    result = result + QString::number(i) + tmpColorSet.defaultFileExtension();
    return result;
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
{ }

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
