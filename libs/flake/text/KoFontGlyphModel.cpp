/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoFontGlyphModel.h"
#include "KoOpenTypeFeatureInfoFactory.h"
#include <QDebug>
#include <hb.h>
#include <hb-ft.h>

struct KoFontGlyphModel::Private {
    struct InfoNode {
        virtual ~InfoNode() {}
        uint ucs = 0;
        uint parentUcs = 0;
        virtual int childCount() = 0;
    };

    struct GlyphInfo
            : public InfoNode {

        GlyphInfo()
        {
        }
        GlyphInfo(uint utf)
        {
            ucs = utf;
            parentUcs = utf;
        }
        ~GlyphInfo() override {}
        GlyphType type = Base;
        QString baseString;
        int featureIndex = -1;
        int childCount() override {
            return 0;
        }
    };

    struct CodePointInfo
            : public InfoNode {
        ~CodePointInfo() override {}

        uint glyphIndex = 0;
        QString utfString = QString();

        QVector<GlyphInfo> glyphs;
        int childCount() override {
            return glyphs.size();
        }
    };
    QVector<CodePointInfo> codePoints;
    QMap<QString, KoOpenTypeFeatureInfo> featureData;
    QVector<KoUnicodeBlockData> blocks;

    ~Private() = default;


    static QVector<CodePointInfo> charMap(FT_FaceSP face) {
        QVector<CodePointInfo> codePoints;

        FT_UInt   gindex;
        FT_ULong  charcode = FT_Get_First_Char(face.data(), &gindex);

        while (gindex != 0) {
            CodePointInfo cpi;
            cpi.ucs = charcode;
            cpi.glyphIndex = gindex;
            cpi.utfString = QString::fromUcs4(&cpi.ucs, 1);
            cpi.glyphs.append(GlyphInfo(cpi.ucs));

            codePoints.append(cpi);
            charcode = FT_Get_Next_Char(face.data(), charcode, &gindex);
        }

        return codePoints;
    }

    static QMap<uint, QVector<GlyphInfo>> getVSData(FT_FaceSP face) {
        QMap<uint, QVector<GlyphInfo>> vsData;
        hb_face_t_sp hbFace(hb_ft_face_create_referenced(face.data()));
        hb_set_t_sp variationSelectors(hb_set_create());
        hb_face_collect_variation_selectors(hbFace.data(), variationSelectors.data());

        hb_codepoint_t hbVSPoint = HB_SET_VALUE_INVALID;

        while(hb_set_next(variationSelectors.data(), &hbVSPoint)) {
            hb_set_t_sp unicodes(hb_set_create());

            hb_face_collect_variation_unicodes(hbFace.data(), hbVSPoint, unicodes.data());
            hb_codepoint_t hbCodePointPoint  = HB_SET_VALUE_INVALID;
            while(hb_set_next(unicodes.data(), &hbCodePointPoint)) {
                QVector<GlyphInfo> glyphs = vsData.value(hbCodePointPoint);
                GlyphInfo gci(hbCodePointPoint);
                gci.type = UnicodeVariationSelector;
                gci.baseString = QString::fromUcs4(&hbVSPoint, 1);
                glyphs.append(gci);
                vsData.insert(hbCodePointPoint, glyphs);
            }
        }
        return vsData;
    }

