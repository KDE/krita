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
    qreal id {0.0};
    QString name;
    QString pathName;
    QString baseName;
    QString layerName;
    qreal width {0.0};
    qreal height {0.0};
    qreal x {0.0};
    qreal y {0.0};
};

struct Folder {
    qreal id {0.0};
    QString name;
    QString pathName;
    QString baseName;
    QString groupName;
    QList<SpriterFile> files;
};

struct Bone {
    qreal id {0.0};
    const Bone *parentBone {nullptr};
    QString name;
    qreal x {0.0};
    qreal y {0.0};
    qreal width {0.0};
    qreal height {0.0};
    qreal localX {0.0};
    qreal localY {0.0};
    qreal localAngle {0.0};
    qreal localScaleX {0.0};
    qreal localScaleY {0.0};
    qreal fixLocalX {0.0};
    qreal fixLocalY {0.0};
    qreal fixLocalAngle {0.0};
    qreal fixLocalScaleX {0.0};
    qreal fixLocalScaleY {0.0};
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
    qreal id {0.0};
    qreal folderId {0.0};
    qreal fileId {0.0};
    Bone *bone {nullptr};
    SpriterSlot *slot {nullptr};
    qreal x {0.0};
    qreal y {0.0};
    qreal localX {0.0};
    qreal localY {0.0};
    qreal localAngle {0.0};
    qreal localScaleX {0.0};
    qreal localScaleY {0.0};
    qreal fixLocalX {0.0};
    qreal fixLocalY {0.0};
    qreal fixLocalAngle {0.0};
    qreal fixLocalScaleX {0.0};
    qreal fixLocalScaleY {0.0};

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
    qreal m_timelineid {0.0};
    QList<Folder> m_folders;
    Bone *m_rootBone {nullptr};
    QList<SpriterObject> m_objects;
    KisGroupLayerSP m_rootLayer; // Not the image's root later, but the one that is named "root"
    KisLayerSP m_boneLayer;

};

#endif
