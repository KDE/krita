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

#include <q3vbox.h>
#include <qlayout.h>
#include <q3header.h>
#include <q3widgetstack.h>
#include <qlabel.h>
#include <q3valuelist.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpen.h>
//Added by qt3to4:
#include <QPixmap>

#include <klocale.h>
#include <kfiledialog.h>
#include <kinstance.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <k3listview.h>

#include "KoFilterManager.h"
#include "KoTemplates.h"
#include "KoDocument.h"
#include "KoDetailsPane.h"
#include "koDetailsPaneBase.h"

#include <limits.h>

class KoSectionListItem : public Q3ListViewItem
{
  public:
    KoSectionListItem(K3ListView* listView, const QString& name, int sortWeight, int widgetIndex = -1)
      : Q3ListViewItem(listView, name), m_sortWeight(sortWeight), m_widgetIndex(widgetIndex)
    {
    }

    virtual int compare(Q3ListViewItem* i, int, bool) const
    {
      KoSectionListItem* item = dynamic_cast<KoSectionListItem*>(i);

      if(!item)
        return 0;

      return sortWeight() - item->sortWeight();
    }

    virtual void paintCell(QPainter* p, const QColorGroup& cg, int column, int width, int align)
    {
      if(widgetIndex() >= 0) {
        Q3ListViewItem::paintCell(p, cg, column, width, align);
      } else {
        int ypos = (height() - 2) / 2;
        QPen pen(cg.foreground(), 2);
        p->setPen(pen);
        p->drawLine(0, ypos, width, ypos);
      }
    }

    int sortWeight() const { return m_sortWeight; }
    int widgetIndex() const { return m_widgetIndex; }

  private:
    int m_sortWeight;
    int m_widgetIndex;
};

class KoOpenPanePrivate
{
  public:
    KoOpenPanePrivate() :
      m_instance(0)
    {
    }

    KInstance* m_instance;
};

KoOpenPane::KoOpenPane(QWidget *parent, KInstance* instance, const QString& templateType)
  : KoOpenPaneBase(parent, "OpenPane")
{
  d = new KoOpenPanePrivate;
  d->m_instance = instance;

  m_sectionList->header()->hide();
  m_sectionList->setSorting(0);
  connect(m_sectionList, SIGNAL(selectionChanged(Q3ListViewItem*)),
          this, SLOT(selectionChanged(Q3ListViewItem*)));
  connect(m_sectionList, SIGNAL(pressed(Q3ListViewItem*)),
          this, SLOT(itemClicked(Q3ListViewItem*)));
  connect(m_sectionList, SIGNAL(spacePressed(Q3ListViewItem*)),
          this, SLOT(itemClicked(Q3ListViewItem*)));
  connect(m_sectionList, SIGNAL(returnPressed(Q3ListViewItem*)),
          this, SLOT(itemClicked(Q3ListViewItem*)));

  KGuiItem openExistingGItem(i18n("Open Existing Document..."), "fileopen");
  m_openExistingButton->setGuiItem(openExistingGItem);
  connect(m_openExistingButton, SIGNAL(clicked()), this, SLOT(showOpenFileDialog()));

  initRecentDocs();
  initTemplates(templateType);

  KoSectionListItem* selectedItem = static_cast<KoSectionListItem*>(m_sectionList->selectedItem());

  if(selectedItem) {
    m_widgetStack->widget(selectedItem->widgetIndex())->setFocus();
  }

  Q3ValueList<int> sizes;
  sizes << 20 << width() - 20;
  m_splitter->setSizes(sizes);

  // Set the sizes of the details pane splitters
  KConfigGroup cfgGrp(d->m_instance->config(), "TemplateChooserDialog");
  sizes = cfgGrp.readIntListEntry("DetailsPaneSplitterSizes");
  emit splitterResized(0, sizes);

  connect(this, SIGNAL(splitterResized(KoDetailsPaneBase*, const Q3ValueList<int>&)),
          this, SLOT(saveSplitterSizes(KoDetailsPaneBase*, const Q3ValueList<int>&)));
}

