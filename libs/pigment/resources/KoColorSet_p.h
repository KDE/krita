/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KOCOLORSET_P_H
#define KOCOLORSET_P_H

#include <QHash>
#include <QXmlStreamReader>
#include <QDomElement>
#include <QPointer>
#include <kundo2stack.h>

#include <KisSwatch.h>
#include <KisSwatchGroup.h>

#include "KoColorSet.h"

struct RiffHeader {
    quint32 riff;
    quint32 size;
    quint32 signature;
    quint32 data;
    quint32 datasize;
    quint16 version;
    quint16 colorcount;
};

class KoColorSet::Private
{

public:
    Private(KoColorSet *a_colorSet);

public:
    KisSwatchGroupSP global() {
        Q_ASSERT(swatchGroups.size() > 0 && swatchGroups.first()->name() == GLOBAL_GROUP_NAME);
        return swatchGroups.first();
    }
public:
    bool init();

    bool saveGpl(QIODevice *dev) const;
    bool loadGpl();

    bool loadAct();
    bool loadRiff();
    bool loadPsp();
    bool loadAco();
    bool loadXml();
    bool loadSbz();
    bool loadAse();
    bool loadAcb();

    bool saveKpl(QIODevice *dev) const;
    bool loadKpl();

public:

    KoColorSet *colorSet {0};
    KoColorSet::PaletteType paletteType {UNKNOWN};
    QByteArray data;
    QString comment;
    QList<KisSwatchGroupSP> swatchGroups;
    KUndo2Stack undoStack;
    bool isLocked {false};
    int columns;

private:

    friend struct AddSwatchCommand;
    friend struct RemoveSwatchCommand;
    friend struct ChangeGroupNameCommand;
    friend struct AddGroupCommand;
    friend struct RemoveGroupCommand;
    friend struct ClearCommand;
    friend struct SetColumnCountCommand;
    friend struct SetCommentCommand;
    friend struct SetPaletteTypeCommand;
    friend struct MoveGroupCommand;

    KoColorSet::PaletteType detectFormat(const QString &fileName, const QByteArray &ba);
    void scribusParseColor(KoColorSet *set, QXmlStreamReader *xml);
    bool loadScribusXmlPalette(KoColorSet *set, QXmlStreamReader *xml);
    quint8 readByte(QIODevice *io);
    quint16 readShort(QIODevice *io);
    qint32 readInt(QIODevice *io);
    float readFloat(QIODevice *io);
    QString readUnicodeString(QIODevice *io, bool sizeIsInt = false);

    const KoColorProfile *loadColorProfile(QScopedPointer<KoStore> &store,
                                           const QString &path,
                                           const QString &modelId,
                                           const QString &colorDepthId);

    void saveKplGroup(QDomDocument &doc, QDomElement &groupEle,
                      const KisSwatchGroupSP group, QSet<const KoColorSpace *> &colorSetSet) const;
    bool loadKplProfiles(QScopedPointer<KoStore> &store);
    bool loadKplColorset(QScopedPointer<KoStore> &store);
    bool loadSbzSwatchbook(QScopedPointer<KoStore> &store);
    void loadKplGroup(const QDomDocument &doc, const QDomElement &parentElement, KisSwatchGroupSP group, QString version);
};

#endif // KOCOLORSET_P_H
