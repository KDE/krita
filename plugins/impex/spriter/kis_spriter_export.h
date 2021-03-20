/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_SPRITER_EXPORT_H_
#define _KIS_SPRITER_EXPORT_H_

#include <QVariant>
#include <QDomDocument>
#include <QList>

#include <KisImportExportFilter.h>
#include <kis_types.h>

struct SpriterFile {
    qreal id;
    QString name;
    QString pathName;
    QString baseName;
    QString layerName;
    qreal width;
    qreal height;
    qreal x;
    qreal y;
};

struct Folder {
    qreal id;
    QString name;
    QString pathName;
    QString baseName;
    QString groupName;
    QList<SpriterFile> files;
};

struct Bone {
    qreal id;
    const Bone *parentBone;
    QString name;
    qreal x;
    qreal y;
    qreal width;
    qreal height;
    qreal localX;
    qreal localY;
    qreal localAngle;
    qreal localScaleX;
    qreal localScaleY;
    qreal fixLocalX;
    qreal fixLocalY;
    qreal fixLocalAngle;
    qreal fixLocalScaleX;
    qreal fixLocalScaleY;
    QList<Bone*> bones;

    ~Bone() {
        qDeleteAll(bones);
        bones.clear();
    }
};

struct SpriterSlot {
    QString name;
    bool defaultAttachmentFlag = false;
};

struct SpriterObject {
    qreal id;
    qreal folderId;
    qreal fileId;
    Bone *bone;
    SpriterSlot *slot;
    qreal x;
    qreal y;
    qreal localX;
    qreal localY;
    qreal localAngle;
    qreal localScaleX;
    qreal localScaleY;
    qreal fixLocalX;
    qreal fixLocalY;
    qreal fixLocalAngle;
    qreal fixLocalScaleX;
    qreal fixLocalScaleY;

    ~SpriterObject() {
        delete slot;
    }
};

class KisSpriterExport : public KisImportExportFilter
{
    Q_OBJECT
public:
    KisSpriterExport(QObject *parent, const QVariantList &);
    ~KisSpriterExport() override;
    bool supportsIO() const override { return false; }
    KisImportExportErrorCode convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration = 0) override;
    void initializeCapabilities() override;
private:

    KisImportExportErrorCode savePaintDevice(KisPaintDeviceSP dev, const QString &fileName);
    KisImportExportErrorCode parseFolder(KisGroupLayerSP parentGroup, const QString &folderName, const QString &basePath, int *folderId = 0);
    Bone *parseBone(const Bone *parent, KisGroupLayerSP groupLayer);
    void fixBone(Bone *bone);
    void fillScml(QDomDocument &scml, const QString &entityName);
    void writeBoneRef(const Bone *bone, QDomElement &mainline, QDomDocument &scml);
    void writeBone(const Bone *bone, QDomElement &timeline, QDomDocument &scml);

    KisImageSP m_image;
    qreal m_timelineid;
    QList<Folder> m_folders;
    Bone *m_rootBone;
    QList<SpriterObject> m_objects;
    KisGroupLayerSP m_rootLayer; // Not the image's root later, but the one that is named "root"
    KisLayerSP m_boneLayer;

};

#endif
