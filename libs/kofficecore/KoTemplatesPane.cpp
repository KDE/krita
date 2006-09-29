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

#include "KoTemplatesPane.h"

#include <QCheckBox>
#include <QLabel>
#include <QFile>
#include <QImage>
#include <QRect>
#include <QBrush>
#include <QPainter>
#include <QSplitter>
#include <QPixmap>
#include <QStandardItemModel>

#include <kinstance.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kconfig.h>
#include <kurl.h>
#include <kdebug.h>
#include <ktextbrowser.h>

#include "KoTemplates.h"

class KoTemplatesPanePrivate
{
  public:
      KoTemplatesPanePrivate()
  : m_selected(false)
  {
  }

  bool m_selected;
  QString m_alwaysUseTemplate;
};


KoTemplatesPane::KoTemplatesPane(QWidget* parent, KInstance* _instance, const QString& header,
                                  KoTemplateGroup *group, KoTemplate* /*defaultTemplate*/)
  : KoDetailsPane(parent, _instance, header)
{
  d = new KoTemplatesPanePrivate;
  setFocusProxy(m_documentList);

  KGuiItem openGItem(i18n("Use This Template"));
  m_openButton->setGuiItem(openGItem);
  KConfigGroup cfgGrp(instance()->config(), "TemplateChooserDialog");
  QString fullTemplateName = cfgGrp.readPathEntry("FullTemplateName");
  d->m_alwaysUseTemplate = cfgGrp.readPathEntry("AlwaysUseTemplate");
  connect(m_alwaysUseCheckBox, SIGNAL(clicked()), this, SLOT(alwaysUseClicked()));

  QString dontShow = "imperial";

  if(KGlobal::locale()->measureSystem() == KLocale::Imperial) {
    dontShow = "metric";
  }

  QStandardItem* selectItem = 0;
  QStandardItem* rootItem = model()->invisibleRootItem();

  for (KoTemplate* t = group->first(); t != 0L; t = group->next()) {
    if(t->isHidden() || (t->measureSystem() == dontShow))
          continue;

    QPixmap preview = t->loadPicture(instance());
    QImage icon = preview.toImage();
    icon = icon.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    icon.convertToFormat(QImage::Format_ARGB32);
    icon = icon.copy((icon.width() - 64) / 2, (icon.height() - 64) / 2, 64, 64);
    QStandardItem* item = new QStandardItem(QPixmap::fromImage(icon), t->name());
    item->setEditable(false);
    item->setData(t->description(), Qt::UserRole);
    item->setData(t->file(), Qt::UserRole + 1);
    item->setData(preview, Qt::UserRole + 2);
    rootItem->appendRow(item);

    if(d->m_alwaysUseTemplate == t->file()) {
      selectItem = item;
    } else if(!selectItem && (t->file() == fullTemplateName)) {
      selectItem = item;
    }
  }

  QModelIndex selectedIndex;

  if(selectItem) {
    selectedIndex = model()->indexFromItem(selectItem);
    d->m_selected = true;
  } else {
    selectedIndex = model()->indexFromItem(model()->item(0));
  }

  m_documentList->selectionModel()->select(selectedIndex, QItemSelectionModel::Select);
  m_documentList->selectionModel()->setCurrentIndex(selectedIndex, QItemSelectionModel::Select);
}

KoTemplatesPane::~KoTemplatesPane()
{
  delete d;
}

void KoTemplatesPane::selectionChanged(const QModelIndex& index)
{
  if(index.isValid()) {
    QStandardItem* item = model()->itemFromIndex(index);
    m_openButton->setEnabled(true);
    m_alwaysUseCheckBox->setEnabled(true);
    m_titleLabel->setText(item->data(Qt::DisplayRole).toString());
    m_previewLabel->setPixmap(item->data(Qt::UserRole + 2).value<QPixmap>());
    m_detailsLabel->setHtml(item->data(Qt::UserRole).toString());
    m_alwaysUseCheckBox->setChecked(item->data(Qt::UserRole + 1).toString() == d->m_alwaysUseTemplate);
  } else {
    m_openButton->setEnabled(false);
    m_alwaysUseCheckBox->setEnabled(false);
    m_alwaysUseCheckBox->setChecked(false);
    m_titleLabel->clear();
    m_previewLabel->setPixmap(QPixmap());
    m_detailsLabel->clear();
  }
}

void KoTemplatesPane::openFile(const QModelIndex& index)
{
  if(index.isValid()) {
    QStandardItem* item = model()->itemFromIndex(index);
    KConfigGroup cfgGrp(instance()->config(), "TemplateChooserDialog");
    cfgGrp.writePathEntry("FullTemplateName", item->data(Qt::UserRole + 1).toString());
    cfgGrp.writeEntry("LastReturnType", "Template");
    cfgGrp.writeEntry("AlwaysUseTemplate", d->m_alwaysUseTemplate);
    emit openUrl(KUrl(item->data(Qt::UserRole + 1).toString()));
  }
}

bool KoTemplatesPane::isSelected()
{
  return d->m_selected;
}

void KoTemplatesPane::alwaysUseClicked()
{
  QStandardItem* item = model()->itemFromIndex(m_documentList->selectionModel()->currentIndex());

  if(!m_alwaysUseCheckBox->isChecked()) {
    KConfigGroup cfgGrp(instance()->config(), "TemplateChooserDialog");
    cfgGrp.writeEntry("AlwaysUseTemplate", QString());
    d->m_alwaysUseTemplate = QString();
  } else {
    d->m_alwaysUseTemplate = item->data(Qt::UserRole + 1).toString();
  }

  emit alwaysUseChanged(this, d->m_alwaysUseTemplate);
}

void KoTemplatesPane::changeAlwaysUseTemplate(KoTemplatesPane* sender, const QString& alwaysUse)
{
  if(this == sender)
      return;

  QStandardItem* item = model()->itemFromIndex(m_documentList->selectionModel()->currentIndex());

  // If the old always use template is selected uncheck the checkbox
  if(item && (item->data(Qt::UserRole + 1).toString() == d->m_alwaysUseTemplate)) {
    m_alwaysUseCheckBox->setChecked(false);
  }

  d->m_alwaysUseTemplate = alwaysUse;
}

#include "KoTemplatesPane.moc"