    static QMap<QString, KoOpenTypeFeatureInfo> getOpenTypeTables(FT_FaceSP face, QVector<CodePointInfo> &charMap, QMap<QString, KoOpenTypeFeatureInfo> previousFeatureInfo, bool gpos, QStringList locales, QLatin1String lang = QLatin1String()) {
        // All of this was referenced from Inkscape's OpenTypeUtil.cpp::readOpenTypeGsubTable
        QMap<QString, KoOpenTypeFeatureInfo> featureInfo = previousFeatureInfo;
        hb_face_t_sp hbFace(hb_ft_face_create_referenced(face.data()));
        hb_tag_t table = gpos? HB_OT_TAG_GPOS: HB_OT_TAG_GSUB;
        uint targetLanguage = HB_OT_LAYOUT_DEFAULT_LANGUAGE_INDEX;
        uint targetScript = 0;

        QVector<hb_language_t> localeTags;
        Q_FOREACH(const QString locale, locales) {
            QLatin1String l(locale.split("_").join("-").toLatin1());
            localeTags.append(hb_language_from_string(l.data(), l.size()));
        }

        hb_language_t languageTag = lang.isEmpty()? HB_LANGUAGE_INVALID: hb_language_from_string(lang.data(), lang.size());
        uint scriptCount = hb_ot_layout_table_get_script_tags(hbFace.data(), table, 0, nullptr, nullptr);
        QVector<hb_tag_t> tags;
        for (uint script = 0; script < scriptCount; script++) {
            uint languageCount = hb_ot_layout_script_get_language_tags(hbFace.data(), table, script, 0, nullptr, nullptr);

                for(uint j = 0; j < languageCount; j++) {
                    hb_tag_t langTag;
                    uint count = 1;
                    hb_ot_layout_script_get_language_tags(hbFace.data(), table, script, j, &count, &langTag);
                    if (count < 1) continue;
                    if (hb_ot_tag_to_language(langTag) == languageTag) {
                        targetLanguage = j;
                        targetScript = script;
                        break;
                    }
                }
        }

        uint featureCount = hb_ot_layout_language_get_feature_tags(hbFace.data(),
                                                                   table,
                                                                   targetScript,
                                                                   targetLanguage,
                                                                   0, nullptr, nullptr);
        hb_ot_layout_language_get_feature_tags(hbFace.data(), table, targetScript,
                                               targetLanguage,
                                               0, nullptr, nullptr);
        for(uint k = 0; k < featureCount; k++) {
            uint count = 1;
            hb_tag_t features;
            hb_ot_layout_language_get_feature_tags(hbFace.data(), table, targetScript,
                                                   targetLanguage,
                                                   k, &count, &features);
            if (count < 1) continue;
            tags.append(features);
        }

        KoOpenTypeFeatureInfoFactory factory;
        for (auto it = tags.begin(); it != tags.end(); it++) {
            char c[4];
            hb_tag_to_string(*it, c);
            const QLatin1String tagName(c, 4);
            uint featureIndex;

            QVector<uint> lookUpsProcessed;
            bool found = hb_ot_layout_language_find_feature (hbFace.data(), table,
                                                             targetScript,
                                                             targetLanguage,
                                                             *it,
                                                             &featureIndex );

            if (!found) {
                continue;
            }

            hb_ot_name_id_t labelId = HB_OT_NAME_ID_INVALID;
            hb_ot_name_id_t toolTipId = HB_OT_NAME_ID_INVALID;
            hb_ot_name_id_t sampleId = HB_OT_NAME_ID_INVALID;
            uint namedParameters;
            hb_ot_name_id_t firstParamId = HB_OT_NAME_ID_INVALID;
            KoOpenTypeFeatureInfo info = factory.infoByTag(tagName);
            if (hb_ot_layout_feature_get_name_ids(hbFace.data(), table, featureIndex, &labelId, &toolTipId, &sampleId, &namedParameters, &firstParamId)) {
                QVector<hb_ot_name_id_t> nameIds = {labelId, toolTipId, sampleId};
                if (firstParamId != HB_OT_NAME_ID_INVALID) {
                    for (uint i = 0; i < namedParameters; i++) {
                        nameIds += firstParamId + i;
                    }
                }

                for(auto nameId = nameIds.begin(); nameId != nameIds.end(); nameId++) {
                    if (*nameId == HB_OT_NAME_ID_INVALID) {
                        continue;
                    }
                    QVector<hb_language_t> testLang;
                    uint length = 0;
                    if (*nameId == sampleId) {
                        testLang.append(languageTag);
                    } else {
                        testLang = localeTags;
                    }
                    testLang.append(HB_LANGUAGE_INVALID);
                    for (auto tag = testLang.begin(); tag != testLang.end(); tag++) {
                        length = hb_ot_name_get_utf8(hbFace.data(), *nameId, *tag, nullptr, nullptr);
                        if (length > 0) {
                            length+=1;
                            break;
                        }
                    }

                    std::vector<char> buff(length);
                    hb_ot_name_get_utf8(hbFace.data(), *nameId, languageTag, &length, buff.data());
                    if (length > 0) {
                        const QString nameString = QString::fromUtf8(buff.data(), length);
                        if (*nameId == labelId) {
                            info.name = nameString;
                        } else if (*nameId == toolTipId) {
                            info.description = nameString;
                        } else if (*nameId == sampleId) {
                            info.sample = nameString;
                        } else {
                            info.namedParameters.append(nameString);
                        }
                    }
                }
            }

            featureInfo.insert(tagName, info);
            if (!info.glyphPalette) {
                continue;
            }

            QStringList samples;
            int lookupCount = hb_ot_layout_feature_get_lookups (hbFace.data(), table,
                                                                featureIndex,
                                                                0,
                                                                nullptr,
                                                                nullptr );
            for (int i = 0; i < lookupCount; ++i) {
                uint maxCount = 1;
                uint lookUpIndex = 0;
                hb_ot_layout_feature_get_lookups (hbFace.data(), table,
                                                  featureIndex,
                                                  i,
                                                  &maxCount,
                                                  &lookUpIndex );
                if (maxCount < 1 || lookUpsProcessed.contains(lookUpIndex)) {
                    continue;
                }

                // https://github.com/harfbuzz/harfbuzz/issues/673 suggest against checking the lookups,
                // but if we only check for alternates, features like small-caps etc. won't be found (being gpos type 1 instead of type 3)
                // Given this is run only when the model is created, this should be fine for now.

                hb_set_t_sp glyphsBefore (hb_set_create());
                hb_set_t_sp glyphsInput (hb_set_create());
                hb_set_t_sp glyphsAfter (hb_set_create());
                hb_set_t_sp glyphsOutput (hb_set_create());

                hb_ot_layout_lookup_collect_glyphs (hbFace.data(), table,
                                                    lookUpIndex,
                                                    glyphsBefore.data(),
                                                    glyphsInput.data(),
                                                    glyphsAfter.data(),
                                                    glyphsOutput.data() );


                GlyphInfo gci;
                gci.type = OpenType;
                gci.baseString = tagName;

                for(auto it = charMap.begin(); it != charMap.end(); it++) {
                    gci.ucs = it->ucs;
                    hb_codepoint_t glyphIndex = it->glyphIndex;
                    bool addSample = false;

                    uint alt_count = hb_ot_layout_lookup_get_glyph_alternates (hbFace.data(),
                                                                               lookUpIndex, glyphIndex,
                                                                               0,
                                                                               nullptr, nullptr);

                    if (alt_count > 0) {
                        // 0 is the default value.
                        for(uint j = 1; j < alt_count; ++j) {
                            gci.featureIndex = j;
                            it->glyphs.append(gci);
                        }
                        info.maxValue = qMax(int(alt_count), info.maxValue);
                        addSample = true;
                    } else if (hb_set_has(glyphsInput.data(), glyphIndex)) {
                        gci.featureIndex = 1;
                        it->glyphs.append(gci);
                        addSample = true;
                    }
                    if (samples.size() < 6 && addSample) {
                        samples.append(QString::fromUcs4(&gci.ucs, 1));
                    }
                }
                lookUpsProcessed.append(lookUpIndex);
                if (info.sample.isEmpty() && !samples.isEmpty()) {
                    info.sample = samples.join(" ");
                }
                featureInfo.insert(tagName, info);

            }

        }


        return featureInfo;
    }
};




