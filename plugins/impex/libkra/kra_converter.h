/*
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KRA_CONVERTER_H_
#define _KRA_CONVERTER_H_

#include <stdio.h>

#include <QObject>
#include <QDomDocument>

#include <KoStore.h>
#include <kis_png_converter.h>
#include <kis_types.h>
#include <kis_kra_saver.h>
#include <kis_kra_loader.h>
#include <KoProgressUpdater.h>
#include <QPointer>
#include <KoUpdater.h>

#include "kritalibkra_export.h"

class KisDocument;

class KRITALIBKRA_EXPORT KraConverter : public QObject
{
    Q_OBJECT

public:

    KraConverter(KisDocument *doc);
    KraConverter(KisDocument *doc, QPointer<KoUpdater> updater);
    ~KraConverter() override;

    KisImportExportErrorCode buildImage(QIODevice *io);
    KisImportExportErrorCode buildFile(QIODevice *io, const QString &filename, bool addMergedImage = true);
    /**
     * Retrieve the constructed image
     */
    KisImageSP image();
    vKisNodeSP activeNodes();
    QList<KisPaintingAssistantSP> assistants();
    StoryboardItemList storyboardItemList();
    QVector<StoryboardComment> storyboardCommentList();

public Q_SLOTS:

    virtual void cancel();

private:

    KisImportExportErrorCode saveRootDocuments(KoStore *store);
    bool saveToStream(QIODevice *dev);
    QDomDocument createDomDocument();
    KisImportExportErrorCode savePreview(KoStore *store);
    KisImportExportErrorCode oldLoadAndParse(KoStore *store, const QString &filename, QDomDocument &xmldoc);
    KisImportExportErrorCode loadXML(const QDomDocument &doc, KoStore *store);
    bool completeLoading(KoStore *store);

    void setProgress(int progress);

    KisDocument *m_doc {0};
    KisImageSP m_image;

    vKisNodeSP m_activeNodes;
    QList<KisPaintingAssistantSP> m_assistants;
    StoryboardItemList m_storyboardItemList;
    QVector<StoryboardComment> m_storyboardCommentList;
    bool m_stop {false};

    KoStore *m_store {0};
    KisKraSaver *m_kraSaver {0};
    KisKraLoader *m_kraLoader {0};
    QPointer<KoUpdater> m_updater;
};

#endif
