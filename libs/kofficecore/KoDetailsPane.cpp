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
#include "KoDetailsPane.h"

#include <qcheckbox.h>
#include <qlabel.h>
#include <qfile.h>
#include <qimage.h>
#include <q3header.h>
#include <qrect.h>
#include <qbrush.h>
#include <qpainter.h>
#include <q3simplerichtext.h>
#include <qevent.h>
#include <qsplitter.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3ValueList>

#include <kinstance.h>
#include <klocale.h>
#include <k3listview.h>
#include <kpushbutton.h>
#include <kconfig.h>
#include <kurl.h>
#include <kfileitem.h>
#include <kio/previewjob.h>
#include <kdebug.h>
#include <ktextbrowser.h>
#include <kapplication.h>

#include "KoTemplates.h"

class KoFileListItem : public K3ListViewItem
{
  public:
    KoFileListItem(K3ListView* listView, Q3ListViewItem* after, const QString& filename,
                   const QString& fullPath, KFileItem* fileItem)
      : K3ListViewItem(listView, after, filename, fullPath), m_fileItem(fileItem)
    {
    }

    ~KoFileListItem()
    {
      delete m_fileItem;
    }

    KFileItem* fileItem() const
    {
      return m_fileItem;
    }

  private:
    KFileItem* m_fileItem;
};

class KoTemplatesPanePrivate
{
  public:
    KoTemplatesPanePrivate()
      : m_instance(0), m_selected(false)
    {
    }

    KInstance* m_instance;
    bool m_selected;
    QString m_alwaysUseTemplate;
};


KoTemplatesPane::KoTemplatesPane(QWidget* parent, KInstance* instance,
                                 KoTemplateGroup *group, KoTemplate* /*defaultTemplate*/)
  : KoDetailsPaneBase(parent, "TemplatesPane")
{
  d = new KoTemplatesPanePrivate;
  d->m_instance = instance;
  m_previewLabel->installEventFilter(this);
  m_documentList->installEventFilter(this);
  setFocusProxy(m_documentList);
  m_documentList->setShadeSortColumn(false);

  KGuiItem openGItem(i18n("Use This Template"));
  m_openButton->setGuiItem(openGItem);
  m_documentList->header()->hide();
  KConfigGroup cfgGrp(d->m_instance->config(), "TemplateChooserDialog");
  QString fullTemplateName = cfgGrp.readPathEntry("FullTemplateName");
  d->m_alwaysUseTemplate = cfgGrp.readPathEntry("AlwaysUseTemplate");
  connect(m_alwaysUseCheckBox, SIGNAL(clicked()), this, SLOT(alwaysUseClicked()));
  changePalette();

  if(kapp) {
    connect(kapp, SIGNAL(kdisplayPaletteChanged()), this, SLOT(changePalette()));
  }

  QString dontShow = "imperial";

  if(KGlobal::locale()->measureSystem() == KLocale::Imperial) {
    dontShow = "metric";
  }

  K3ListViewItem* selectItem = 0;

  for (KoTemplate* t = group->first(); t != 0L; t = group->next()) {
    if(t->isHidden() || (t->measureSystem() == dontShow))
      continue;

    K3ListViewItem* item = new K3ListViewItem(m_documentList, t->name(), t->description(), t->file());
    QPixmap preview = t->loadPicture(instance);
    QImage icon = preview.toImage();
    icon = icon.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    icon.convertToFormat(QImage::Format_ARGB32);
    icon = icon.copy((icon.width() - 64) / 2, (icon.height() - 64) / 2, 64, 64);
    item->setPixmap(0, QPixmap::fromImage(icon));
    item->setPixmap(2, preview);

    if(d->m_alwaysUseTemplate == t->file()) {
      selectItem = item;
    } else if(!selectItem && (t->file() == fullTemplateName)) {
      selectItem = item;
    }
  }

  connect(m_documentList, SIGNAL(selectionChanged(Q3ListViewItem*)),
          this, SLOT(selectionChanged(Q3ListViewItem*)));
  connect(m_documentList, SIGNAL(doubleClicked(Q3ListViewItem*, const QPoint&, int)),
          this, SLOT(openTemplate(Q3ListViewItem*)));
  connect(m_documentList, SIGNAL(returnPressed(Q3ListViewItem*)),
          this, SLOT(openTemplate(Q3ListViewItem*)));
  connect(m_openButton, SIGNAL(clicked()), this, SLOT(openTemplate()));

  if(selectItem) {
    m_documentList->setSelected(selectItem, true);
    d->m_selected = true;
  } else {
    m_documentList->setSelected(m_documentList->firstChild(), true);
  }
}

