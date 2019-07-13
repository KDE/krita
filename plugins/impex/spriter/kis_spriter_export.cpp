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
#include <QSlider>
#include <QDir>

#include <kpluginfactory.h>

#include <KoColorSpaceConstants.h>
#include <KoColorSpaceRegistry.h>

#include <KisExportCheckRegistry.h>
#include <KisImportExportManager.h>

#include <KisDocument.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_node.h>
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
#include <kis_global.h> // for KisDegreesToRadians
#include <kis_fast_math.h>
#include <math.h>
#include <kis_dom_utils.h>

K_PLUGIN_FACTORY_WITH_JSON(KisSpriterExportFactory, "krita_spriter_export.json", registerPlugin<KisSpriterExport>();)

KisSpriterExport::KisSpriterExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisSpriterExport::~KisSpriterExport()
{
}

KisImportExportErrorCode KisSpriterExport::savePaintDevice(KisPaintDeviceSP dev, const QString &fileName)
{
    QFileInfo fi(fileName);

    QDir d = fi.absoluteDir();
    d.mkpath(d.path());
    QRect rc = m_image->bounds().intersected(dev->exactBounds());

    if (!KisPNGConverter::isColorSpaceSupported(dev->colorSpace())) {
        dev = new KisPaintDevice(*dev.data());
        dev->convertTo(KoColorSpaceRegistry::instance()->rgb8());
    }

    KisPNGOptions options;
    options.forceSRGB = true;

    vKisAnnotationSP_it beginIt = m_image->beginAnnotations();
    vKisAnnotationSP_it endIt = m_image->endAnnotations();

    KisPNGConverter converter(0);
    KisImportExportErrorCode res = converter.buildFile(fileName, rc, m_image->xRes(), m_image->yRes(), dev, beginIt, endIt, options, 0);

    return res;
}

KisImportExportErrorCode KisSpriterExport::parseFolder(KisGroupLayerSP parentGroup, const QString &folderName, const QString &basePath, int *folderId)
{
//    qDebug() << "parseFolder: parent" << parentGroup->name()
//                << "folderName" << folderName
//                << "basepath" << basePath;

    int currentFolder=0;
	if(folderId == 0)
	{
		folderId = &currentFolder;
	}
    QString pathName;
    if (!folderName.isEmpty()) {
        pathName = folderName + "/";
    }


    KisNodeSP child = parentGroup->lastChild();
    while (child) {
        if (child->visible() && child->inherits("KisGroupLayer")) {
            KisImportExportErrorCode res = parseFolder(qobject_cast<KisGroupLayer*>(child.data()), child->name().split(" ").first(), basePath + "/" + pathName, folderId);
            if (!res.isOk()) {
                return res;
            }
        }
        child = child->prevSibling();
    }

    Folder folder;
    folder.id = *folderId;
    folder.name = folderName;
    folder.groupName = parentGroup->name();

    int fileId = 0;
    child = parentGroup->lastChild();

    while (child) {
        if (child->visible() && !child->inherits("KisGroupLayer") && !child->inherits("KisMask")) {
            QRectF rc = m_image->bounds().intersected(child->exactBounds());
            QString layerBaseName = child->name().split(" ").first();
            SpriterFile file;
            file.id = fileId++;
            file.pathName = pathName;
            file.baseName = layerBaseName;
            file.layerName = child->name();
            file.name = folderName + "/" + layerBaseName + ".png";

            qreal xmin = rc.left();
            qreal ymin = rc.top();
            qreal xmax = rc.right();
            qreal ymax = rc.bottom();

            file.width = xmax - xmin;
            file.height = ymax - ymin;
            file.x = xmin;
            file.y = ymin;
            //qDebug() << "Created file" << file.id << file.name << file.pathName << file.baseName << file.width << file.height << file.layerName;
            KisImportExportErrorCode result = savePaintDevice(child->projection(), basePath + file.name);
            if (result.isOk()) {
                folder.files.append(file);
            } else {
                return result;
            }
        }

        child = child->prevSibling();
    }

    if (folder.files.size() > 0) {
        //qDebug() << "Adding folder" << folder.id << folder.name << folder.groupName << folder.files.length();
        m_folders.append(folder);
        (*folderId)++;
    }

    return ImportExportCodes::OK;
}

