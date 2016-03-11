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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_spriter_export.h"

#include <QApplication>
#include <QCheckBox>
#include <QDomDocument>
#include <QFileInfo>
#include <QMessageBox>
#include <QSlider>
#include <QUrl>
#include <QDir>
#include <QFileInfo>

#include <kpluginfactory.h>

#include <KoColorSpaceConstants.h>

#include <KisDocument.h>
#include <KisFilterChain.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <KisImportExportManager.h>
#include <kis_layer.h>
#include <kis_node.h>
#include <kis_node_visitor.h>
#include <kis_painter.h>
#include <kis_paint_layer.h>
#include <kis_shape_layer.h>
#include <kis_file_layer.h>
#include <kis_clone_layer.h>
#include <kis_adjustment_layer.h>
#include <KisPart.h>
#include <kis_types.h>


class KRITAUI_EXPORT KisSaveSCMLVisitor : public KisNodeVisitor
{
public:

    KisSaveSCMLVisitor(KisImageWSP image,
                        const QString &path,
                        const QString &baseName,
                        const QDomDocument &scml)
        : m_image(image)
        , m_path(path)
        , m_baseName(baseName)
        , m_scml(scml)
        , m_depth(0)
    {

    }

    virtual ~KisSaveSCMLVisitor()
    {

    }

public:

    bool visit(KisNode* l)
    {
        return true;
    }

    bool visit(KisPaintLayer *l)
    {
        return savePaintDevice(l->projection(), l->objectName());
    }

    bool visit(KisAdjustmentLayer *l)
    {
        return savePaintDevice(l->projection(), l->objectName());
    }

    bool visit(KisExternalLayer *l)
    {
        return savePaintDevice(l->projection(), l->objectName());
    }

    bool visit(KisCloneLayer *l)
    {
        return savePaintDevice(l->projection(), l->objectName());
    }

    bool visit(KisFilterMask *)
    {
        return true;
    }

    bool visit(KisTransformMask *)
    {
        return true;
    }

    bool visit(KisTransparencyMask *)
    {
        return true;
    }

    bool visit(KisGeneratorLayer * )
    {
        return true;
    }

    bool visit(KisSelectionMask* )
    {
        return true;
    }

    bool visit(KisGroupLayer *l)
    {
        m_depth++;
        qDebug() << "saving group layer" << l << m_depth;

        m_currentFolder = l->objectName();

        if (m_depth > 1) {
            // We don't descend into subgroups; instead the subgroup is saved as a png
            return savePaintDevice(l->projection(), l->objectName());
        }
        else {
            KisLayerSP child = dynamic_cast<KisLayer*>(l->firstChild().data());
            while (child) {
                child->accept(*this);
                child = dynamic_cast<KisLayer*>(child->nextSibling().data());
            }
        }
        m_depth--;
        return true;

    }

private:

    bool savePaintDevice(KisPaintDeviceSP dev, const QString &fileName) {
        qDebug() << "savePaintDevice" << fileName;
        return true;
    }

    KisImageWSP m_image;
    QString m_path;
    QString m_baseName;
    QDomDocument m_scml;
    QString m_currentFolder;
    int m_depth;
};


K_PLUGIN_FACTORY_WITH_JSON(KisSpriterExportFactory, "krita_spriter_export.json", registerPlugin<KisSpriterExport>();)

KisSpriterExport::KisSpriterExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisSpriterExport::~KisSpriterExport()
{
}

KisImportExportFilter::ConversionStatus KisSpriterExport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile << "Spriter export! From:" << from << ", To:" << to << "" << outputFile();

    if (from != "application/x-krita") {
        return KisImportExportFilter::NotImplemented;
    }

    KisDocument *input = inputDocument();

    QString filename = outputFile();
    if (filename.isEmpty()) {
        return KisImportExportFilter::FileNotFound;
    }

    if (!input) {
        return KisImportExportFilter::NoDocumentCreated;
    }

    qApp->processEvents(); // For vector layers to be updated
    input->image()->waitForDone();

    QFileInfo fi(filename);

    QDomDocument scmlDoc;

    KisSaveSCMLVisitor visitor(input->image(), fi.absolutePath(), fi.baseName(), scmlDoc);
    if (input->image()->rootLayer()->accept(visitor)) {

        QFile f(filename);
        f.open(QFile::WriteOnly);
        f.write(scmlDoc.toString().toUtf8());
        f.flush();
        f.close();

        return KisImportExportFilter::OK;
    }

    return KisImportExportFilter::InternalError;
}

#include "kis_spriter_export.moc"
