/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoFontGlyphModel.h"
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
        int layoutIndex = 0;
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
                    gci.layoutIndex = i;
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




KoFontGlyphModel::KoFontGlyphModel()
    : QAbstractItemModel()
    , d(new Private)
{
}

KoFontGlyphModel::~KoFontGlyphModel()
{

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
            if (glyph.type == UnicodeVaritionSelector) {
                return QString(codePoint.utfString + glyph.baseString);
            } else {
                return codePoint.utfString;
            }
        }
    } else if (role == Qt::ToolTipRole) {
        if (!index.parent().isValid()) {
            QStringList glyphNames;
            QString base = d->codePoints.value(index.row()).utfString;
            QVector<Private::GlyphInfo> glyphList = d->codePoints.value(index.row()).glyphs;
            glyphNames.append(QString("%1 glyph variants:").arg(glyphList.size()));
            for(auto glyph = glyphList.begin(); glyph != glyphList.end(); glyph++) {
                if (glyph->type == UnicodeVaritionSelector) {
                    glyphNames.append("UVS:"+base+glyph->baseString);
                } else if (glyph->type == OpenType) {
                    glyphNames.append("OTF:"+glyph->baseString+" "+QString::number(glyph->layoutIndex));
                }
            }
            return glyphNames.join(" ");
        } else {
            Private::CodePointInfo codePoint = d->codePoints.value(index.parent().row());
            Private::GlyphInfo glyph = codePoint.glyphs.value(index.row());
            if (glyph.type == OpenType) {
                return QString("OTF:"+glyph.baseString+" "+QString::number(glyph.layoutIndex));
            } else {
                return QString(codePoint.utfString + glyph.baseString);
            }
        }
    } else if (role == OpenTypeFeatures) {
        QStringList features;
        if (index.parent().isValid()) {
            Private::CodePointInfo codePoint = d->codePoints.value(index.parent().row());
            Private::GlyphInfo glyph = codePoint.glyphs.value(index.row());
            //qDebug () << index.parent().row() << index.row() << codePoint.utfString << codePoint.ucs;
            if (glyph.type == OpenType) {
                features.append("'"+glyph.baseString+"' 1");
            }

        }
        return features;
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

    if (node->parentUcs == node->ucs) {
        for(int i = 0; i < d->codePoints.size(); i++) {
            Private::CodePointInfo info = d->codePoints.at(i);
            if (info.ucs == node->ucs) {
                return index(i, 0, QModelIndex());
            }
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

void KoFontGlyphModel::setFace(FT_FaceSP face)
{
    beginResetModel();
    d->codePoints = Private::charMap(face);
    QMap<uint, QVector<Private::GlyphInfo>> otfData = Private::getOpenTypeTables(face);
    QMap<uint, QVector<Private::GlyphInfo>> VSData = Private::getVSData(face);
    for(auto it = d->codePoints.begin(); it != d->codePoints.end(); it++) {
        it->glyphs.append(VSData.value(it->ucs));
        QVector<Private::GlyphInfo> otfTable = otfData.value(it->glyphIndex);
        for (auto glyphInfo = otfTable.begin(); glyphInfo != otfTable.end(); glyphInfo++) {
            glyphInfo->ucs = it->ucs;
            glyphInfo->parentUcs = it->ucs;
            it->glyphs.append(*glyphInfo);
        }
    }
    endResetModel();
}

QHash<int, QByteArray> KoFontGlyphModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles[OpenTypeFeatures] = "openType";
    return roles;
}