KoOpenPane::~KoOpenPane()
{
  KoSectionListItem* item = dynamic_cast<KoSectionListItem*>(m_sectionList->selectedItem());

  if(item) {
    if(!dynamic_cast<KoDetailsPaneBase*>(m_widgetStack->widget(item->widgetIndex()))) {
      KConfigGroup cfgGrp(d->m_instance->config(), "TemplateChooserDialog");
      cfgGrp.writeEntry("LastReturnType", "Custom");
    }
  }

  delete d;
}

void KoOpenPane::showOpenFileDialog()
{
  const QStringList mimeFilter = KoFilterManager::mimeFilter(KoDocument::readNativeFormatMimeType(),
      KoFilterManager::Import, KoDocument::readExtraNativeMimeTypes());

  KUrl url = KFileDialog::getOpenURL(":OpenDialog", mimeFilter.join(" "), this);

  if(!url.isEmpty()) {
    KConfigGroup cfgGrp(d->m_instance->config(), "TemplateChooserDialog");
    cfgGrp.writeEntry("LastReturnType", "File");
    emit openExistingFile(url.url());
  }
}

void KoOpenPane::initRecentDocs()
{
  KoRecentDocumentsPane* recentDocPane = new KoRecentDocumentsPane(this, d->m_instance);
  connect(recentDocPane, SIGNAL(openFile(const QString&)), this, SIGNAL(openExistingFile(const QString&)));
  Q3ListViewItem* item = addPane(i18n("Recent Documents"), "fileopen", recentDocPane, 0);
  connect(recentDocPane, SIGNAL(splitterResized(KoDetailsPaneBase*, const Q3ValueList<int>&)),
          this, SIGNAL(splitterResized(KoDetailsPaneBase*, const Q3ValueList<int>&)));
  connect(this, SIGNAL(splitterResized(KoDetailsPaneBase*, const Q3ValueList<int>&)),
          recentDocPane, SLOT(resizeSplitter(KoDetailsPaneBase*, const Q3ValueList<int>&)));

  KoSectionListItem* separator = new KoSectionListItem(m_sectionList, "", 1);
  separator->setEnabled(false);

  if(d->m_instance->config()->hasGroup("RecentFiles")) {
    m_sectionList->setSelected(item, true);
  }
}

void KoOpenPane::initTemplates(const QString& templateType)
{
  Q3ListViewItem* selectItem = 0;
  Q3ListViewItem* firstItem = 0;
  const int templateOffset = 1000;

  if(!templateType.isEmpty())
  {
    KoTemplateTree templateTree(templateType.local8Bit(), d->m_instance, true);

    for (KoTemplateGroup *group = templateTree.first(); group != 0L; group = templateTree.next()) {
      if (group->isHidden()) {
        continue;
      }

      KoTemplatesPane* pane = new KoTemplatesPane(this, d->m_instance,
          group, templateTree.defaultTemplate());
      connect(pane, SIGNAL(openTemplate(const QString&)), this, SIGNAL(openTemplate(const QString&)));
      connect(pane, SIGNAL(alwaysUseChanged(KoTemplatesPane*, const QString&)),
              this, SIGNAL(alwaysUseChanged(KoTemplatesPane*, const QString&)));
      connect(this, SIGNAL(alwaysUseChanged(KoTemplatesPane*, const QString&)),
              pane, SLOT(changeAlwaysUseTemplate(KoTemplatesPane*, const QString&)));
      connect(pane, SIGNAL(splitterResized(KoDetailsPaneBase*, const Q3ValueList<int>&)),
              this, SIGNAL(splitterResized(KoDetailsPaneBase*, const Q3ValueList<int>&)));
      connect(this, SIGNAL(splitterResized(KoDetailsPaneBase*, const Q3ValueList<int>&)),
              pane, SLOT(resizeSplitter(KoDetailsPaneBase*, const Q3ValueList<int>&)));
      Q3ListViewItem* item = addPane(group->name(), group->first()->loadPicture(d->m_instance),
                                    pane, group->sortingWeight() + templateOffset);

      if(!firstItem) {
        firstItem = item;
      }

      if(group == templateTree.defaultGroup()) {
        firstItem = item;
      }

      if(pane->isSelected()) {
        selectItem = item;
      }
    }
  } else {
    firstItem = m_sectionList->firstChild();
  }

  KConfigGroup cfgGrp(d->m_instance->config(), "TemplateChooserDialog");

  if(selectItem && (cfgGrp.readEntry("LastReturnType") == "Template")) {
    m_sectionList->setSelected(selectItem, true);
  } else if(!m_sectionList->selectedItem() && firstItem) {
    m_sectionList->setSelected(firstItem, true);
  }
}

