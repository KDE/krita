/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2005 Peter Simonsson <psn@linux.se>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KisOpenPane.h"

#include <QLayout>
#include <QLabel>
#include <QImage>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QSize>
#include <QString>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStyledItemDelegate>
#include <QLinearGradient>
#include <QStandardPaths>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kis_debug.h>
#include <QUrl>


#include <KoFileDialog.h>
#include <KoIcon.h>
#include "KisTemplateTree.h"
#include "KisTemplateGroup.h"
#include "KisTemplate.h"
#include "KisDetailsPane.h"
#include "KisTemplatesPane.h"
#include "ui_KisOpenPaneBase.h"

#include <limits.h>
#include <kconfiggroup.h>

#include <kis_icon.h>

class KoSectionListItem : public QTreeWidgetItem
{
public:
    KoSectionListItem(QTreeWidget* treeWidget, const QString& name, QString untranslatedName, int sortWeight, int widgetIndex = -1)
        : QTreeWidgetItem(treeWidget, QStringList() << name)
        , m_sortWeight(sortWeight)
        , m_widgetIndex(widgetIndex)
        , m_untranslatedName(untranslatedName)
    {
        Qt::ItemFlags newFlags = Qt::NoItemFlags;

        if(m_widgetIndex >= 0)
            newFlags |= Qt::ItemIsEnabled | Qt::ItemIsSelectable;

        setFlags(newFlags);
    }

    bool operator<(const QTreeWidgetItem & other) const override {
        const KoSectionListItem* item = dynamic_cast<const KoSectionListItem*>(&other);

        if (!item)
            return 0;

        return ((item->sortWeight() - sortWeight()) < 0);
    }

    int sortWeight() const {
        return m_sortWeight;
    }

    int widgetIndex() const {
        return m_widgetIndex;
    }

    QString untranslatedName() const {
        return m_untranslatedName;
    }

private:
    int m_sortWeight;
    int m_widgetIndex;
    QString m_untranslatedName;
};

class KisOpenPanePrivate : public Ui_KisOpenPaneBase
{
public:
    KisOpenPanePrivate() :
        Ui_KisOpenPaneBase() {
        m_templatesSeparator = 0;
    }

    int m_freeCustomWidgetIndex;
    KoSectionListItem* m_templatesSeparator;


};

KisOpenPane::KisOpenPane(QWidget *parent, const QStringList& mimeFilter, const QString& templatesResourcePath)
    : QDialog(parent)
    , d(new KisOpenPanePrivate)
{
    d->setupUi(this);

    m_mimeFilter = mimeFilter;

    QStyledItemDelegate* delegate = new QStyledItemDelegate(d->m_sectionList);
    d->m_sectionList->setItemDelegate(delegate);

    connect(d->m_sectionList, SIGNAL(itemSelectionChanged()),
            this, SLOT(updateSelectedWidget()));
    connect(d->m_sectionList, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(itemClicked(QTreeWidgetItem*)));
    connect(d->m_sectionList, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(itemClicked(QTreeWidgetItem*)));

    initTemplates(templatesResourcePath);

    d->m_freeCustomWidgetIndex = 4;

    if (!d->m_sectionList->selectedItems().isEmpty())
    {
        KoSectionListItem* selectedItem = static_cast<KoSectionListItem*>(d->m_sectionList->selectedItems().first());

        if (selectedItem) {
            d->m_widgetStack->widget(selectedItem->widgetIndex())->setFocus();
        }
    }

    QList<int> sizes;

    // Set the sizes of the details pane splitters
    KConfigGroup cfgGrp( KSharedConfig::openConfig(), "TemplateChooserDialog"); sizes = cfgGrp.readEntry("DetailsPaneSplitterSizes", sizes);

    if (!sizes.isEmpty())
        emit splitterResized(0, sizes);

    connect(this, SIGNAL(splitterResized(KisDetailsPane*,QList<int>)),
            this, SLOT(saveSplitterSizes(KisDetailsPane*,QList<int>)));

    setAcceptDrops(true);
}

