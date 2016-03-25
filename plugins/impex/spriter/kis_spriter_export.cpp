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
#include <QFileInfo>
#include <QDir>
#include <QFileInfo>
#include <QApplication>

#include <kpluginfactory.h>

#include <KoColorSpaceConstants.h>
#include <KoColorSpaceRegistry.h>

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
#include <kis_generator_layer.h>
#include <kis_adjustment_layer.h>
#include <KisPart.h>
#include <kis_types.h>
#include <kis_png_converter.h>

class KisSaveSCMLVisitor : public KisNodeVisitor
{
public:

    KisSaveSCMLVisitor(KisImageWSP image,
                       const QString &path,
                       const QString &baseName,
                       QDomDocument *scml)
        : m_image(image)
        , m_path(path)
        , m_baseName(baseName)
        , m_scml(scml)
        , m_depth(0)
        , m_fileId(0)
        , m_hasBone(true)
    {
        m_root = m_scml->createElement("spriter_data");
        m_scml->appendChild(m_root);
        m_root.setAttribute("scml_version", 1);
        m_root.setAttribute("generator", "krita");
        m_root.setAttribute("generator_version", qApp->applicationVersion());

        m_folder = m_scml->createElement("folder");
        m_folder.setAttribute("id", 0);
        m_folder.setAttribute("name", "objects");
        m_root.appendChild(m_folder);

        m_entity = m_scml->createElement("entity");
        m_entity.setAttribute("id", 0);
        m_entity.setAttribute("name", baseName);
        m_root.appendChild(m_entity);

        m_animation = m_scml->createElement("animation");
        m_animation.setAttribute("id", 0);
        m_animation.setAttribute("name", "default");
        m_animation.setAttribute("length", 1000);
        m_animation.setAttribute("looping", false);
        m_entity.appendChild(m_animation);

        QDomElement mainline = m_scml->createElement("mainline");
        m_animation.appendChild(mainline);
        m_key = m_scml->createElement("key");
        m_key.setAttribute("id", 0);
        mainline.appendChild(m_key);

        if (m_hasBone) {
            QDomElement bone_ref = m_scml->createElement("bone_ref");
            bone_ref.setAttribute("id", 0);
            bone_ref.setAttribute("timeline", 0);
            bone_ref.setAttribute("key", 0);
            m_key.appendChild(bone_ref);

            QDomElement bone_timeline = m_scml->createElement("timeline");
            bone_timeline.setAttribute("id", 0);
            bone_timeline.setAttribute("name", "root");
            bone_timeline.setAttribute("object_type", "bone");
            m_animation.appendChild(bone_timeline);

            QDomElement bone_key = m_scml->createElement("key");
            bone_key.setAttribute("id", 0);
            bone_key.setAttribute("spin", 0);
            bone_timeline.appendChild(bone_key);

            QDomElement bone = m_scml->createElement("bone");
            bone.setAttribute("x", 0);
            bone.setAttribute("y", 0);
            bone.setAttribute("angle", 0.0);
            bone.setAttribute("scale_x", 1.0);
            bone.setAttribute("scale_y", 1.0);
            bone_key.appendChild(bone);\
        }
    }

    virtual ~KisSaveSCMLVisitor()
    {
    }

public:

    void finish()
    {
        // Fix up the object id's
        int objectCount = m_folder.childNodes().count();
        QDomElement e = m_folder.lastChildElement();
        for (int i = 0; i < objectCount; i++) {
            e.setAttribute("id", i);
            e = e.previousSiblingElement();
        }
    }

    bool visit(KisNode* n)
    {
        qDebug() << "visit node" << n->name();
        return true;
    }

    bool visit(KisPaintLayer *l)
    {
        qDebug() << "visit paint layer" << l->name();
        saveFolder(l);
        return savePaintDevice(l->projection(), l->name());
    }

    bool visit(KisAdjustmentLayer *l)
    {
        saveFolder(l);
        return savePaintDevice(l->projection(), l->name());
    }

    bool visit(KisExternalLayer *l)
    {
        saveFolder(l);
        return savePaintDevice(l->projection(), l->name());
    }