KoFontGlyphModel::KoFontGlyphModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d(new Private)
{
}

KoFontGlyphModel::~KoFontGlyphModel()
{

}

QString unicodeHexFromUCS(const uint codePoint) {
    QByteArray ba;
    ba.setNum(codePoint, 16);
    QString hex = QString(ba);
    return QString("U+%1").arg(hex, hex.size() > 4? 6: 4, '0');
}

QVariant KoFontGlyphModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    if (role == Qt::DisplayRole) {
        //qDebug() << Q_FUNC_INFO<< index << index.parent().isValid() << index.parent();
        if (!index.parent().isValid()) {
            return d->codePoints.value(index.row()).utfString;
        } else {
            Private::CodePointInfo codePoint = d->codePoints.value(index.parent().row());
            Private::GlyphInfo glyph = codePoint.glyphs.value(index.row());
            //qDebug () << index.parent().row() << index.row() << codePoint.utfString << codePoint.ucs;
            if (glyph.type == UnicodeVariationSelector) {
                return QString(codePoint.utfString + glyph.baseString);
            } else {
                return codePoint.utfString;
            }
        }
    } else if (role == Qt::ToolTipRole) {
        if (!index.parent().isValid()) {
            QStringList glyphNames;
            Private::CodePointInfo codePoint = d->codePoints.value(index.row());
            QString base = codePoint.utfString;
            QVector<Private::GlyphInfo> glyphList = codePoint.glyphs;
            glyphNames.append(QString("%1 (%2)").arg(base).arg(unicodeHexFromUCS(codePoint.ucs)));
            if (glyphList.size() > 0) {
                glyphNames.append(i18nc("@info:tooltip", "%1 glyph variants.").arg(glyphList.size()));
            }
            return glyphNames.join(" ");
        } else {
            Private::CodePointInfo codePoint = d->codePoints.value(index.parent().row());
            Private::GlyphInfo glyph = codePoint.glyphs.value(index.row());
            if (glyph.type == OpenType) {
                KoOpenTypeFeatureInfo info = d->featureData.value(glyph.baseString);
                QString parameterString = info.namedParameters.value(glyph.featureIndex-1);
                if (parameterString.isEmpty()) {
                    return info.name.isEmpty()? glyph.baseString: info.name;
                } else {
                    return QString("%1: %2").arg(info.name).arg(parameterString);
                }
            } else {
                return QString(codePoint.utfString + glyph.baseString);
            }
        }
    } else if (role == OpenTypeFeatures) {
        QVariantMap features;
        if (index.parent().isValid()) {
            Private::CodePointInfo codePoint = d->codePoints.value(index.parent().row());
            Private::GlyphInfo glyph = codePoint.glyphs.value(index.row());
            //qDebug () << index.parent().row() << index.row() << codePoint.utfString << codePoint.ucs;
            if (glyph.type == OpenType) {
                features.insert(glyph.baseString, glyph.featureIndex);
            }
        }
        return features;
    } else if (role == GlyphLabel) {
        QString glyphId;
        if (!index.parent().isValid()) {
            Private::CodePointInfo codePoint = d->codePoints.value(index.row());
            glyphId = unicodeHexFromUCS(codePoint.ucs);
        } else {
            Private::CodePointInfo codePoint = d->codePoints.value(index.parent().row());
            Private::GlyphInfo glyph = codePoint.glyphs.value(index.row());
            if (glyph.type == OpenType) {
                glyphId = QString("'%1' %2").arg(glyph.baseString).arg(glyph.featureIndex);
            } else if (glyph.type == UnicodeVariationSelector)  {
                glyphId = unicodeHexFromUCS(glyph.baseString.toUcs4().first());
            } else {
                glyphId = unicodeHexFromUCS(codePoint.ucs);
            }
        }
        return glyphId;
    } else if (role == ChildCount) {
        int childCount = 0;
        if (!index.parent().isValid()) {
            Private::CodePointInfo codePoint = d->codePoints.value(index.row());
            childCount = codePoint.childCount();
        }
        return childCount;
    }
    return QVariant();
}

