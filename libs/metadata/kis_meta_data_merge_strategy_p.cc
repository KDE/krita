/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_meta_data_merge_strategy_p.h"
#include <klocalizedstring.h>

#include "kis_debug.h"

#include "kis_meta_data_entry.h"
#include "kis_meta_data_schema.h"
#include "kis_meta_data_schema_registry.h"
#include "kis_meta_data_store.h"
#include "kis_meta_data_value.h"

using namespace KisMetaData;

//-------------------------------------------//
//------------ DropMergeStrategy ------------//
//-------------------------------------------//

DropMergeStrategy::DropMergeStrategy()
{
}

DropMergeStrategy::~DropMergeStrategy()
{
}

QString DropMergeStrategy::id() const
{
    return "Drop";
}
QString DropMergeStrategy::name() const
{
    return i18n("Drop");
}

QString DropMergeStrategy::description() const
{
    return i18n("Drop all meta data");
}

void DropMergeStrategy::merge(Store* dst, QList<const Store*> srcs, QList<double> score) const
{
    Q_UNUSED(dst);
    Q_UNUSED(srcs);
    Q_UNUSED(score);
    dbgMetaData << "Drop meta data";
}

//---------------------------------------//
//---------- DropMergeStrategy ----------//
//---------------------------------------//

PriorityToFirstMergeStrategy::PriorityToFirstMergeStrategy()
{
}

PriorityToFirstMergeStrategy::~PriorityToFirstMergeStrategy()
{
}

QString PriorityToFirstMergeStrategy::id() const
{
    return "PriorityToFirst";
}
QString PriorityToFirstMergeStrategy::name() const
{
    return i18n("Priority to first meta data");
}

QString PriorityToFirstMergeStrategy::description() const
{
    return i18n("Use in priority the meta data from the layers at the bottom of the stack.");
}

void PriorityToFirstMergeStrategy::merge(Store* dst, QList<const Store*> srcs, QList<double> score) const
{
    Q_UNUSED(score);
    dbgMetaData << "Priority to first meta data";

    Q_FOREACH (const Store* store, srcs) {
        QList<QString> keys = store->keys();
        Q_FOREACH (const QString & key, keys) {
            if (!dst->containsEntry(key)) {
                dst->addEntry(store->getEntry(key));
            }
        }
    }
}
//-------------------------------------------//
//------ OnlyIdenticalMergeStrategy ---------//
//-------------------------------------------//

OnlyIdenticalMergeStrategy::OnlyIdenticalMergeStrategy()
{
}

OnlyIdenticalMergeStrategy::~OnlyIdenticalMergeStrategy()
{
}

QString OnlyIdenticalMergeStrategy::id() const
{
    return "OnlyIdentical";
}
QString OnlyIdenticalMergeStrategy::name() const
{
    return i18n("Only identical");
}

QString OnlyIdenticalMergeStrategy::description() const
{
    return i18n("Keep only meta data that are identical");
}

void OnlyIdenticalMergeStrategy::merge(Store* dst, QList<const Store*> srcs, QList<double> score) const
{
    Q_UNUSED(score);
    dbgMetaData << "OnlyIdenticalMergeStrategy";
    dbgMetaData << "Priority to first meta data";

    Q_ASSERT(srcs.size() > 0);
    QList<QString> keys = srcs[0]->keys();
    Q_FOREACH (const QString & key, keys) {
        bool keep = true;
        const Entry& e = srcs[0]->getEntry(key);
        const Value& v = e.value();
        Q_FOREACH (const Store* store, srcs) {
            if (!(store->containsEntry(key) && e.value() == v)) {
                keep = false;
                break;
            }
        }
        if (keep) {
            dst->addEntry(e);
        }
    }
}

//-------------------------------------------//
//------------ SmartMergeStrategy -----------//
//-------------------------------------------//

SmartMergeStrategy::SmartMergeStrategy()
{
}

SmartMergeStrategy::~SmartMergeStrategy()
{
}

QString SmartMergeStrategy::id() const
{
    return "Smart";
}
QString SmartMergeStrategy::name() const
{
    return i18n("Smart");
}

QString SmartMergeStrategy::description() const
{
    return i18n("This merge strategy attempts to find the best solution for merging, "
                "for instance by merging the list of authors together, or keeping "
                "identical photographic information.");
}

struct ScoreValue {
    double score;
    Value value;
};