Bone *KisSpriterExport::parseBone(const Bone *parent, KisGroupLayerSP groupLayer)
{
    static int boneId = 0;
    QString groupBaseName = groupLayer->name().split(" ").first();
    Bone *bone = new Bone;
    bone->id = boneId++;
    bone->parentBone = parent;
    bone->name = groupBaseName;

    if (m_boneLayer) {
        QRectF rc = m_image->bounds().intersected(m_boneLayer->exactBounds());

        qreal xmin = rc.left();
        qreal ymin = rc.top();
        qreal xmax = rc.right();
        qreal ymax = rc.bottom();

        bone->x = (xmin + xmax) / 2;
        bone->y = -(ymin + ymax) / 2;
        bone->width = xmax - xmin;
        bone->height = ymax - ymin;
    }
    else {
        bone->x = 0.0;
        bone->y = 0.0;
        bone->width = 0.0;
        bone->height = 0.0;
    }

    if (parent) {
        bone->localX = bone->x - parent->x;
        bone->localY = bone->y - parent->y;
    }
    else {
        bone->localX = bone->x;
        bone->localY = bone->y;
    }

    bone->localAngle = 0.0;
    bone->localScaleX = 1.0;
    bone->localScaleY = 1.0;

    KisNodeSP child = groupLayer->lastChild();
    while (child) {
        if (child->visible() && child->inherits("KisGroupLayer")) {
            bone->bones.append(parseBone(bone, qobject_cast<KisGroupLayer*>(child.data())));
        }
        child = child->prevSibling();
    }

    //qDebug() << "Created bone" << bone->id << "with" << bone->bones.size() << "bones";
    return bone;
}

void copyBone(Bone *startBone)
{
    startBone->fixLocalX = startBone->localX;
    startBone->fixLocalY = startBone->localY;
    startBone->fixLocalAngle = startBone->localAngle;
    startBone->fixLocalScaleX= startBone->localScaleX;
    startBone->fixLocalScaleY= startBone->localScaleY;

    Q_FOREACH(Bone *child, startBone->bones) {
        copyBone(child);
    }
}

void KisSpriterExport::fixBone(Bone *bone)
{
    qreal boneLocalAngle = 0;
    qreal boneLocalScaleX = 1;

    if (bone->bones.length() >= 1) {
        // if a bone has one or more children, point at first child
        Bone *childBone = bone->bones[0];
        qreal dx = childBone->x - bone->x;
        qreal dy = childBone->y - bone->y;
        if (qAbs(dx) > 0 || qAbs(dy) > 0) {
            boneLocalAngle = KisFastMath::atan2(dy, dx);
            boneLocalScaleX = sqrt(dx * dx + dy * dy) / 200;
        }
    }
    else if (bone->parentBone) {
        // else, if bone has parent, point away from parent
        qreal dx = bone->x - bone->parentBone->x;
        qreal dy = bone->y - bone->parentBone->y;
        if (qAbs(dx) > 0 || qAbs(dy) > 0) {
            boneLocalAngle = KisFastMath::atan2(dy, dx);
            boneLocalScaleX = sqrt(dx * dx + dy * dy) / 200;
        }
    }
    // adjust bone angle
    bone->fixLocalAngle += boneLocalAngle;
    bone->fixLocalScaleX *= boneLocalScaleX;

    // rotate all the child bones back to world position
    for (int i = 0; i < bone->bones.length(); ++i) {
        Bone *childBone = bone->bones[i];

        qreal tx = childBone->fixLocalX;
        qreal ty = childBone->fixLocalY;

        childBone->fixLocalX = tx * cos(-boneLocalAngle) - ty * sin(-boneLocalAngle);
        childBone->fixLocalY = tx * sin(-boneLocalAngle) + ty * cos(-boneLocalAngle);

        childBone->fixLocalX /= boneLocalScaleX;
        childBone->fixLocalAngle -= boneLocalAngle;
        childBone->fixLocalScaleX /= boneLocalScaleX;
    }

    // rotate all the child objects back to world position
    for (int i = 0; i < m_objects.length(); ++i) {
        if (m_objects[i].bone == bone) {
            m_objects[i].fixLocalAngle -= boneLocalAngle;
            m_objects[i].fixLocalScaleX /= boneLocalScaleX;
        }
    }

    // process all child bones
    for (int i = 0; i < bone->bones.length(); ++i) {
        fixBone(bone->bones[i]);
    }
}

