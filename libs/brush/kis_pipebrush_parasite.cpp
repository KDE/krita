/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kis_pipebrush_parasite.h"

KisPipeBrushParasite::KisPipeBrushParasite(const QString& source)
{
    init();
    needsMovement = false;
    QRegExp basicSplitter(" ");
    QRegExp parasiteSplitter(":");
    QStringList parasites = source.split(basicSplitter, QString::SkipEmptyParts);
    for (int i = 0; i < parasites.count(); i++) {
        QStringList split = parasites.at(i).split(parasiteSplitter, QString::SkipEmptyParts);
        if (split.count() != 2) {
            warnImage << "Wrong count for this parasite key/value:" << parasites.at(i);
            continue;
        }
        QString index = split.at(0);
        if (index == "dim") {
            dim = (split.at(1)).toInt();
            if (dim < 1 || dim > MaxDim) {
                dim = 1;
            }
        } else if (index.startsWith(QString("sel"))) {
            int selIndex = index.mid(strlen("sel")).toInt();

            if (selIndex >= 0 && selIndex < dim) {
                selectionMode = split.at(1);

                if (selectionMode == "incremental") {
                    selection[selIndex] = KisParasite::Incremental;                    
                }
                else if (selectionMode == "angular") {
                    selection[selIndex] = KisParasite::Angular;
                    needsMovement = true;
                }
                else if (selectionMode == "random") {
                    selection[selIndex] = KisParasite::Random;
                }
                else if (selectionMode == "pressure") {
                    selection[selIndex] = KisParasite::Pressure;
                }
                else if (selectionMode == "xtilt") {
                    selection[selIndex] = KisParasite::TiltX;
                }
                else if (selectionMode == "ytilt") {
                    selection[selIndex] = KisParasite::TiltY;
                }
                else if (selectionMode == "velocity") {
                    selection[selIndex] = KisParasite::Velocity;
                }
                else {
                    selection[selIndex] = KisParasite::Constant;
                }
            }
            else {
                warnImage << "Sel: wrong index: " << selIndex << "(dim = " << dim << ")";
            }
        }
        else if (index.startsWith(QString("rank"))) {
            int rankIndex = index.mid(strlen("rank")).toInt();
            if (rankIndex < 0 || rankIndex > dim) {
                warnImage << "Rankindex out of range: " << rankIndex;
                continue;
            }
            rank[rankIndex] = (split.at(1)).toInt();
        }
        else if (index == "ncells") {
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
