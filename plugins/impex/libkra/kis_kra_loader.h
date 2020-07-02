/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2007
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KIS_KRA_LOADER_H
#define KIS_KRA_LOADER_H

class QString;
class QStringList;

#include "KoXmlReaderForward.h"
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
    KisImageSP loadXML(const KoXmlElement& elem);

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

    void loadAnimationMetadataFromXML(const KoXmlElement& element, KisImageSP image);

    KisNodeSP loadNodes(const KoXmlElement& element, KisImageSP image, KisNodeSP parent);

    KisNodeSP loadNode(const KoXmlElement& elem, KisImageSP image);

    KisNodeSP loadPaintLayer(const KoXmlElement& elem, KisImageSP image, const QString& name, const KoColorSpace* cs, quint32 opacity);

    KisNodeSP loadGroupLayer(const KoXmlElement& elem, KisImageSP image, const QString& name, const KoColorSpace* cs, quint32 opacity);

    KisNodeSP loadAdjustmentLayer(const KoXmlElement& elem, KisImageSP image, const QString& name, const KoColorSpace* cs, quint32 opacity);

    KisNodeSP loadShapeLayer(const KoXmlElement& elem, KisImageSP image, const QString& name, const KoColorSpace* cs, quint32 opacity);

    KisNodeSP loadGeneratorLayer(const KoXmlElement& elem, KisImageSP image, const QString& name, const KoColorSpace* cs, quint32 opacity);

    KisNodeSP loadCloneLayer(const KoXmlElement& elem, KisImageSP image, const QString& name, const KoColorSpace* cs, quint32 opacity);

    KisNodeSP loadFilterMask(KisImageSP image, const KoXmlElement& elem);

    KisNodeSP loadTransformMask(KisImageSP image, const KoXmlElement& elem);

    KisNodeSP loadTransparencyMask(KisImageSP image, const KoXmlElement& elem);

    KisNodeSP loadSelectionMask(KisImageSP image, const KoXmlElement& elem);

    KisNodeSP loadColorizeMask(KisImageSP image, const KoXmlElement& elem, const KoColorSpace *colorSpace);

    KisNodeSP loadFileLayer(const KoXmlElement& elem, KisImageSP image, const QString& name, quint32 opacity);

    KisNodeSP loadReferenceImagesLayer(const KoXmlElement& elem, KisImageSP image);

    void loadNodeKeyframes(KoStore *store, const QString &location, KisNodeSP node);

    void loadCompositions(const KoXmlElement& elem, KisImageSP image);

    void loadAssistantsList(const KoXmlElement& elem);
    void loadGrid(const KoXmlElement& elem);
    void loadGuides(const KoXmlElement& elem);
    void loadMirrorAxis(const KoXmlElement& elem);
    void loadAudio(const KoXmlElement& elem, KisImageSP image);
    void loadStoryboardItemList(const KoXmlElement& elem);
    void loadStoryboardCommentList(const KoXmlElement& elem);
private:

    struct Private;
    Private * const m_d;

};

#endif