KoTemplatesPane::~KoTemplatesPane()
{
  delete d;
}

void KoTemplatesPane::selectionChanged(Q3ListViewItem* item)
{
  if(item) {
    m_openButton->setEnabled(true);
    m_alwaysUseCheckBox->setEnabled(true);
    m_titleLabel->setText(item->text(0));
    m_previewLabel->setPixmap(*(item->pixmap(2)));
    m_detailsLabel->setHtml(item->text(1));
    m_alwaysUseCheckBox->setChecked(item->text(2) == d->m_alwaysUseTemplate);
  } else {
    m_openButton->setEnabled(false);
    m_alwaysUseCheckBox->setEnabled(false);
    m_alwaysUseCheckBox->setChecked(false);
    m_titleLabel->clear();
    m_previewLabel->setPixmap(QPixmap());
    m_detailsLabel->clear();
  }
}

void KoTemplatesPane::openTemplate()
{
  Q3ListViewItem* item = m_documentList->selectedItem();
  openTemplate(item);
}

void KoTemplatesPane::openTemplate(Q3ListViewItem* item)
{
  if(item) {
    KConfigGroup cfgGrp(d->m_instance->config(), "TemplateChooserDialog");
    cfgGrp.writePathEntry("FullTemplateName", item->text(2));
    cfgGrp.writeEntry("LastReturnType", "Template");
    cfgGrp.writeEntry("AlwaysUseTemplate", d->m_alwaysUseTemplate);
    emit openTemplate(item->text(2));
  }
}

void KoTemplatesPane::changePalette()
{
  QPalette p = kapp ? kapp->palette() : palette();
  p.setBrush(QColorGroup::Base, p.brush(QPalette::Normal, QColorGroup::Background));
  p.setColor(QColorGroup::Text, p.color(QPalette::Normal, QColorGroup::Foreground));
  m_detailsLabel->setPalette(p);
}

bool KoTemplatesPane::isSelected()
{
  return d->m_selected;
}

void KoTemplatesPane::alwaysUseClicked()
{
  Q3ListViewItem* item = m_documentList->selectedItem();

  if(!m_alwaysUseCheckBox->isChecked()) {
    KConfigGroup cfgGrp(d->m_instance->config(), "TemplateChooserDialog");
    cfgGrp.writeEntry("AlwaysUseTemplate", QString());
    d->m_alwaysUseTemplate = QString::null;
  } else {
    d->m_alwaysUseTemplate = item->text(2);
  }

  emit alwaysUseChanged(this, d->m_alwaysUseTemplate);
}

void KoTemplatesPane::changeAlwaysUseTemplate(KoTemplatesPane* sender, const QString& alwaysUse)
{
  if(this == sender)
    return;

  Q3ListViewItem* item = m_documentList->selectedItem();

  // If the old always use template is selected uncheck the checkbox
  if(item && (item->text(2) == d->m_alwaysUseTemplate)) {
    m_alwaysUseCheckBox->setChecked(false);
  }

  d->m_alwaysUseTemplate = alwaysUse;
}

bool KoTemplatesPane::eventFilter(QObject* watched, QEvent* e)
{
  if(watched == m_previewLabel) {
    if(e->type() == QEvent::MouseButtonDblClick) {
      openTemplate();
    }
  }

  if(watched == m_documentList) {
    if((e->type() == QEvent::Resize) && isVisible()) {
      emit splitterResized(this, m_splitter->sizes());
    }
  }

  return false;
}