KisOpenPane::~KisOpenPane()
{
    if (!d->m_sectionList->selectedItems().isEmpty()) {
        KoSectionListItem* item = dynamic_cast<KoSectionListItem*>(d->m_sectionList->selectedItems().first());

        if (item) {
            if (!qobject_cast<KisDetailsPane*>(d->m_widgetStack->widget(item->widgetIndex()))) {
                KConfigGroup cfgGrp( KSharedConfig::openConfig(), "TemplateChooserDialog");
                cfgGrp.writeEntry("LastReturnType", item->untranslatedName());
            }
        }
    }

    delete d;
}



void KisOpenPane::openFileDialog()
{

    KoFileDialog dialog(this, KoFileDialog::OpenFiles, "OpenDocument");
    dialog.setCaption(i18n("Open Existing Document"));
    dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    dialog.setMimeTypeFilters(m_mimeFilter);
    Q_FOREACH (const QString &filename, dialog.filenames()) {
        emit openExistingFile(QUrl::fromUserInput(filename));
    }
}

void KisOpenPane::slotOpenTemplate(const QUrl &url)
{
    accept();
    emit openTemplate(url);
}

void KisOpenPane::initTemplates(const QString& templatesResourcePath)
{
    QTreeWidgetItem* selectItem = 0;
    QTreeWidgetItem* firstItem = 0;
    const int templateOffset = 1000;

    if (!templatesResourcePath.isEmpty()) {
        KisTemplateTree templateTree(templatesResourcePath, true);

        Q_FOREACH (KisTemplateGroup *group, templateTree.groups()) {
            if (group->isHidden()) {
                continue;
            }

            if (!d->m_templatesSeparator) {
                d->m_templatesSeparator = new KoSectionListItem(d->m_sectionList, "", "", 999);
            }

            KisTemplatesPane* pane = new KisTemplatesPane(this, group->name(),
                                                          group, templateTree.defaultTemplate());
            connect(pane, SIGNAL(openUrl(QUrl)), this, SLOT(slotOpenTemplate(QUrl)));
            connect(pane, SIGNAL(alwaysUseChanged(KisTemplatesPane*,QString)),
                    this, SIGNAL(alwaysUseChanged(KisTemplatesPane*,QString)));
            connect(this, SIGNAL(alwaysUseChanged(KisTemplatesPane*,QString)),
                    pane, SLOT(changeAlwaysUseTemplate(KisTemplatesPane*,QString)));
            connect(pane, SIGNAL(splitterResized(KisDetailsPane*,QList<int>)),
                    this, SIGNAL(splitterResized(KisDetailsPane*,QList<int>)));
            connect(this, SIGNAL(splitterResized(KisDetailsPane*,QList<int>)),
                    pane, SLOT(resizeSplitter(KisDetailsPane*,QList<int>)));
            QTreeWidgetItem* item = addPane(group->name(), "Template", group->templates().first()->loadPicture(),
                                            pane, group->sortingWeight() + templateOffset);



            if (!firstItem) {
                firstItem = item;
            }

            if (group == templateTree.defaultGroup()) {
                firstItem = item;
            }

            if (pane->isSelected()) {
                selectItem = item;
            }
        }
    } else {
        firstItem = d->m_sectionList->topLevelItem(0);
    }

    KConfigGroup cfgGrp( KSharedConfig::openConfig(), "TemplateChooserDialog");

    if (selectItem && (cfgGrp.readEntry("LastReturnType") == "Template")) {
        d->m_sectionList->setCurrentItem(selectItem, 0, QItemSelectionModel::ClearAndSelect);
    }
    else if (d->m_sectionList->selectedItems().isEmpty() && firstItem) {
        d->m_sectionList->setCurrentItem(firstItem, 0, QItemSelectionModel::ClearAndSelect);
    }
}

void KisOpenPane::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->accept();
    }
}

