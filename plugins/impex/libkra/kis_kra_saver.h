/* This file is part of the KDE project
 * Copyright 2008 (C) Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KIS_KRA_SAVER
#define KIS_KRA_SAVER

#include <kis_types.h>

class KisDocument;
class QDomElement;
class QDomDocument;
class KoStore;
class QString;
class QStringList;

#include "kritalibkra_export.h"

class KRITALIBKRA_EXPORT KisKraSaver
{
public:

    KisKraSaver(KisDocument* document, const QString &filename, bool addMergedImage = true);

    ~KisKraSaver();

    QDomElement saveXML(QDomDocument& doc,  KisImageSP image);

    bool saveKeyframes(KoStore *store, const QString &uri, bool external);

    bool saveBinaryData(KoStore* store, KisImageSP image, const QString & uri, bool external, bool addMergedImage);

    bool savePalettes(KoStore *store, KisImageSP image, const QString &uri);

    bool saveStoryboard(KoStore *store, KisImageSP image, const QString &uri);

    bool saveAnimationMetadata(KoStore *store, KisImageSP image, const QString &uri);

    /// @return a list with everything that went wrong while saving
    QStringList errorMessages() const;

private:
    void saveBackgroundColor(QDomDocument& doc, QDomElement& element, KisImageSP image);
    void saveAssistantsGlobalColor(QDomDocument& doc, QDomElement& element);
    void saveWarningColor(QDomDocument& doc, QDomElement& element, KisImageSP image);
    void saveCompositions(QDomDocument& doc, QDomElement& element, KisImageSP image);
    bool saveAssistants(KoStore *store,QString uri, bool external);
    bool saveAssistantsList(QDomDocument& doc, QDomElement& element);
    bool saveGrid(QDomDocument& doc, QDomElement& element);
    bool saveGuides(QDomDocument& doc, QDomElement& element);
    bool saveMirrorAxis(QDomDocument& doc, QDomElement& element);
    bool saveAudio(QDomDocument& doc, QDomElement& element);
    bool saveNodeKeyframes(KoStore *store, QString location, const KisNode *node);
    void savePalettesToXML(QDomDocument& doc, QDomElement &element);
    void saveStoryboardToXML(QDomDocument& doc, QDomElement &element);
    void saveAnimationMetadataToXML(QDomDocument& doc, QDomElement &element, KisImageSP image);

    struct Private;
    Private * const m_d;
};

#endif