void KoTemplatesPane::resizeSplitter(KoDetailsPaneBase* sender, const QList<int>& sizes)
{
  if(sender == this)
    return;

  m_splitter->setSizes(sizes);
}


class KoRecentDocumentsPanePrivate
{
  public:
    KoRecentDocumentsPanePrivate()
      : m_previewJob(0), m_instance(0)
    {
    }

    ~KoRecentDocumentsPanePrivate()
    {
      if(m_previewJob)
        m_previewJob->kill();
    }

    KIO::PreviewJob* m_previewJob;
    KInstance* m_instance;
};

KoRecentDocumentsPane::KoRecentDocumentsPane(QWidget* parent, KInstance* instance)
  : KoDetailsPaneBase(parent, "RecentDocsPane")
{
  d = new KoRecentDocumentsPanePrivate;
  d->m_instance = instance;
  m_previewLabel->installEventFilter(this);
  m_documentList->installEventFilter(this);
  setFocusProxy(m_documentList);
  KGuiItem openGItem(i18n("Open This Document"), "fileopen");
  m_openButton->setGuiItem(openGItem);
  m_alwaysUseCheckBox->hide();
  m_documentList->header()->hide();
  m_documentList->setSorting(-1); // Disable sorting
  changePalette();

  if(kapp) {
    connect(kapp, SIGNAL(kdisplayPaletteChanged()), this, SLOT(changePalette()));
  }

  QString oldGroup = instance->config()->group();
  instance->config()->setGroup("RecentFiles");

  int i = 0;
  QString value;
  KFileItemList fileList;

  do {
    QString key = QString("File%1").arg(i);
    value = instance->config()->readPathEntry(key);

    if(!value.isEmpty()) {
      QString path = value;
      QString name;

      // Support for kdelibs-3.5's new RecentFiles format: name[url]
      if(path.endsWith("]")) {
        int pos = path.indexOf("[");
        name = path.mid(0, pos - 1);
        path = path.mid(pos + 1, path.length() - pos - 2);
      }

      KUrl url = KUrl::fromPathOrURL(path);

      if(name.isEmpty())
        name = url.fileName();

      if(!url.isLocalFile() || QFile::exists(url.path())) {
        KFileItem* fileItem = new KFileItem(KFileItem::Unknown, KFileItem::Unknown, url);
        fileList.append(fileItem);
        KoFileListItem* item = new KoFileListItem(m_documentList,
            m_documentList->lastItem(), name, url.url(), fileItem);
        //center all icons in 64x64 area
        QImage icon = fileItem->pixmap(64).toImage();
        icon.convertToFormat(QImage::Format_ARGB32);
        icon = icon.copy((icon.width() - 64) / 2, (icon.height() - 64) / 2, 64, 64);
        item->setPixmap(0, QPixmap::fromImage(icon));
        item->setPixmap(2, fileItem->pixmap(128));
      }
    }

    i++;
  } while ( !value.isEmpty() || i<=10 );

  instance->config()->setGroup( oldGroup );

  connect(m_documentList, SIGNAL(selectionChanged(Q3ListViewItem*)),
          this, SLOT(selectionChanged(Q3ListViewItem*)));
  connect(m_documentList, SIGNAL(clicked(Q3ListViewItem*)),
          this, SLOT(selectionChanged(Q3ListViewItem*)));
  connect(m_documentList, SIGNAL(doubleClicked(Q3ListViewItem*, const QPoint&, int)),
          this, SLOT(openFile(Q3ListViewItem*)));
  connect(m_documentList, SIGNAL(returnPressed(Q3ListViewItem*)),
          this, SLOT(openFile(Q3ListViewItem*)));
  connect(m_openButton, SIGNAL(clicked()), this, SLOT(openFile()));

  m_documentList->setSelected(m_documentList->firstChild(), true);

  d->m_previewJob = KIO::filePreview(fileList, 200, 200);

  connect(d->m_previewJob, SIGNAL(result(KIO::Job*)), this, SLOT(previewResult(KIO::Job*)));
  connect(d->m_previewJob, SIGNAL(gotPreview(const KFileItem*, const QPixmap&)),
          this, SLOT(updatePreview(const KFileItem*, const QPixmap&)));
}