QModelIndex KoFontGlyphModel::index(int row, int column, const QModelIndex &parent) const
{
    //qDebug() << Q_FUNC_INFO << row  << column << parent;
    if (parent.isValid() && parent.row() >= 0 && parent.row() < d->codePoints.size()) {
        Private::CodePointInfo info = d->codePoints.at(parent.row());
        if (row >= 0 && row < info.glyphs.size()) {
            return createIndex(row, column, &info.glyphs[row]);
        }

    } else if (row >= 0 && row < d->codePoints.size()) {
        return createIndex(row, column);
    }
    return QModelIndex();
}

QModelIndex KoFontGlyphModel::parent(const QModelIndex &child) const
{
    if (!child.isValid() || !child.internalPointer()) {
        return QModelIndex();
    }
    Private::InfoNode *node = static_cast<Private::InfoNode*>(child.internalPointer());

    for(int i = 0; i < d->codePoints.size(); i++) {
        Private::CodePointInfo info = d->codePoints.at(i);
        if (info.ucs == node->ucs) {
            return index(i, 0, QModelIndex());
        }
    }


    return QModelIndex();
}

int KoFontGlyphModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return d->codePoints.value(parent.row()).glyphs.size();
    }
    return d->codePoints.size();
}

int KoFontGlyphModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