void KisSpriterExport::writeBoneRef(const Bone *bone, QDomElement &key, QDomDocument &scml)
{
    if (!bone) return;
    QDomElement boneRef = scml.createElement("bone_ref");
    key.appendChild(boneRef);
    boneRef.setAttribute("id", bone->id);
    if (bone->parentBone) {
        boneRef.setAttribute("parent", bone->parentBone->id);
    }
    boneRef.setAttribute("timeline", m_timelineid++);
    boneRef.setAttribute("key", "0");
    Q_FOREACH(const Bone *childBone, bone->bones) {
        writeBoneRef(childBone, key, scml);
    }
}

void KisSpriterExport::writeBone(const Bone *bone, QDomElement &animation, QDomDocument &scml)
{
    if (!bone) return;
    QDomElement timeline = scml.createElement("timeline");
    animation.appendChild(timeline);
    timeline.setAttribute("id", m_timelineid);
    timeline.setAttribute("name", bone->name);
    timeline.setAttribute("object_type", "bone");

    QDomElement key = scml.createElement("key");
    timeline.appendChild(key);
    key.setAttribute("id", "0");
    key.setAttribute("spin", 0);

    QDomElement boneEl = scml.createElement("bone");
    key.appendChild(boneEl);
    boneEl.setAttribute("x", QString::number(bone->fixLocalX, 'f', 2));
    boneEl.setAttribute("y", QString::number(bone->fixLocalY, 'f', 2));
    boneEl.setAttribute("angle", QString::number(bone->fixLocalAngle, 'f', 2));
    boneEl.setAttribute("scale_x", QString::number(bone->fixLocalScaleX, 'f', 2));
    boneEl.setAttribute("scale_y", QString::number(bone->fixLocalScaleY, 'f', 2));

    m_timelineid++;

    Q_FOREACH(const Bone *childBone, bone->bones) {
        writeBone(childBone, animation, scml);
    }
}

void KisSpriterExport::fillScml(QDomDocument &scml, const QString &entityName)
{
    //qDebug() << "Creating scml" << entityName;

    QDomElement root = scml.createElement("spriter_data");
    scml.appendChild(root);
    root.setAttribute("scml_version", 1);
    root.setAttribute("generator", "krita");
    root.setAttribute("generator_version", qApp->applicationVersion());

    Q_FOREACH(const Folder &folder, m_folders) {
        QDomElement fe = scml.createElement("folder");
        root.appendChild(fe);
        fe.setAttribute("id", folder.id);
        fe.setAttribute("name", folder.name);
        Q_FOREACH(const SpriterFile &file, folder.files) {
            QDomElement fileElement = scml.createElement("file");
            fe.appendChild(fileElement);
            fileElement.setAttribute("id", file.id);
            fileElement.setAttribute("name", file.name);
            fileElement.setAttribute("width", QString::number(file.width, 'f', 2));
            fileElement.setAttribute("height", QString::number(file.height, 'f', 2));
            // qreal pivotX=0;
            // qreal pivotY=1;
            // Q_FOREACH(const SpriterObject &object, m_objects) {
                // if(file.id == object.fileId)
                // {
                    // pivotX = (0.0 -(object.fixLocalX / file.width));
                    // pivotY = (1.0 -(object.fixLocalY / file.height));
                    // break;
                // }
            // }
            // fileElement.setAttribute("pivot_x", QString::number(pivotX, 'f', 2));
            // fileElement.setAttribute("pivot_y", QString::number(pivotY, 'f', 2));
        }
    }

    // entity
    QDomElement entity = scml.createElement("entity");
    root.appendChild(entity);
    entity.setAttribute("id", "0");
    entity.setAttribute("name", entityName);

    // entity/animation
    QDomElement animation = scml.createElement("animation");
    entity.appendChild(animation);
    animation.setAttribute("id", "0");
    animation.setAttribute("name", "default");
    animation.setAttribute("length", "1000");
    animation.setAttribute("looping", "false");

    // entity/animation/mainline
    QDomElement mainline = scml.createElement("mainline");
    animation.appendChild(mainline);

    QDomElement key = scml.createElement("key");
    mainline.appendChild(key);
    key.setAttribute("id", "0");

    m_timelineid = 0;
    writeBoneRef(m_rootBone, key, scml);

    Q_FOREACH(const SpriterObject &object, m_objects) {
        QDomElement oe = scml.createElement("object_ref");
        key.appendChild(oe);
        oe.setAttribute("id", object.id);
        if (object.bone) {
            oe.setAttribute("parent", object.bone->id);
        }
        oe.setAttribute("timeline", m_timelineid++);
        oe.setAttribute("key", "0");
        oe.setAttribute("z_index", object.id);
    }

    // entity/animation/timeline
    m_timelineid = 0;
    if (m_rootBone) {
        writeBone(m_rootBone, animation, scml);
    }

    Q_FOREACH(const SpriterObject &object, m_objects) {
        Folder folder;
        Q_FOREACH(const Folder & f, m_folders) {
            if (f.id == object.folderId) {
                folder = f;
                break;
            }
        }
        SpriterFile file;
        file.id = -1;
        Q_FOREACH(const SpriterFile &f, folder.files) {
            if (f.id == object.fileId) {
                file = f;
                break;
            }
        }
        Q_ASSERT(file.id >= 0);

        QString objectName = "object-" + file.baseName;

        QDomElement timeline = scml.createElement("timeline");
        animation.appendChild(timeline);
        timeline.setAttribute("id", m_timelineid++);
        timeline.setAttribute("name", objectName);

        QDomElement key = scml.createElement("key");
        timeline.appendChild(key);
        key.setAttribute("id", "0");
        key.setAttribute("spin", "0");

        QDomElement objectEl = scml.createElement("object");
        key.appendChild(objectEl);
        objectEl.setAttribute("folder", object.folderId);
        objectEl.setAttribute("file", object.fileId);
        objectEl.setAttribute("x", object.fixLocalX);
        objectEl.setAttribute("y", object.fixLocalY);
        objectEl.setAttribute("angle", QString::number(kisRadiansToDegrees(object.fixLocalAngle), 'f', 2));
        objectEl.setAttribute("scale_x", QString::number(object.fixLocalScaleX, 'f', 2));
        objectEl.setAttribute("scale_y", QString::number(object.fixLocalScaleY, 'f', 2));
    }
}

