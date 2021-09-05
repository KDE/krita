/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_ocio_display_filter_test.h"

#include <simpletest.h>
#include <stroke_testing_utils.h>
#include <testutil.h>

#include <QFile>

#include <KoChannelInfo.h>
#include <KoColorModelStandardIds.h>

#include <kis_exposure_gamma_correction_interface.h>
#include <kis_display_color_converter.h>
#include <kis_canvas_resource_provider.h>

#include <config-ocio.h>
#ifdef HAVE_OCIO_V2
#include <ocio_display_filter_vfx2021.h>
#else
#include <ocio_display_filter_vfx2020.h>
#endif

void KisOcioDisplayFilterTest::test()
{
    KisExposureGammaCorrectionInterface *egInterface =
            new KisDumbExposureGammaCorrectionInterface();

    QSharedPointer<OcioDisplayFilter> filter(new OcioDisplayFilter(egInterface));

    QString configFile = TestUtil::fetchDataFileLazy("./psyfiTestingConfig-master/config.ocio");
    dbgKrita << ppVar(configFile);

    Q_ASSERT(QFile::exists(configFile));

    OCIO::ConstConfigRcPtr ocioConfig =
            OCIO::Config::CreateFromFile(configFile.toUtf8());

    filter->config = ocioConfig;
    filter->inputColorSpaceName = ocioConfig->getColorSpaceNameByIndex(0);
    filter->displayDevice = ocioConfig->getDisplay(1);
    filter->view = ocioConfig->getView(filter->displayDevice, 0);
    filter->gamma = 1.0;
    filter->exposure = 0.0;
    filter->swizzle = RGBA;

    filter->blackPoint = 0.0;
    filter->whitePoint = 1.0;

    filter->forceInternalColorManagement = false;
    filter->setLockCurrentColorVisualRepresentation(false);

    filter->updateProcessor();

    dbgKrita << ppVar(filter->inputColorSpaceName);
    dbgKrita << ppVar(filter->displayDevice);
    dbgKrita << ppVar(filter->view);
    dbgKrita << ppVar(filter->gamma);
    dbgKrita << ppVar(filter->exposure);

    const KoColorSpace *paintingCS =
            KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), 0);

    KisImageSP image = utils::createImage(0, QSize(100, 100));
    image->convertImageColorSpace(paintingCS, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
    image->waitForDone();

    dbgKrita << ppVar(paintingCS) << ppVar(image->root()->firstChild()->colorSpace());

    KoCanvasResourceProvider *resourceManager =
            utils::createResourceManager(image,
                                         image->root(), "");

    KisDisplayColorConverter converter(resourceManager, 0);

    dbgKrita << ppVar(image->root()->firstChild());

    QVariant v;
    v.setValue(KisNodeWSP(image->root()->firstChild()));
    resourceManager->setResource(KoCanvasResource::CurrentKritaNode, v);

    converter.setDisplayFilter(filter);
    dbgKrita << ppVar(converter.paintingColorSpace());

    {
        QColor refColor(255, 128, 0);
        KoColor realColor = converter.approximateFromRenderedQColor(refColor);
        QColor roundTripColor = converter.toQColor(realColor);

        dbgKrita << ppVar(refColor);
        dbgKrita << ppVar(realColor.colorSpace()) << ppVar(KoColor::toQString(realColor));
        dbgKrita << ppVar(roundTripColor);
    }

    {
        KoColor realColor(Qt::red, paintingCS);
        QColor roundTripColor = converter.toQColor(realColor);

        dbgKrita << ppVar(realColor.colorSpace()) << ppVar(KoColor::toQString(realColor));
        dbgKrita << ppVar(roundTripColor);
    }

}

SIMPLE_TEST_MAIN(KisOcioDisplayFilterTest)