bool KoFontGlyphModel::hasChildren(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return !d->codePoints.value(parent.row()).glyphs.isEmpty();
    }
    return false;
}

QModelIndex KoFontGlyphModel::indexForString(QString grapheme)
{
    for (int i = 0; i < d->codePoints.size(); i++) {
        if (grapheme.toUcs4().startsWith(d->codePoints.at(i).ucs)) {
            QModelIndex idx = index(i, 0, QModelIndex());
            return idx;
        }
    }
    return QModelIndex();
}

static bool sortBlocks(const KoUnicodeBlockData &a, const KoUnicodeBlockData &b) {
    return a.start < b.start;
}

void KoFontGlyphModel::setFace(FT_FaceSP face, QLatin1String language)
{
    beginResetModel();
    d->codePoints = Private::charMap(face);
    d->blocks.clear();
    QMap<uint, QVector<Private::GlyphInfo>> VSData = Private::getVSData(face);
    KoUnicodeBlockDataFactory blockFactory;

    for(auto it = d->codePoints.begin(); it != d->codePoints.end(); it++) {
        it->glyphs.append(VSData.value(it->ucs));

        auto block = d->blocks.begin();
        for (; block != d->blocks.end(); block++) {
            if (block->match(it->ucs)) {
                break;
            }
        }
        if (block == d->blocks.end()) {
            KoUnicodeBlockData newBlock = blockFactory.blockForUCS(it->ucs);
            if (newBlock != KoUnicodeBlockDataFactory::noBlock()) {
                d->blocks.append(newBlock);
            }
        }
    }
    std::sort(d->blocks.begin(), d->blocks.end(), sortBlocks);
    d->featureData = Private::getOpenTypeTables(face, d->codePoints, QMap<QString, KoOpenTypeFeatureInfo>() ,false, KLocalizedString::languages(), language);
    d->featureData = Private::getOpenTypeTables(face, d->codePoints, d->featureData, true, KLocalizedString::languages(), language);

    endResetModel();
}

QHash<int, QByteArray> KoFontGlyphModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[OpenTypeFeatures] = "openType";
    roles[GlyphLabel] = "glyphLabel";
    roles[ChildCount] = "childCount";
    return roles;
}

QVector<KoUnicodeBlockData> KoFontGlyphModel::blocks() const
{
    return d->blocks;
}

QMap<QString, KoOpenTypeFeatureInfo> KoFontGlyphModel::featureInfo() const
{
    return d->featureData;
}