    bool visit(KisCloneLayer *l)
    {
        saveFolder(l);
        return savePaintDevice(l->projection(), l->name());
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

    bool visit(KisGeneratorLayer * l)
    {
        saveFolder(l);
        return savePaintDevice(l->projection(), l->name());
    }

    bool visit(KisSelectionMask* )
    {
        return true;
    }

    bool visit(KisGroupLayer *l)
    {
        m_depth++;
        qDebug() << "saving group layer" << l << m_depth << m_path;

        QDir d(m_path);
        d.mkpath(l->name());
        m_currentFolder = l->name();

        if (m_depth > 2) {
            // We don't descend into subgroups; instead the subgroup is saved as a png
            m_depth--;
            saveFolder(l);
            return savePaintDevice(l->projection(), l->name());
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

    void saveFolder(KisLayer * l)
    {
        QDomElement el = m_scml->createElement("file");
        el.setAttribute("id", m_fileId);
        el.setAttribute("name", m_currentFolder + "/" + l->name() + ".png");
        QRect rc = l->exactBounds();
        el.setAttribute("width", rc.width());
        el.setAttribute("height", rc.height());
        m_folder.appendChild(el);

        // Key
        QDomElement object_ref = m_scml->createElement("object_ref");
        object_ref.setAttribute("id", m_fileId);
        object_ref.setAttribute("parent", 0);
        object_ref.setAttribute("timeline", m_hasBone ? m_fileId + 1 : m_fileId);
        object_ref.setAttribute("key", 0);
        object_ref.setAttribute("z_index", m_fileId);
        m_key.appendChild(object_ref);

        // timeline
        QDomElement object_timeline = m_scml->createElement("timeline");
        object_timeline.setAttribute("id",  m_hasBone ? m_fileId + 1 : m_fileId);
        object_timeline.setAttribute("name", l->name());
        object_timeline.setAttribute("object_type", "object");
        m_animation.appendChild(object_timeline);

        QDomElement object_key = m_scml->createElement("key");
        object_key.setAttribute("id", 0);
        object_key.setAttribute("spin", 0);
        object_timeline.appendChild(object_key);

        QDomElement object = m_scml->createElement("object");
        object.setAttribute("folder", 0);
        object.setAttribute("file", m_fileId); // This will be fixed afterwards
        object.setAttribute("x", rc.x());
        object.setAttribute("y", rc.y());
        object.setAttribute("pivot_x", 0);
        object.setAttribute("pivot_y", 0);
        object.setAttribute("angle", 0.0);
        object.setAttribute("scale_x", 1.0);
        object.setAttribute("scale_y", 1.0);
        object_key.appendChild(object);\

        m_fileId++;

    }

    bool savePaintDevice(KisPaintDeviceSP dev, const QString &fileName)
    {
        qDebug() << "savePaintDevice" << m_path + "/" + m_currentFolder + "/" + fileName << dev;
        QRect rc = dev->exactBounds();

        if (!KisPNGConverter::isColorSpaceSupported(dev->colorSpace())) {
            dev = new KisPaintDevice(*dev.data());
            KUndo2Command *cmd = dev->convertTo(KoColorSpaceRegistry::instance()->rgb8());
            delete cmd;
        }

        QFile fp(m_path + "/" + m_currentFolder + "/" + fileName + ".png");

        KisPNGOptions options;
        options.forceSRGB = true;

        vKisAnnotationSP_it beginIt = m_image->beginAnnotations();
        vKisAnnotationSP_it endIt = m_image->endAnnotations();

        KisPNGConverter converter(0);
        KisImageBuilder_Result res = converter.buildFile(&fp, rc, m_image->xRes(), m_image->yRes(), dev, beginIt, endIt, options, 0);

        return (res == KisImageBuilder_RESULT_OK);
    }

    KisImageWSP m_image;
    QString m_path;
    QString m_baseName;
    QDomDocument *m_scml;
    QString m_currentFolder;
    int m_depth;
    QDomElement m_root;
    QDomElement m_folder;
    QDomElement m_entity;
    QDomElement m_animation;
    QDomElement m_key;
    int m_fileId;
    bool m_hasBone;
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
    KisSaveSCMLVisitor visitor(input->image(), fi.absolutePath(), fi.baseName(), &scmlDoc);
    if (input->image()->rootLayer()->accept(visitor)) {
        visitor.finish();

        QFile f(filename);
        f.open(QFile::WriteOnly);
        f.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        f.write(scmlDoc.toString(5).toUtf8());
        f.flush();
        f.close();

        return KisImportExportFilter::OK;
    }

    return KisImportExportFilter::InternalError;
}

#include "kis_spriter_export.moc"
