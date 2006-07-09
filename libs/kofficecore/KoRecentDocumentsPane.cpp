/* This file is part of the KDE project
   Copyright (C) 2005-2006 Peter Simonsson <psn@linux.se>

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
   Boston, MA 02110-1301, USA.
*/

#include "KoRecentDocumentsPane.h"

#include <QCheckBox>
#include <QLabel>
#include <QFile>
#include <QImage>
#include <QRect>
#include <QBrush>
#include <QPainter>
#include <QSplitter>
#include <QPixmap>

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

class KoFileListItem : public K3ListViewItem
{
  public:
    KoFileListItem(K3ListView* listView, const QString& filename,
                    const QString& fullPath, KFileItem* fileItem)
      : K3ListViewItem(listView, filename, fullPath), m_fileItem(fileItem)
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


class KoRecentDocumentsPanePrivate
{
  public:
    KoRecentDocumentsPanePrivate()
      : m_previewJob(0)
    {
    }

    ~KoRecentDocumentsPanePrivate()
    {
      if(m_previewJob)
        m_previewJob->kill();
    }

    KIO::PreviewJob* m_previewJob;
};


KoRecentDocumentsPane::KoRecentDocumentsPane(QWidget* parent, KInstance* _instance,
                                              const QString& header)
  : KoDetailsPane(parent, _instance, header)
{
  d = new KoRecentDocumentsPanePrivate;
  setFocusProxy(m_documentList);
  KGuiItem openGItem(i18n("Open This Document"), "fileopen");
  m_openButton->setGuiItem(openGItem);
  m_alwaysUseCheckBox->hide();
  m_documentList->setSorting(-1); // Disable sorting

  QString oldGroup = instance()->config()->group();
  instance()->config()->setGroup("RecentFiles");

  int i = 0;
  QString value;
  KFileItemList fileList;

  do {
    QString key = QString("File%1").arg(i);
    value = instance()->config()->readPathEntry(key);

    if(!value.isEmpty()) {
      QString path = value;
      QString name;

      // Support for kdelibs-3.5's new RecentFiles format: name[url]
      if(path.endsWith("]")) {
        int pos = path.indexOf("[");
        name = path.mid(0, pos - 1);
        path = path.mid(pos + 1, path.length() - pos - 2);
      }

      KUrl url(path);

      if(name.isEmpty())
          name = url.fileName();

      if(!url.isLocalFile() || QFile::exists(url.path())) {
        KFileItem* fileItem = new KFileItem(KFileItem::Unknown, KFileItem::Unknown, url);
        fileList.append(fileItem);
        KoFileListItem* item = new KoFileListItem(m_documentList, name, url.url(), fileItem);
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

  instance()->config()->setGroup( oldGroup );

  m_documentList->setSelected(m_documentList->firstChild(), true);

  d->m_previewJob = KIO::filePreview(fileList, 200, 200);

  connect(d->m_previewJob, SIGNAL(result(KJob*)), this, SLOT(previewResult(KJob*)));
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
      details += i18nc("File modification date and time. %1 is date time",
                        "<tr><td><b>Modified:</b></td><td>%1</td></tr>",
                        QString(fileItem->timeString(KIO::UDS_MODIFICATION_TIME)));
      details += i18nc("File access date and time. %1 is date time",
                        "<tr><td><b>Accessed:</b></td><td>%1</td></tr>",
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

void KoRecentDocumentsPane::openFile(Q3ListViewItem* item)
{
  if(!item) return;

  KConfigGroup cfgGrp(instance()->config(), "TemplateChooserDialog");
  cfgGrp.writeEntry("LastReturnType", "File");

  KFileItem* fileItem = static_cast<KoFileListItem*>(item)->fileItem();

  if(fileItem) {
    emit openUrl(fileItem->url());
  }
}

void KoRecentDocumentsPane::previewResult(KJob* job)
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
  KoFileListItem* item = 0;

  while(it.current()) {
    item = static_cast<KoFileListItem*>(it.current());
    if(item->fileItem() == fileItem) {
      item->setPixmap(2, preview);
      QImage icon = preview.toImage();
      icon = icon.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
      icon.convertToFormat(QImage::Format_ARGB32);
      icon = icon.copy((icon.width() - 64) / 2, (icon.height() - 64) / 2, 64, 64);
      item->setPixmap(0, QPixmap::fromImage(icon));

      if(item->isSelected()) {
        m_previewLabel->setPixmap(preview);
      }

      break;
    }

    ++it;
  }
}

#include "KoRecentDocumentsPane.moc"