void KisOpenPane::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() > 0) {
        // XXX: when the MVC refactoring is done, this can open a bunch of
        //      urls, but since the part/document combination is still 1:1
        //      that won't work for now.
        emit openExistingFile(event->mimeData()->urls().first());

    }
}

void KisOpenPane::addCustomDocumentWidget(QWidget *widget, const QString& title, const QString &untranslatedName, const QString& icon)
{
    Q_ASSERT(widget);

    QTreeWidgetItem* item = addPane(title, untranslatedName, icon, widget, d->m_freeCustomWidgetIndex);
    ++d->m_freeCustomWidgetIndex;
    KConfigGroup cfgGrp( KSharedConfig::openConfig(), "TemplateChooserDialog");

    QString lastActiveItem = cfgGrp.readEntry("LastReturnType");
    bool showCustomItemByDefault = cfgGrp.readEntry("ShowCustomDocumentWidgetByDefault", false);
    if (lastActiveItem == untranslatedName || (lastActiveItem.isEmpty() && showCustomItemByDefault)) {
        d->m_sectionList->setCurrentItem(item, 0, QItemSelectionModel::ClearAndSelect);
        KoSectionListItem* selectedItem = static_cast<KoSectionListItem*>(item);
        d->m_widgetStack->widget(selectedItem->widgetIndex())->setFocus();
    }
}


QTreeWidgetItem* KisOpenPane::addPane(const QString &title, const QString &untranslatedName, const QString &iconName, QWidget *widget, int sortWeight)
{
    if (!widget) {
        return 0;
    }

    int id = d->m_widgetStack->addWidget(widget);
    KoSectionListItem* listItem = new KoSectionListItem(d->m_sectionList, title, untranslatedName, sortWeight, id);

    // resizes icons so they are a bit smaller
    QIcon icon = KisIconUtils::loadIcon(iconName);
    QPixmap iconPixmap = icon.pixmap(32, 32);

    QIcon finalIcon(iconPixmap);
    listItem->setIcon(0, finalIcon);

    return listItem;
}

QTreeWidgetItem* KisOpenPane::addPane(const QString &title, const QString &untranslatedName, const QPixmap& icon, QWidget* widget, int sortWeight)
{
    if (!widget) {
        return 0;
    }

    int id = d->m_widgetStack->addWidget(widget);

    int iconSize = 32;

    KoSectionListItem* listItem = new KoSectionListItem(d->m_sectionList, title, untranslatedName, sortWeight, id);

    if (!icon.isNull()) {
        QImage image = icon.toImage();

        if (!image.isNull() && ((image.width() > iconSize) || (image.height() > iconSize))) {
            image = image.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        image = image.convertToFormat(QImage::Format_ARGB32);
        image = image.copy((image.width() - iconSize) / 2, (image.height() - iconSize) / 2, iconSize, iconSize);
        listItem->setIcon(0, QIcon(QPixmap::fromImage(image)));
    }

    return listItem;
}

void KisOpenPane::updateSelectedWidget()
{
    if(!d->m_sectionList->selectedItems().isEmpty())
    {
        KoSectionListItem* section = dynamic_cast<KoSectionListItem*>(d->m_sectionList->selectedItems().first());

        if (!section)
            return;

        d->m_widgetStack->setCurrentIndex(section->widgetIndex());
    }
}

void KisOpenPane::saveSplitterSizes(KisDetailsPane* sender, const QList<int>& sizes)
{
    Q_UNUSED(sender);
    KConfigGroup cfgGrp( KSharedConfig::openConfig(), "TemplateChooserDialog");
    cfgGrp.writeEntry("DetailsPaneSplitterSizes", sizes);
}

void KisOpenPane::itemClicked(QTreeWidgetItem* item)
{
    KoSectionListItem* selectedItem = static_cast<KoSectionListItem*>(item);

    if (selectedItem && selectedItem->widgetIndex() >= 0) {
        d->m_widgetStack->widget(selectedItem->widgetIndex())->setFocus();
    }
}
