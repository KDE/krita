/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2005 Bart Coppens <kde@bartcoppens.be>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_pipebrush_parasite.h"

KisPipeBrushParasite::KisPipeBrushParasite(QStringView source)
{
    init();
    needsMovement = false;

    const QList<QStringView> parasites = source.split(QLatin1Char(' '), Qt::SkipEmptyParts);

    for (int i = 0; i < parasites.count(); i++) {
        const QList<QStringView> split = parasites.at(i).split(QLatin1Char(':'), Qt::SkipEmptyParts);

        if (split.count() != 2) {
            warnImage << "Wrong count for this parasite key/value:" << parasites.at(i);
            continue;
        }
        const QStringView &index = split.at(0);
        if (index == QLatin1String("dim")) {
            dim = (split.at(1)).toInt();
            if (dim < 1 || dim > MaxDim) {
                dim = 1;
            }
        } else if (index.startsWith(QLatin1String("sel"))) {
            int selIndex = index.mid(strlen("sel")).toInt();

            if (selIndex >= 0 && selIndex < dim) {
                selectionMode = split.at(1).toString();

                if (selectionMode == QLatin1String("incremental")) {
                    selection[selIndex] = KisParasite::Incremental;
                } else if (selectionMode == QLatin1String("angular")) {
                    selection[selIndex] = KisParasite::Angular;
                    needsMovement = true;
                } else if (selectionMode == QLatin1String("random")) {
                    selection[selIndex] = KisParasite::Random;
                } else if (selectionMode == QLatin1String("pressure")) {
                    selection[selIndex] = KisParasite::Pressure;
                } else if (selectionMode == QLatin1String("xtilt")) {
                    selection[selIndex] = KisParasite::TiltX;
                } else if (selectionMode == QLatin1String("ytilt")) {
                    selection[selIndex] = KisParasite::TiltY;
                } else if (selectionMode == QLatin1String("velocity")) {
                    selection[selIndex] = KisParasite::Velocity;
                } else {
                    selection[selIndex] = KisParasite::Constant;
                }
            }
            else {
                warnImage << "Sel: wrong index: " << selIndex << "(dim = " << dim << ")";
            }
        } else if (index.startsWith(QLatin1String("rank"))) {
            int rankIndex = index.mid(strlen("rank")).toInt();
            if (rankIndex < 0 || rankIndex > dim) {
                warnImage << "Rankindex out of range: " << rankIndex;
                continue;
            }
            rank[rankIndex] = (split.at(1)).toInt();
        } else if (index == QLatin1String("ncells")) {
            ncells = (split.at(1)).toInt();
            if (ncells < 1) {
                warnImage << "ncells out of range: " << ncells;
                ncells = 1;
            }
        }
    }

    for (int i = 0; i < dim; i++) {
        index[i] = 0;
    }

    setBrushesCount();
}

void KisPipeBrushParasite::init()
{
    for (int i = 0; i < MaxDim; i++) {
        rank[i] = index[i] = brushesCount[i] = 0;
        selection[i] = KisParasite::Constant;
    }
}

void KisPipeBrushParasite::sanitize()
{
    for (int i = 0; i < dim; i++) {
        // In the 2 listed cases, we'd divide by 0!
        if (rank[i] == 0 &&
                (selection[i] == KisParasite::Incremental
                 || selection[i] == KisParasite::Angular)) {

            warnImage << "PIPE brush has a wrong rank for its selection mode!";
            selection[i] = KisParasite::Constant;
        }
    }
}

void KisPipeBrushParasite::setBrushesCount()
{
    // I assume ncells is correct. If it isn't, complain to the parasite header.
    if (rank[0] != 0) {
        brushesCount[0] = ncells / rank[0];
    }
    else {
        brushesCount[0] = ncells;
    }

    for (int i = 1; i < dim; i++) {
        if (rank[i] == 0) {
            brushesCount[i] = brushesCount[i - 1];
        }
        else {
            brushesCount[i] = brushesCount[i - 1] / rank[i];
        }
    }
}

bool KisPipeBrushParasite::saveToDevice(QIODevice* dev) const
{
    // write out something like
    // <count> ncells:<count> dim:<dim> rank0:<rank0> sel0:<sel0> <...>

    QTextStream stream(dev);
    stream.setCodec("UTF-8");

    // XXX: FIXME things like step, placement and so are not added (nor loaded, as a matter of fact)"
    stream << ncells << " ncells:" << ncells << " dim:" << dim;

    for (int i = 0; i < dim; i++) {
        stream << " rank" << i << ":" << rank[i] << " sel" << i << ":";
        switch (selection[i]) {
        case KisParasite::Constant:
            stream << "constant"; break;
        case KisParasite::Incremental:
            stream << "incremental"; break;
        case KisParasite::Angular:
            stream << "angular"; break;
        case KisParasite::Velocity:
            stream << "velocity"; break;
        case KisParasite::Random:
            stream << "random"; break;
        case KisParasite::Pressure:
            stream << "pressure"; break;
        case KisParasite::TiltX:
            stream << "xtilt"; break;
        case KisParasite::TiltY:
            stream << "ytilt"; break;
        }
    }

    return true;
}

bool loadFromDevice(QIODevice */*dev*/)
{
    // XXX: implement...
    return true;
}
