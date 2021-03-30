/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KIS_KRA_LOADER_H
#define KIS_KRA_LOADER_H

class QString;
class QStringList;

#include "QDomDocument"
class KoStore;

class KisDocument;
class KoColorSpace;
class KisPaintingAssistant;
class StoryboardComment;

#include <kis_types.h>
#include "kritalibkra_export.h"
/**
 * Load old-style 1.x .kra files. Updated for 2.0, let's try to stay
 * compatible. But 2.0 won't be able to save 1.x .kra files unless we
 * implement an export filter.
 */
class KRITALIBKRA_EXPORT KisKraLoader
{

public:

    KisKraLoader(KisDocument * document, int syntaxVersion);

    ~KisKraLoader();

    /**
     * Loading is done in two steps: first all xml is loaded, then, in finishLoading,
     * the actual layer data is loaded.
     */
    KisImageSP loadXML(const QDomElement& elem);

    void loadBinaryData(KoStore* store, KisImageSP image, const QString & uri, bool external);

    void loadPalettes(KoStore *store, KisDocument *doc);
    void loadStoryboards(KoStore *store, KisDocument *doc);
    void loadAnimationMetadata(KoStore *store, KisImageSP image);

    vKisNodeSP selectedNodes() const;

    // it's neater to follow the same design as with selectedNodes, so let's have a getter here
    QList<KisPaintingAssistantSP> assistants() const;

    StoryboardItemList storyboardItemList() const;

    QVector<StoryboardComment> storyboardCommentList() const;

    /// if empty, loading didn't fail...
    QStringList errorMessages() const;

    /// if not empty, loading didn't fail, but there are problems
    QStringList warningMessages() const;

    /// Returns the name of the image as defined in maindoc.xml. This might
    /// be different from the name of the image as used in the path to the
    /// layers, because before Krita 4.2, under some circumstances, this
    /// string is in utf8, but the paths were stored in a different encoding.
    QString imageName() const;

private:

    // this needs to be private, for neatness sake
    void loadAssistants(KoStore* store, const QString & uri, bool external);

    void loadAnimationMetadataFromXML(const QDomElement& element, KisImageSP image);

    KisNodeSP loadNodes(const QDomElement& element, KisImageSP image, KisNodeSP parent);

    KisNodeSP loadNode(const QDomElement& elem, KisImageSP image);

    KisNodeSP loadPaintLayer(const QDomElement& elem, KisImageSP image, const QString& name, const KoColorSpace* cs, quint32 opacity);

    KisNodeSP loadGroupLayer(const QDomElement& elem, KisImageSP image, const QString& name, const KoColorSpace* cs, quint32 opacity);

    KisNodeSP loadAdjustmentLayer(const QDomElement& elem, KisImageSP image, const QString& name, const KoColorSpace* cs, quint32 opacity);

    KisNodeSP loadShapeLayer(const QDomElement& elem, KisImageSP image, const QString& name, const KoColorSpace* cs, quint32 opacity);

    KisNodeSP loadGeneratorLayer(const QDomElement& elem, KisImageSP image, const QString& name, const KoColorSpace* cs, quint32 opacity);

    KisNodeSP loadCloneLayer(const QDomElement& elem, KisImageSP image, const QString& name, const KoColorSpace* cs, quint32 opacity);

    KisNodeSP loadFilterMask(KisImageSP image, const QDomElement& elem);

    KisNodeSP loadTransformMask(KisImageSP image, const QDomElement& elem);

    KisNodeSP loadTransparencyMask(KisImageSP image, const QDomElement& elem);

    KisNodeSP loadSelectionMask(KisImageSP image, const QDomElement& elem);

    KisNodeSP loadColorizeMask(KisImageSP image, const QDomElement& elem, const KoColorSpace *colorSpace);

    KisNodeSP loadFileLayer(const QDomElement& elem, KisImageSP image, const QString& name, quint32 opacity);

    KisNodeSP loadReferenceImagesLayer(const QDomElement& elem, KisImageSP image);

    void loadNodeKeyframes(KoStore *store, const QString &location, KisNodeSP node);

    void loadCompositions(const QDomElement& elem, KisImageSP image);

    void loadAssistantsList(const QDomElement& elem);
    void loadGrid(const QDomElement& elem);
    void loadGuides(const QDomElement& elem);
    void loadMirrorAxis(const QDomElement& elem);
    void loadAudio(const QDomElement& elem, KisImageSP image);
    void loadStoryboardItemList(const QDomElement& elem);
    void loadStoryboardCommentList(const QDomElement& elem);
private:

    struct Private;
    Private * const m_d;

};

#endif
