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

#include <kinstance.h>
#include <klocale.h>
#include <k3listview.h>
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

  K3ListViewItem* selectItem = 0;

  for (KoTemplate* t = group->first(); t != 0L; t = group->next()) {
    if(t->isHidden() || (t->measureSystem() == dontShow))
          continue;

    K3ListViewItem* item = new K3ListViewItem(m_documentList, t->name(), t->description(), t->file());
    QPixmap preview = t->loadPicture(instance());
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

void KoTemplatesPane::openFile(Q3ListViewItem* item)
{
  if(item) {
    KConfigGroup cfgGrp(instance()->config(), "TemplateChooserDialog");
    cfgGrp.writePathEntry("FullTemplateName", item->text(2));
    cfgGrp.writeEntry("LastReturnType", "Template");
    cfgGrp.writeEntry("AlwaysUseTemplate", d->m_alwaysUseTemplate);
    emit openUrl(KUrl(item->text(2)));
  }
}

bool KoTemplatesPane::isSelected()
{
  return d->m_selected;
}

void KoTemplatesPane::alwaysUseClicked()
{
  Q3ListViewItem* item = m_documentList->selectedItem();

  if(!m_alwaysUseCheckBox->isChecked()) {
    KConfigGroup cfgGrp(instance()->config(), "TemplateChooserDialog");
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

#include "KoTemplatesPane.moc"
