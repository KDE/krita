/*
 * imagesplit.cc -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (c) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
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

#include "imagesplit.h"

#include <klocale.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kis_config.h>
#include <kis_types.h>
#include <kis_view2.h>
#include <kis_image.h>

#include <kis_doc2.h>
#include <KoDocument.h>
#include <kmimetype.h>
#include <QStringList>
#include <KoFilterManager.h>
#include <kfiledialog.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <QDir>

#include <kis_paint_device.h>
#include <kis_background.h>
#include "dlg_imagesplit.h"

K_PLUGIN_FACTORY(ImagesplitFactory, registerPlugin<Imagesplit>();)
K_EXPORT_PLUGIN(ImagesplitFactory("krita"))

Imagesplit::Imagesplit(QObject *parent, const QVariantList &)
        : KParts::Plugin(parent)
{
    if (parent->inherits("KisView2")) {
        setXMLFile(KStandardDirs::locate("data", "kritaplugins/imagesplit.rc"), true);
        KAction *action  = new KAction(i18n("Image Split "), this);
        actionCollection()->addAction("imagesplit", action);
        connect(action, SIGNAL(triggered()), this, SLOT(slotImagesplit()));
        m_view = (KisView2*) parent;

    }
}

Imagesplit::~Imagesplit()
{
    m_view = 0;
}

void Imagesplit::saveAsImage(QRect imgSize,QString mimeType,KUrl url)
{
    KisImageWSP image = m_view->image();

    KisDoc2 d;
    d.prepareForImport();

    KisImageWSP dst = new KisImage(d.createUndoStore(), imgSize.width(),imgSize.height(), image->colorSpace(), image->objectName());
    dst->setResolution(image->xRes(), image->yRes());
    d.setCurrentImage(dst);

    KisPaintLayer* paintLayer = new KisPaintLayer(dst,dst->nextLayerName(), 255);
    KisPainter gc(paintLayer->paintDevice());
    gc.bitBlt(QPoint(0,0), image->projection(), imgSize);

    dst->addNode(paintLayer, KisNodeSP(0));
    dst->refreshGraph();
    d.setOutputMimeType(mimeType.toLatin1());
    d.exportDocument(url);
}

void Imagesplit::slotImagesplit()
{
    // Taking the title - url from caption function and removing file extension
    QStringList strList = ((m_view->document())->caption()).split(".");
    QString suffix = strList.at(0);

    // Getting all mime types and converting them into names which are displayed at combo box
    QStringList listMimeFilter = KoFilterManager::mimeFilter("application/x-krita", KoFilterManager::Export);
    QStringList listFileType;
    foreach(const QString tempStr, listMimeFilter) {
        KMimeType::Ptr type = KMimeType::mimeType( tempStr );
        listFileType.append(type->comment());
    }


    DlgImagesplit * dlgImagesplit = new DlgImagesplit(m_view,suffix,listFileType);
    dlgImagesplit->setObjectName("Imagesplit");
    Q_CHECK_PTR(dlgImagesplit);

    KisImageWSP image = m_view->image();

    if (dlgImagesplit->exec() == QDialog::Accepted) {

        int numHorizontalLines = dlgImagesplit->horizontalLines();
        int numVerticalLines = dlgImagesplit->verticalLines();

        int img_width = image->width()/(numVerticalLines+1);
        int img_height = image->height()/(numHorizontalLines+1);


        if (dlgImagesplit->autoSave()) {
            for(int i=0,k=1;i<(numVerticalLines+1);i++) {
                for(int j=0;j<(numHorizontalLines+1);j++,k++)
                {
                    KMimeType::Ptr mimeTypeSelected = KMimeType::mimeType( listMimeFilter.at(dlgImagesplit->cmbIndex));
                    KUrl url( QDir::homePath());
                    QString fileName = dlgImagesplit->suffix()+"_"+ QString::number(k)+mimeTypeSelected->mainExtension();
                    url.addPath( fileName );
                    KUrl kurl=url.url();
                    saveAsImage(QRect((i*img_width),(j*img_height),img_width,img_height),listMimeFilter.at(dlgImagesplit->cmbIndex),kurl);
                }
            }
        }
        else {

            for(int i=0;i<(numVerticalLines+1);i++) {
              for(int j=0;j<(numHorizontalLines+1);j++)
              {
                    QString mimelist = listMimeFilter.join(" ");
                    KFileDialog fd(KUrl(QString()), mimelist, m_view);
                    fd.setObjectName("Save Image on Split");
                    fd.setCaption(i18n("Save Image on Split"));
                    fd.setMimeFilter(listMimeFilter, listMimeFilter.at(0));
                    fd.setOperationMode(KFileDialog::Saving);

                    if (!fd.exec()) return;

                    KUrl url = fd.selectedUrl();
                    QString mimefilter = fd.currentMimeFilter();

                    if (url.isEmpty())
                        return;

                    saveAsImage(QRect((i*img_width),(j*img_height),img_width,img_height),mimefilter,url);
                }
            }

        }

    }
    delete dlgImagesplit;
}

#include "imagesplit.moc"