void KoOpenPane::setCustomDocumentWidget(QWidget *widget) {
  Q_ASSERT(widget);
  KoSectionListItem* separator = new KoSectionListItem(m_sectionList, "", INT_MAX-1);
  separator->setEnabled(false);

  Q3ListViewItem* item = addPane(i18n("Custom Document"), QString::null, widget, INT_MAX);

  KConfigGroup cfgGrp(d->m_instance->config(), "TemplateChooserDialog");

  if(cfgGrp.readEntry("LastReturnType") == "Custom") {
    m_sectionList->setSelected(item, true);
    KoSectionListItem* selectedItem = static_cast<KoSectionListItem*>(item);
    m_widgetStack->widget(selectedItem->widgetIndex())->setFocus();
  }
}

Q3ListViewItem* KoOpenPane::addPane(const QString& title, const QString& icon, QWidget* widget, int sortWeight)
{
  return addPane(title, SmallIcon(icon, K3Icon::SizeLarge, K3Icon::DefaultState, d->m_instance),
                 widget, sortWeight);
}

Q3ListViewItem* KoOpenPane::addPane(const QString& title, const QPixmap& icon, QWidget* widget, int sortWeight)
{
  if(!widget) {
    return 0;
  }

  int id = m_widgetStack->addWidget(widget);
  KoSectionListItem* listItem = new KoSectionListItem(m_sectionList, title, sortWeight, id);

  if(!icon.isNull()) {
    QImage image = icon.convertToImage();

    if((image.width() > 48) || (image.height() > 48)) {
      image = image.smoothScale(48, 48, Qt::KeepAspectRatio);
    }

    image.setAlphaBuffer(true);
    image = image.copy((image.width() - 48) / 2, (image.height() - 48) / 2, 48, 48);
    listItem->setPixmap(0, QPixmap(image));
  }

  return listItem;
}

void KoOpenPane::selectionChanged(Q3ListViewItem* item)
{
  KoSectionListItem* section = dynamic_cast<KoSectionListItem*>(item);

  if(!item)
    return;

  m_headerLabel->setText(section->text(0));
  m_widgetStack->raiseWidget(section->widgetIndex());
}

void KoOpenPane::saveSplitterSizes(KoDetailsPaneBase* /*sender*/, const Q3ValueList<int>& sizes)
{
#warning "kde4: port it"		
#if 0		
  KConfigGroup cfgGrp(d->m_instance->config(), "TemplateChooserDialog");
  cfgGrp.writeEntry("DetailsPaneSplitterSizes", sizes);
#endif  
}

void KoOpenPane::itemClicked(Q3ListViewItem* item)
{
  KoSectionListItem* selectedItem = static_cast<KoSectionListItem*>(item);

  if(selectedItem) {
    m_widgetStack->widget(selectedItem->widgetIndex())->setFocus();
  }
}

#include "KoOpenPane.moc"