Bone *findBoneByName(Bone *startBone, const QString &name)
{
    if (!startBone) return 0;
    //qDebug() << "findBoneByName" << name << "starting with" << startBone->name;

    if (startBone->name == name) {
        return startBone;
    }
    Q_FOREACH(Bone *child, startBone->bones) {
        //qDebug() << "looking for" << name << "found" << child->name;
        if (child->name == name) {
            return child;
        }
        Bone *grandChild = findBoneByName(child, name);
        if (grandChild){
            return grandChild;
        }
    }
    return 0;
}

KisImportExportErrorCode KisSpriterExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    QFileInfo fi(filename());

    m_image = document->savingImage();

    if (m_image->rootLayer()->childCount() == 0) {
        return ImportExportCodes::Failure;
    }

    KisGroupLayerSP root = m_image->rootLayer();

    m_boneLayer = qobject_cast<KisLayer*>(root->findChildByName("bone").data());
    //qDebug() << "Found boneLayer" << m_boneLayer;

    m_rootLayer= qobject_cast<KisGroupLayer*>(root->findChildByName("root").data());
    //qDebug() << "Fond rootLayer" << m_rootLayer;

    KisImportExportErrorCode result = parseFolder(m_image->rootLayer(), "", fi.absolutePath());
    if (!result.isOk()) {
        dbgFile << "There were errors encountered while using the spriter exporter.";
        return result;
    }

    m_rootBone = 0;

    if (m_rootLayer) {
        m_rootBone = parseBone(0, m_rootLayer);
    }
    // Generate objects
    int objectId = 0;
    for (int folderIndex = 0, folderCount = m_folders.size(); folderIndex < folderCount; ++folderIndex) {
        Folder folder = m_folders[folderCount - 1 - folderIndex];
        for (int fileIndex = 0, fileCount = folder.files.size(); fileIndex < fileCount; ++ fileIndex) {
            SpriterFile file = folder.files[fileCount - 1 - fileIndex];
            SpriterObject spriterObject;
            spriterObject.id = objectId++;
            spriterObject.folderId = folder.id;
            spriterObject.fileId = file.id;
            spriterObject.x = file.x;
            spriterObject.y = -file.y;
            Bone *bone = 0;

            //qDebug() << "file layername" << file.layerName;
            // layer.name format: "base_name bone(bone_name) slot(slot_name)"
            if (file.layerName.contains("bone(")) {
                int start = file.layerName.indexOf("bone(") + 5;
                int end = file.layerName.indexOf(')', start);
                QString boneName = file.layerName.mid(start, end - start);
                bone = findBoneByName(m_rootBone, boneName);
            }


            // layer.name format: "base_name"
            if (!bone && m_rootBone) {
                bone = findBoneByName(m_rootBone, file.layerName);
            }
            // group.name format: "base_name bone(bone_name)"
            if (!bone && m_rootBone) {
                if (folder.groupName.contains("bone(")) {
                    int start = folder.groupName.indexOf("bone(") + 5;
                    int end = folder.groupName.indexOf(')', start);
                    QString boneName = folder.groupName.mid(start, end - start);
                    bone = findBoneByName(m_rootBone, boneName);
                }

                // group.name format: "base_name"
                if (!bone) {
                    bone = findBoneByName(m_rootBone, folder.groupName);
                }
            }

            if (!bone) {
                bone = m_rootBone;
            }

            if (bone) {
                spriterObject.bone = bone;
                spriterObject.localX = spriterObject.x - bone->x;
                spriterObject.localY = spriterObject.y - bone->y;
            }
            else {
                spriterObject.bone = 0;
                spriterObject.localX = spriterObject.x;
                spriterObject.localY = spriterObject.y;
            }

            spriterObject.localAngle = 0;
            spriterObject.localScaleX = 1.0;
            spriterObject.localScaleY = 1.0;

            SpriterSlot *slot = 0;

            // layer.name format: "base_name bone(bone_name) slot(slot_name)"
            if (file.layerName.contains("slot(")) {
                int start = file.layerName.indexOf("slot(") + 5;
                int end = file.layerName.indexOf(')', start);
                slot = new SpriterSlot();
                slot->name = file.layerName.mid(start, end - start);
                slot->defaultAttachmentFlag = file.layerName.contains("*");
            }

            spriterObject.slot = slot;

//            qDebug() << "Created object" << spriterObject.id << spriterObject.folderId
//                     << spriterObject.fileId << spriterObject.x << spriterObject.y
//                     << spriterObject.localX << spriterObject.localY;

            m_objects.append(spriterObject);
        }
    }

    // Copy object transforms
    for (int i = 0; i < m_objects.size(); ++i) {
        m_objects[i].fixLocalX = m_objects[i].localX;
        m_objects[i].fixLocalY = m_objects[i].localY;
        m_objects[i].fixLocalAngle = m_objects[i].localAngle;
        m_objects[i].fixLocalScaleX = m_objects[i].localScaleX;
        m_objects[i].fixLocalScaleY = m_objects[i].localScaleY;
    }

    // Calculate bone angles
    if (m_rootBone) {
        copyBone(m_rootBone);
        fixBone(m_rootBone);
    }

    // Generate scml
    QDomDocument scml;
    fillScml(scml, fi.completeBaseName());

    bool openedHere = false;
    if (!io->isOpen()) {
        openedHere = io->open(QIODevice::WriteOnly);
        if (!openedHere) {
            // unsuccessful open
            return ImportExportCodes::NoAccessToWrite;
        }
    }

    QString towrite = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    if (io->write(towrite.toUtf8()) != towrite.length()) {
        return ImportExportCodes::ErrorWhileWriting;
    }
    towrite = scml.toString(4).toUtf8();
    if (io->write(towrite.toUtf8()) != towrite.length()) {
        return ImportExportCodes::ErrorWhileWriting;
    }

    delete m_rootBone;

    if (openedHere) {
        // FIXME: casues crash...
        //io->close();
    }

    return ImportExportCodes::OK;
}

void KisSpriterExport::initializeCapabilities()
{
    addCapability(KisExportCheckRegistry::instance()->get("MultiLayerCheck")->create(KisExportCheckBase::SUPPORTED));
    QList<QPair<KoID, KoID> > supportedColorModels;
    supportedColorModels << QPair<KoID, KoID>()
            << QPair<KoID, KoID>(RGBAColorModelID, Integer8BitsColorDepthID);
    addSupportedColorModels(supportedColorModels, "Spriter");
}



#include "kis_spriter_export.moc"
