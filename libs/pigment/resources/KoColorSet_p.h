#ifndef KOCOLORSET_P_H
#define KOCOLORSET_P_H

#include <QHash>
#include <QXmlStreamReader>
#include <QPointer>

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

KoColorSet::PaletteType detectFormat(const QString &fileName, const QByteArray &ba);
void scribusParseColor(KoColorSet *set, QXmlStreamReader *xml);
bool loadScribusXmlPalette(KoColorSet *set, QXmlStreamReader *xml);

class KoColorSet::Private
{
public:
    Private(KoColorSet *a_colorSet);

public:
    KisSwatchGroup &global() {
        Q_ASSERT(groups.contains(GLOBAL_GROUP_NAME));
        return groups[GLOBAL_GROUP_NAME];
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

    bool saveKpl(QIODevice *dev) const;
    bool loadKpl();

public:
    KoColorSet *colorSet;
    KoColorSet::PaletteType paletteType;
    QByteArray data;
    QString comment;
    QStringList groupNames; //names of the groups, this is used to determine the order they are in.
    QHash<QString, KisSwatchGroup> groups; //grouped colors.
    bool isGlobal;

public:
    static QString GLOBAL_GROUP_NAME;
};

#endif // KOCOLORSET_P_H
