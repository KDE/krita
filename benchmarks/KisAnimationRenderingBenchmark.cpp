/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAnimationRenderingBenchmark.h"

#include <QTest>

#include <testutil.h>
#include "kis_time_span.h"
#include "dialogs/KisAsyncAnimationFramesSaveDialog.h"
#include "kis_image_animation_interface.h"
#include "KisPart.h"
#include "KisDocument.h"
#include "kis_image.h"
#include "kis_image_config.h"

namespace {
void removeTempFiles(const QString &filesMask)
{
    QFileInfo info(filesMask);
    QDir dir(info.absolutePath());
    QStringList filesList = dir.entryList({ info.fileName() });
    if (!filesList.isEmpty()) {
        Q_FOREACH (const QString &file, filesList) {
            if (!dir.remove(file)) {
                QFAIL("Couldn't remove the old testing file!");
            }
        }
    }
}

void runRenderingTest(KisImageSP image, int numCores, int numClones)
{
    {
        KisImageConfig cfg(false);
        cfg.setMaxNumberOfThreads(numCores);
        cfg.setFrameRenderingClones(numClones);
    }

    const KisTimeSpan range = image->animationInterface()->fullClipRange();

    KisAsyncAnimationFramesSaveDialog dlg(image, range, "temp_frames.png", 0, false, 0);
    dlg.setBatchMode(true);

    // repeat rendering twice!
    for (int i = 0; i < 1; i++) {
        removeTempFiles(dlg.savedFilesMaskWildcard());
        KisAsyncAnimationFramesSaveDialog::Result result = dlg.regenerateRange(0);
        QCOMPARE(result, KisAsyncAnimationFramesSaveDialog::RenderComplete);
        removeTempFiles(dlg.savedFilesMaskWildcard());
    }
}


}




void KisAnimationRenderingBenchmark::testCacheRendering()
{
    const QString fileName = TestUtil::fetchDataFileLazy("miloor_turntable_002.kra", true);
    QVERIFY(QFileInfo(fileName).exists());


    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());

    bool loadingResult = doc->loadNativeFormat(fileName);
    QVERIFY(loadingResult);


    doc->image()->barrierLock();
    doc->image()->unlock();


    for (int numCores = 1; numCores <= QThread::idealThreadCount(); numCores++) {
        QElapsedTimer timer;
        timer.start();

        const int numClones = qMax(1, numCores / 2);
        runRenderingTest(doc->image(), numCores, numClones);

        qDebug() << "Cores:" << numCores << "Clones:" << numClones << "Time:" << timer.elapsed();
    }

    for (int numCores = 1; numCores <= QThread::idealThreadCount(); numCores++) {
        QElapsedTimer timer;
        timer.start();

        const int numClones = numCores;
        runRenderingTest(doc->image(), numCores, numClones);

        qDebug() << "Cores:" << numCores << "Clones:" << numClones << "Time:" << timer.elapsed();
    }
}

QTEST_MAIN(KisAnimationRenderingBenchmark)
