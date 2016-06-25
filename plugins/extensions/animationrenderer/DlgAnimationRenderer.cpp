/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "DlgAnimationRenderer.h"

#include <QPluginLoader>
#include <QJsonObject>

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <kis_properties_configuration.h>
#include <kis_debug.h>
#include <KisMimeDatabase.h>
#include <KoJsonTrader.h>
#include <KisImportExportFilter.h>

DlgAnimaterionRenderer::DlgAnimaterionRenderer(QWidget *parent)
    : KoDialog(parent)
{
    setCaption(i18n("Render Animation"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    m_page = new WdgAnimaterionRenderer(this);
    m_page->layout()->setMargin(0);
    m_page->dirRequester->setMode(KoFileDialog::OpenDirectory);
    setMainWidget(m_page);
    resize(m_page->sizeHint());

    KoJsonTrader trader;
    QList<QPluginLoader *>list = trader.query("Krita/AnimationExporter", "");
    Q_FOREACH(QPluginLoader *loader, list) {
        QJsonObject json = loader->metaData().value("MetaData").toObject();
        Q_FOREACH(const QString &mimetype, json.value("X-KDE-Export").toString().split(",")) {

            KLibFactory *factory = qobject_cast<KLibFactory *>(loader->instance());

            if (!factory) {
                warnUI << loader->errorString();
                continue;
            }

            QObject* obj = factory->create<KisImportExportFilter>(parent);
            if (!obj || !obj->inherits("KisImportExportFilter")) {
                delete obj;
                continue;
            }

            QSharedPointer<KisImportExportFilter> filter(static_cast<KisImportExportFilter*>(obj));
            if (!filter) {
                delete obj;
                continue;
            }

            m_filters.append(filter);
            //            m_configWidgets.append(filter->createConfigurationWidget(m_page->grpRenderOptions));
            //            m_configWidgets.last()->setVisible(false);

            m_page->cmbRenderType->addItem(KisMimeDatabase::descriptionForMimeType(mimetype));
        }
    }

    connect(m_page->cmbRenderType, SIGNAL(activated(int)), this, SLOT(selectRenderType(int)));
}

DlgAnimaterionRenderer::~DlgAnimaterionRenderer()
{
    delete m_page;
}

KisPropertiesConfigurationSP DlgAnimaterionRenderer::getSequenceConfiguration() const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("basename", m_page->txtBasename->text());
    cfg->setProperty("directory", m_page->dirRequester->fileName());
    cfg->setProperty("first_frame", m_page->intStart->value());
    cfg->setProperty("last_frame", m_page->intEnd->value());
    cfg->setProperty("sequence_start", m_page->sequenceStart->value());
    return cfg;
}

KisPropertiesConfigurationSP DlgAnimaterionRenderer::getVideoConfiguration() const
{
    if (!m_page->grpRenderOptions->isChecked()) {
        return 0;
    }
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("filename", m_page->videoFilename->fileName());
    cfg->setProperty("delete_sequence", m_page->chkDeleteSequence->isChecked());
    return cfg;
}

KisPropertiesConfigurationSP DlgAnimaterionRenderer::getencoderConfiguration() const
{
    if (!m_page->grpRenderOptions->isChecked()) {
        return 0;
    }
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    return cfg;
}

void DlgAnimaterionRenderer::selectRenderType(int renderType)
{
    //    if (renderType >= m_configWidgets->size) return;

    //    Q_FOREACH(QWidget *w, m_configWidgets) {
    //        if (w) {
    //            w->setVisible(false);
    //        }
    //    }
    //    if (m_configWidgets[renterType]) {
    //        m_configWidgets[renderType]->setVisible(true);
    //    }
}
