/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoFontGlyphModel.h"
#include <QDebug>
#include <hb.h>
#include <hb-ft.h>

struct KoFontGlyphModel::GlyphInfo {

    GlyphInfo()
    {
    }
    GlyphInfo(uint utf)
        : ucs(utf)
    {
    }
    uint ucs;
    GlyphType type = Base;
    QString baseString;
};

struct KoFontGlyphModel::CodePointInfo {
    uint ucs = -1;
    uint glyphIndex = 0;
    QString utfString = QString();

    QVector<GlyphInfo> glyphs;
};

struct KoFontGlyphModel::Private {
    QVector<CodePointInfo> codePoints;

    ~Private() = default;

    static QVector<KoFontGlyphModel::CodePointInfo> charMap(FT_FaceSP face) {
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
                gci.type = UnicodeVaritionSelector;
                gci.baseString = QString::fromUcs4(&hbVSPoint, 1);
                glyphs.append(gci);
                vsData.insert(hbCodePointPoint, glyphs);
            }
        }
        return vsData;
    }

    static QMap<uint, QVector<GlyphInfo>> getOpenTypeTables(FT_FaceSP face) {
        // All of this was referenced from Inkscape's OpenTypeUtil.cpp::readOpenTypeGsubTable
        QMap<uint, QVector<GlyphInfo>> otfData;
        hb_face_t_sp hbFace(hb_ft_face_create_referenced(face.data()));
        hb_tag_t table = HB_OT_TAG_GSUB;
        uint scriptCount = hb_ot_layout_table_get_script_tags(hbFace.data(), table, 0, nullptr, nullptr);
        hb_tag_t scriptTags[scriptCount];
        hb_ot_layout_table_get_script_tags(hbFace.data(), table, 0, &scriptCount, scriptTags);
        QVector<hb_tag_t> tags;
        for (uint i = 0; i < scriptCount; i++) {
            uint languageCount = hb_ot_layout_script_get_language_tags(hbFace.data(), table, i, 0, nullptr, nullptr);
            if(languageCount > 0) {

                for(uint j = 0; j < languageCount; j++) {
                    uint featureCount = hb_ot_layout_language_get_feature_tags(hbFace.data(),
                                                                               table,
                                                                               i,
                                                                               j,
                                                                               0, nullptr, nullptr);
                    hb_tag_t features[featureCount];
                    hb_ot_layout_language_get_feature_tags(hbFace.data(), table, i,
                                                           j,
                                                           0, &featureCount, features);
                    for(uint k = 0; k < featureCount; k++) {
                        tags.append(features[k]);
                    }
                }
            }
        }

        for (auto it = tags.begin(); it != tags.end(); it++) {
            char c[4];
            hb_tag_to_string(*it, c);
            QString tagName(c);
            uint featureIndex;
            bool found = hb_ot_layout_language_find_feature (hbFace.data(), table,
                                                             0,
                                                             HB_OT_LAYOUT_DEFAULT_LANGUAGE_INDEX,
                                                             *it,
                                                             &featureIndex );
            if (found) {
                uint lookup_indexes[32];
                uint lookup_count = 32;
                int count = hb_ot_layout_feature_get_lookups (hbFace.data(), table,
                                                              featureIndex,
                                                              0,
                                                              &lookup_count,
                                                              lookup_indexes );
                for (int i = 0; i < count; ++i) {
                    hb_set_t_sp glyphsBefore (hb_set_create());
                    hb_set_t_sp glyphsInput (hb_set_create());
                    hb_set_t_sp glyphsAfter (hb_set_create());
                    hb_set_t_sp glyphsOutput (hb_set_create());

                    hb_ot_layout_lookup_collect_glyphs (hbFace.data(), table,
                                                        lookup_indexes[i],
                                                        glyphsBefore.data(),
                                                        glyphsInput.data(),
                                                        glyphsAfter.data(),
                                                        glyphsOutput.data() );

                    GlyphInfo gci;
                    gci.type = OpenType;
                    gci.baseString = tagName;
                    hb_codepoint_t hbGlyphPoint  = HB_SET_VALUE_INVALID;
                    while(hb_set_next(glyphsInput.data(), &hbGlyphPoint)) {
                        QVector<GlyphInfo> glyphs = otfData.value(hbGlyphPoint);
                        glyphs.append(gci);
                        otfData.insert(hbGlyphPoint, glyphs);
                    }

                }

            }

        }
        return otfData;
    }
};



KoFontGlyphModel::KoFontGlyphModel(FT_FaceSP face)
    : QAbstractItemModel()
    , d(new Private)
{

    d->codePoints = Private::charMap(face);
    QMap<uint, QVector<GlyphInfo>> otfData = Private::getOpenTypeTables(face);
    QMap<uint, QVector<GlyphInfo>> VSData = Private::getVSData(face);
    for(auto it = d->codePoints.begin(); it != d->codePoints.end(); it++) {
        it->glyphs.append(VSData.value(it->ucs));
        QVector<GlyphInfo> otfTable = otfData.value(it->glyphIndex);
        for (auto glyphInfo = otfTable.begin(); glyphInfo != otfTable.end(); glyphInfo++) {
            glyphInfo->ucs = it->ucs;
            it->glyphs.append(*glyphInfo);
        }
    }

}

QVariant KoFontGlyphModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    if (role == Qt::DisplayRole) {
        if (!index.parent().isValid()) {
            return d->codePoints.value(index.row()).utfString;
        } else {
            return d->codePoints.value(index.parent().row()).glyphs.value(index.row()).baseString;
        }
    } else if (role == Qt::ToolTipRole) {
        if (!index.parent().isValid()) {
            QStringList glyphNames;
            QString base = d->codePoints.value(index.row()).utfString;
            QVector<GlyphInfo> glyphList = d->codePoints.value(index.row()).glyphs;
            glyphNames.append(QString("%1 glyph variants:").arg(glyphList.size()));
            for(auto glyph = glyphList.begin(); glyph != glyphList.end(); glyph++) {
                if (glyph->type == UnicodeVaritionSelector) {
                    glyphNames.append("UVS:"+base+glyph->baseString);
                } else if (glyph->type == OpenType) {
                    glyphNames.append("OTF:"+glyph->baseString);
                }
            }
            return glyphNames.join(" ");
        }
    }
    return QVariant();
}

QModelIndex KoFontGlyphModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        CodePointInfo cpi = d->codePoints.value(row);
        return createIndex(row, column, &cpi);
    } else {
        GlyphInfo gpi = d->codePoints.value(parent.row()).glyphs.value(row);
        return createIndex(row, column, &gpi);
    }
    return QModelIndex();
}

QModelIndex KoFontGlyphModel::parent(const QModelIndex &child) const
{
    if (!child.isValid() || static_cast<CodePointInfo*>(child.internalPointer())) {
        return QModelIndex();
    }
    GlyphInfo *gpi = static_cast<GlyphInfo*>(child.internalPointer());
    for (int i = 0; i < d->codePoints.size(); i++) {
        CodePointInfo cpi = d->codePoints.at(i);
        if (cpi.ucs == gpi->ucs) {
            return createIndex(i, 0, &cpi);
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
    if (parent.isValid())
        return !d->codePoints.value(parent.row()).glyphs.isEmpty();
    return false;
}

QModelIndex KoFontGlyphModel::indexForString(QString grapheme)
{
    for (int i = 0; i < d->codePoints.size(); i++) {
        if (grapheme.toUcs4().startsWith(d->codePoints.at(i).ucs)) {
            CodePointInfo cpi = d->codePoints.at(i);
            return createIndex(i, 1, &cpi);
        }
    }
    return QModelIndex();
}
