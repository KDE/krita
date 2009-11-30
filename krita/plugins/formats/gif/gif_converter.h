/*
 * Copyright (c) 1990-1993 David Koblas <koblas@extra.com>
 * Copyright (c) 2003 Adam D. Moss <adam@gimp.org>
 *
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef _GIF_CONVERTER_H_
#define _GIF_CONVERTER_H_

#include <stdio.h>

#include <QObject>

#include <kio/job.h>

#include "kis_types.h"

class KisDoc2;
class KisUndoAdapter;

/**
 * Image import/export plugins can use these results to report about success or failure.
 */
enum KisImageBuilder_Result {
        KisImageBuilder_RESULT_FAILURE = -400,
        KisImageBuilder_RESULT_NOT_EXIST = -300,
        KisImageBuilder_RESULT_NOT_LOCAL = -200,
        KisImageBuilder_RESULT_BAD_FETCH = -100,
        KisImageBuilder_RESULT_INVALID_ARG = -50,
        KisImageBuilder_RESULT_OK = 0,
        KisImageBuilder_RESULT_PROGRESS = 1,
        KisImageBuilder_RESULT_EMPTY = 100,
        KisImageBuilder_RESULT_BUSY = 150,
        KisImageBuilder_RESULT_NO_URI = 200,
        KisImageBuilder_RESULT_UNSUPPORTED = 300,
        KisImageBuilder_RESULT_INTR = 400,
        KisImageBuilder_RESULT_PATH = 500,
        KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE = 600
};

/**
 * load and save gif files. This is based exclusively on Gimp's gif filter, which
 * seems to be able to handle animated gifs, which the qt-gif-plugin which is based
 * on libgif cannot handle (git://gitorious.org/qt-gif-plugin/qt-gif-plugin.git).
 *
 * And since neither libgif nor libungif have been maintained for years, we'd better
 * look at some example code that _is_ maintained and debugged.
 */
class gifConverter : public QObject {
        Q_OBJECT
    public:
        gifConverter(KisDoc2 *doc, KisUndoAdapter *adapter);
        virtual ~gifConverter();
    public:
        KisImageBuilder_Result buildImage(const KUrl& uri);
        KisImageBuilder_Result buildFile(const KUrl& uri, KisPaintLayerSP layer);
        /**
         * Retrieve the constructed image
         */
        KisImageWSP image();
    private:
        KisImageBuilder_Result decode(const KUrl& uri);
    public slots:
        virtual void cancel();
    private:
        KisImageWSP m_img;
        KisDoc2 *m_doc;
        KisUndoAdapter *m_adapter;
        bool m_stop;
        KIO::TransferJob *m_job;
};

#endif