KoRecentDocumentsPane::~KoRecentDocumentsPane()
{
  delete d;
}

void KoRecentDocumentsPane::selectionChanged(Q3ListViewItem* item)
{
  if(item) {
    m_openButton->setEnabled(true);
    m_titleLabel->setText(item->text(0));
    m_previewLabel->setPixmap(*(item->pixmap(2)));

    if(static_cast<KoFileListItem*>(item)->fileItem()) {
      KFileItem* fileItem = static_cast<KoFileListItem*>(item)->fileItem();
      QString details = "<center><table border=\"0\">";
      details += i18nc("File modification date and time. %1 is date time", "<tr><td><b>Modified:</b></td><td>%1</td></tr>",
          QString(fileItem->timeString(KIO::UDS_MODIFICATION_TIME)));
      details += i18nc("File access date and time. %1 is date time", "<tr><td><b>Accessed:</b></td><td>%1</td></tr>",
          QString(fileItem->timeString(KIO::UDS_ACCESS_TIME)));
      details += "</table></center>";
      m_detailsLabel->setHtml(details);
    } else {
      m_detailsLabel->clear();
    }
  } else {
    m_openButton->setEnabled(false);
    m_titleLabel->clear();
    m_previewLabel->setPixmap(QPixmap());
    m_detailsLabel->clear();
  }
}

void KoRecentDocumentsPane::openFile()
{
  Q3ListViewItem* item = m_documentList->selectedItem();
  openFile(item);
}

void KoRecentDocumentsPane::openFile(Q3ListViewItem* item)
{
  KConfigGroup cfgGrp(d->m_instance->config(), "TemplateChooserDialog");
  cfgGrp.writeEntry("LastReturnType", "File");

  if(item)
    emit openFile(item->text(1));
}

void KoRecentDocumentsPane::previewResult(KIO::Job* job)
{
  if(d->m_previewJob == job)
    d->m_previewJob = 0;
}

void KoRecentDocumentsPane::updatePreview(const KFileItem* fileItem, const QPixmap& preview)
{
  if(preview.isNull()) {
    return;
  }

  Q3ListViewItemIterator it(m_documentList);

  while(it.current()) {
    if(it.current()->text(1) == fileItem->url().url()) {
      it.current()->setPixmap(2, preview);
      QImage icon = preview.toImage();
      icon = icon.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
      icon.convertToFormat(QImage::Format_ARGB32);
      icon = icon.copy((icon.width() - 64) / 2, (icon.height() - 64) / 2, 64, 64);
      it.current()->setPixmap(0, QPixmap::fromImage(icon));

      if(it.current()->isSelected()) {
        m_previewLabel->setPixmap(preview);
      }

      break;
    }

    it++;
  }
}

void KoRecentDocumentsPane::changePalette()
{
  QPalette p = kapp ? kapp->palette() : palette();
  p.setBrush(QColorGroup::Base, p.brush(QPalette::Normal, QColorGroup::Background));
  p.setColor(QColorGroup::Text, p.color(QPalette::Normal, QColorGroup::Foreground));
  m_detailsLabel->setPalette(p);
}

bool KoRecentDocumentsPane::eventFilter(QObject* watched, QEvent* e)
{
  if(watched == m_previewLabel) {
    if(e->type() == QEvent::MouseButtonDblClick) {
      openFile();
    }
  }

  if(watched == m_documentList) {
    if((e->type() == QEvent::Resize) && isVisible()) {
      emit splitterResized(this, m_splitter->sizes());
    }
  }

  return false;
}

void KoRecentDocumentsPane::resizeSplitter(KoDetailsPaneBase* sender, const QList<int>& sizes)
{
  if(sender == this)
    return;

  m_splitter->setSizes(sizes);
}

#include "KoDetailsPane.moc"