Value SmartMergeStrategy::election(QList<const Store*> srcs, QList<double> scores, const QString & key) const
{
    QList<ScoreValue> scoreValues;
    for (int i = 0; i < srcs.size(); i++) {
        if (srcs[i]->containsEntry(key)) {
            const Value& nv = srcs[i]->getEntry(key).value();
            if (nv.type() != Value::Invalid) {
                bool found = false;
                for (int j = 0; j < scoreValues.size(); j++) {
                    ScoreValue& sv = scoreValues[j];
                    if (sv.value == nv) {
                        found = true;
                        sv.score += scores[i];
                        break;
                    }
                }
                if (!found) {
                    ScoreValue sv;
                    sv.score = scores[i];
                    sv.value = nv;
                    scoreValues.append(sv);
                }
            }
        }
    }
    if (scoreValues.size() < 1) {
        warnMetaData << "SmartMergeStrategy::election returned less than 1 score value";
        return Value();
    }
    const ScoreValue* bestSv = 0;
    double bestScore = -1.0;
    Q_FOREACH (const ScoreValue& sv, scoreValues) {
        if (sv.score > bestScore) {
            bestScore = sv.score;
            bestSv = &sv;
        }
    }
    if (bestSv) {
        return bestSv->value;
    }
    else {
        return Value();
    }
}

void SmartMergeStrategy::mergeEntry(Store* dst, QList<const Store*> srcs, const KisMetaData::Schema* schema, const QString & identifier) const
{
    bool foundOnce = false;
    Value v(QList<Value>(), Value::OrderedArray);
    Q_FOREACH (const Store* store, srcs) {
        if (store->containsEntry(schema, identifier)) {
            v += store->getEntry(schema, identifier).value();
            foundOnce = true;
        }
    }
    if (foundOnce) {
        dst->getEntry(schema, identifier).value() = v;
    }
}

void SmartMergeStrategy::merge(Store* dst, QList<const Store*> srcs, QList<double> scores) const
{
    dbgMetaData << "Smart merging of meta data";
    Q_ASSERT(srcs.size() == scores.size());
    Q_ASSERT(srcs.size() > 0);
    if (srcs.size() == 1) {
        dst->copyFrom(srcs[0]);
        return;
    }
    // Initialize some schema
    const KisMetaData::Schema* dcSchema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::DublinCoreSchemaUri);
//     const KisMetaData::Schema* psSchema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::PhotoshopSchemaUri);
    const KisMetaData::Schema* XMPRightsSchema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::XMPRightsSchemaUri);
    const KisMetaData::Schema* XMPSchema = KisMetaData::SchemaRegistry::instance()->schemaFromUri(KisMetaData::Schema::XMPSchemaUri);
    // Sort the stores and scores
    {
        QMultiMap<double, const Store*> scores2srcs;
        for (int i = 0; i < scores.size(); ++i) {
            scores2srcs.insert(scores[i], srcs[i]);
        }
        srcs = scores2srcs.values();
        scores = scores2srcs.keys();
    }

    // First attempt to see if one of the store has a higher score than the others
    if (scores[0] > 2 * scores[1]) { // One of the store has a higher importance than the other ones
        dst->copyFrom(srcs[0]);
    } else {
        // Merge exif info


        // Election
        Q_FOREACH (const Store* store, srcs) {
            QList<QString> keys = store->keys();
            Q_FOREACH (const QString & key, keys) {
                if (!dst->containsEntry(key)) {
                    Value v = election(srcs, scores, key);
                    if (v.type() != Value::Invalid) {
                        dst->getEntry(key).value() = v;
                    }
                }
            }
        }

        // Compute rating
        double rating = 0.0;
        double norm = 0.0;
        for (int i = 0; i < srcs.size(); i++) {
            const Store* store = srcs[i];
            if (store->containsEntry(XMPSchema, "Rating")) {
                double score = scores[i];
                rating += score * store->getEntry(XMPSchema, "Rating").value().asVariant().toDouble();
                norm += score;
            }
        }
        if (norm > 0.01) {
            dst->getEntry(XMPSchema, "Rating").value() = QVariant((int)(rating / norm));
        }
    }
    // Merge the list of authors and keywords and other stuff
    mergeEntry(dst, srcs, dcSchema, "contributor");
    mergeEntry(dst, srcs, dcSchema, "creator");
    mergeEntry(dst, srcs, dcSchema, "publisher");
    mergeEntry(dst, srcs, dcSchema, "subject");
    mergeEntry(dst, srcs, XMPRightsSchema, "Owner");
    mergeEntry(dst, srcs, XMPSchema, "Identifier");
}
