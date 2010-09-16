/* This file is part of the KDE project
   Copyright (C) 2005 Peter Simonsson <psn@linux.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoOpenPane.h"

#include <QLayout>
#include <QLabel>
#include <QImage>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QSize>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStyledItemDelegate>
#include <QLinearGradient>

#include <klocale.h>
#include <kcomponentdata.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kdebug.h>

#include "KoTemplateTree.h"
#include "KoTemplateGroup.h"
#include "KoTemplate.h"
#include "KoDetailsPane.h"
#include "KoTemplatesPane.h"
#include "KoRecentDocumentsPane.h"
#include "ui_KoOpenPaneBase.h"
#include "KoExistingDocumentPane.h"

#include <limits.h>
#include <kconfiggroup.h>

class KoSectionListItem : public QTreeWidgetItem
{
public:
    KoSectionListItem(QTreeWidget* treeWidget, const QString& name, int sortWeight, int widgetIndex = -1)
            : QTreeWidgetItem(treeWidget, QStringList() << name), m_sortWeight(sortWeight), m_widgetIndex(widgetIndex) {
        Qt::ItemFlags newFlags = Qt::NoItemFlags;

        if(m_widgetIndex >= 0)
            newFlags |= Qt::ItemIsEnabled | Qt::ItemIsSelectable;

        setFlags(newFlags);
    }

    virtual bool operator<(const QTreeWidgetItem & other) const {
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

private:
    int m_sortWeight;
    int m_widgetIndex;
};


class KoSectionListDelegate : public QStyledItemDelegate
{
public:
    KoSectionListDelegate(QObject* parent = 0) : QStyledItemDelegate(parent) { }

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        QStyledItemDelegate::paint(painter, option, index);

        if(!(option.state & (int)(QStyle::State_Active && QStyle::State_Enabled)))
        {
            int ypos = option.rect.y() + ((option.rect.height() - 2) / 2);
            QRect lineRect(option.rect.left(), ypos, option.rect.width(), 2);
            QLinearGradient gradient(option.rect.topLeft(), option.rect.bottomRight());
            gradient.setColorAt(option.direction == Qt::LeftToRight ? 0 : 1, option.palette.color(QPalette::Text));
            gradient.setColorAt(option.direction == Qt::LeftToRight ? 1 : 0, Qt::transparent);

            painter->fillRect(lineRect, gradient);
        }
    }
};


class KoOpenPanePrivate : public Ui_KoOpenPaneBase
{
public:
    KoOpenPanePrivate() :
            Ui_KoOpenPaneBase() {
        m_customWidgetsSeparator = 0;
        m_templatesSeparator = 0;
    }

    KComponentData m_componentData;
    int m_freeCustomWidgetIndex;
    KoSectionListItem* m_customWidgetsSeparator;
    KoSectionListItem* m_templatesSeparator;
};

KoOpenPane::KoOpenPane(QWidget *parent, const KComponentData &componentData, const QStringList& mimeFilter, const QString& templateType)
        : QWidget(parent)
        , d(new KoOpenPanePrivate)
{
    d->m_componentData = componentData;
    d->setupUi(this);

    KoSectionListDelegate* delegate = new KoSectionListDelegate(d->m_sectionList);
    d->m_sectionList->setItemDelegate(delegate);

    connect(d->m_sectionList, SIGNAL(itemSelectionChanged()),
            this, SLOT(updateSelectedWidget()));
    connect(d->m_sectionList, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
            this, SLOT(itemClicked(QTreeWidgetItem*)));
    connect(d->m_sectionList, SIGNAL(itemActivated(QTreeWidgetItem*, int)),
            this, SLOT(itemClicked(QTreeWidgetItem*)));

    initRecentDocs();
    initExistingFilesPane(mimeFilter);
    initTemplates(templateType);

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
    KConfigGroup cfgGrp(d->m_componentData.config(), "TemplateChooserDialog");
    sizes = cfgGrp.readEntry("DetailsPaneSplitterSizes", sizes);

    if (!sizes.isEmpty())
        emit splitterResized(0, sizes);

    connect(this, SIGNAL(splitterResized(KoDetailsPane*, const QList<int>&)),
            this, SLOT(saveSplitterSizes(KoDetailsPane*, const QList<int>&)));
}

KoOpenPane::~KoOpenPane()
{
    if (!d->m_sectionList->selectedItems().isEmpty())
    {
        KoSectionListItem* item = dynamic_cast<KoSectionListItem*>(d->m_sectionList->selectedItems().first());

        if (item) {
            if (!qobject_cast<KoDetailsPane*>(d->m_widgetStack->widget(item->widgetIndex()))) {
                KConfigGroup cfgGrp(d->m_componentData.config(), "TemplateChooserDialog");
                cfgGrp.writeEntry("LastReturnType", item->text(0));
            }
        }
    }

    delete d;
}

void KoOpenPane::initRecentDocs()
{
    QString header = i18n("Recent Documents");
    KoRecentDocumentsPane* recentDocPane = new KoRecentDocumentsPane(this, d->m_componentData, header);
    connect(recentDocPane, SIGNAL(openUrl(const KUrl&)), this, SIGNAL(openExistingFile(const KUrl&)));
    QTreeWidgetItem* item = addPane(header, "document-open", recentDocPane, 0);
    connect(recentDocPane, SIGNAL(splitterResized(KoDetailsPane*, const QList<int>&)),
            this, SIGNAL(splitterResized(KoDetailsPane*, const QList<int>&)));
    connect(this, SIGNAL(splitterResized(KoDetailsPane*, const QList<int>&)),
            recentDocPane, SLOT(resizeSplitter(KoDetailsPane*, const QList<int>&)));

    if (d->m_componentData.config()->hasGroup("RecentFiles")) {
        d->m_sectionList->setCurrentItem(item, 0, QItemSelectionModel::ClearAndSelect);
    }
}

void KoOpenPane::initTemplates(const QString& templateType)
{
    QTreeWidgetItem* selectItem = 0;
    QTreeWidgetItem* firstItem = 0;
    const int templateOffset = 1000;

    if (!templateType.isEmpty()) {
        KoTemplateTree templateTree(templateType.toLocal8Bit(), d->m_componentData, true);

        foreach (KoTemplateGroup *group, templateTree.groups()) {
            if (group->isHidden()) {
                continue;
            }

            if (!d->m_templatesSeparator) {
                d->m_templatesSeparator = new KoSectionListItem(d->m_sectionList, "", 999);
            }

            KoTemplatesPane* pane = new KoTemplatesPane(this, d->m_componentData, group->name(),
                    group, templateTree.defaultTemplate());
            connect(pane, SIGNAL(openUrl(const KUrl&)), this, SIGNAL(openTemplate(const KUrl&)));
            connect(pane, SIGNAL(alwaysUseChanged(KoTemplatesPane*, const QString&)),
                    this, SIGNAL(alwaysUseChanged(KoTemplatesPane*, const QString&)));
            connect(this, SIGNAL(alwaysUseChanged(KoTemplatesPane*, const QString&)),
                    pane, SLOT(changeAlwaysUseTemplate(KoTemplatesPane*, const QString&)));
            connect(pane, SIGNAL(splitterResized(KoDetailsPane*, const QList<int>&)),
                    this, SIGNAL(splitterResized(KoDetailsPane*, const QList<int>&)));
            connect(this, SIGNAL(splitterResized(KoDetailsPane*, const QList<int>&)),
                    pane, SLOT(resizeSplitter(KoDetailsPane*, const QList<int>&)));
            QTreeWidgetItem* item = addPane(group->name(), group->templates().first()->loadPicture(d->m_componentData),
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

    KConfigGroup cfgGrp(d->m_componentData.config(), "TemplateChooserDialog");

    if (selectItem && (cfgGrp.readEntry("LastReturnType") == "Template")) {
        d->m_sectionList->setCurrentItem(selectItem, 0, QItemSelectionModel::ClearAndSelect);
    } else if (d->m_sectionList->selectedItems().isEmpty() && firstItem) {
        d->m_sectionList->setCurrentItem(firstItem, 0, QItemSelectionModel::ClearAndSelect);
    }
}

void KoOpenPane::addCustomDocumentWidget(QWidget *widget, const QString& title, const QString& icon)
{
    Q_ASSERT(widget);

    if (!d->m_customWidgetsSeparator) {
        d->m_customWidgetsSeparator = new KoSectionListItem(d->m_sectionList, "", 3);
    }

    QString realtitle = title;

    if (realtitle.isEmpty())
        realtitle = i18n("Custom Document");

    QTreeWidgetItem* item = addPane(realtitle, icon, widget, d->m_freeCustomWidgetIndex);
    ++d->m_freeCustomWidgetIndex;
    KConfigGroup cfgGrp(d->m_componentData.config(), "TemplateChooserDialog");

    QString lastActiveItem = cfgGrp.readEntry("LastReturnType");
    bool showCustomItemByDefault = cfgGrp.readEntry("ShowCustomDocumentWidgetByDefault", false);
    if (lastActiveItem == realtitle || (lastActiveItem.isEmpty() && showCustomItemByDefault)) {
        d->m_sectionList->setCurrentItem(item, 0, QItemSelectionModel::ClearAndSelect);
        KoSectionListItem* selectedItem = static_cast<KoSectionListItem*>(item);
        d->m_widgetStack->widget(selectedItem->widgetIndex())->setFocus();
    }
}

QTreeWidgetItem* KoOpenPane::addPane(const QString& title, const QString& icon, QWidget* widget, int sortWeight)
{
    if (!widget) {
        return 0;
    }

    int id = d->m_widgetStack->addWidget(widget);
    KoSectionListItem* listItem = new KoSectionListItem(d->m_sectionList, title, sortWeight, id);
    listItem->setIcon(0, KIcon(icon));

    return listItem;
}

QTreeWidgetItem* KoOpenPane::addPane(const QString& title, const QPixmap& icon, QWidget* widget, int sortWeight)
{
    if (!widget) {
        return 0;
    }

    int id = d->m_widgetStack->addWidget(widget);
    KoSectionListItem* listItem = new KoSectionListItem(d->m_sectionList, title, sortWeight, id);

    if (!icon.isNull()) {
        QImage image = icon.toImage();

        if ((image.width() > 48) || (image.height() > 48)) {
            image = image.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        image.convertToFormat(QImage::Format_ARGB32);
        image = image.copy((image.width() - 48) / 2, (image.height() - 48) / 2, 48, 48);
        listItem->setIcon(0, QIcon(QPixmap::fromImage(image)));
    }

    return listItem;
}

void KoOpenPane::updateSelectedWidget()
{
    if(!d->m_sectionList->selectedItems().isEmpty())
    {
        KoSectionListItem* section = dynamic_cast<KoSectionListItem*>(d->m_sectionList->selectedItems().first());

        if (!section)
            return;

        d->m_widgetStack->setCurrentIndex(section->widgetIndex());
    }
}

void KoOpenPane::saveSplitterSizes(KoDetailsPane* sender, const QList<int>& sizes)
{
    Q_UNUSED(sender);
    KConfigGroup cfgGrp(d->m_componentData.config(), "TemplateChooserDialog");
    cfgGrp.writeEntry("DetailsPaneSplitterSizes", sizes);
}

void KoOpenPane::itemClicked(QTreeWidgetItem* item)
{
    KoSectionListItem* selectedItem = static_cast<KoSectionListItem*>(item);

    if (selectedItem && selectedItem->widgetIndex() >= 0) {
        d->m_widgetStack->widget(selectedItem->widgetIndex())->setFocus();
    }
}

void KoOpenPane::initExistingFilesPane( const QStringList& mimeFilter )
{
    KoExistingDocumentPane* widget = new KoExistingDocumentPane(this, mimeFilter);
    connect(widget, SIGNAL(openExistingUrl(const KUrl&)),
            this, SIGNAL(openExistingFile(const KUrl&)));
    QTreeWidgetItem* item = addPane(i18n("Open Document"), "document-open", widget, 2);

    KConfigGroup cfgGrp(d->m_componentData.config(), "TemplateChooserDialog");

    if (cfgGrp.readEntry("LastReturnType") == i18n("Open Document")) {
        d->m_sectionList->setCurrentItem(item, 0, QItemSelectionModel::ClearAndSelect);
        KoSectionListItem* selectedItem = static_cast<KoSectionListItem*>(item);
        d->m_widgetStack->widget(selectedItem->widgetIndex())->setFocus();
    }
}

#include <KoOpenPane.moc>
