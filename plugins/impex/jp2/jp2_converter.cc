/*
 *  Copyright (c) 2019 Aaron Boxer <boxerab@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "jp2_converter.h"

#include <openjpeg.h>

#include <QFileInfo>
#include <QApplication>

#include <QMessageBox>

#include <QFileInfo>

#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceTraits.h>
#include <KoColorSpaceConstants.h>
#include <KisImportExportManager.h>
#include <KoColorSpace.h>
#include <KoColorModelStandardIds.h>

#include <KisDocument.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>
#include <kis_transaction.h>
#include "kis_iterator_ng.h"
#include <QThread>

#include <iostream>
#include <sstream>

#define J2K_CFMT 0
#define JP2_CFMT 1

JP2Converter::JP2Converter(KisDocument *doc) {
	m_doc = doc;
	m_stop = false;
}

JP2Converter::~JP2Converter() {
}

/**
 * sample error callback expecting a FILE* client object
 * */
static void error_callback(const char *msg, void *client_data) {
	JP2Converter *converter = (JP2Converter*) client_data;
	converter->addErrorString(msg);
}

/**
 * sample warning callback expecting a FILE* client object
 * */
static void warning_callback(const char *msg, void *client_data) {
	JP2Converter *converter = (JP2Converter*) client_data;
	converter->addWarningString(msg);
}

/**
 * sample debug callback expecting no client object
 * */
static void info_callback(const char *msg, void *client_data) {
	JP2Converter *converter = (JP2Converter*) client_data;
	converter->addInfoString(msg);
}

static int getFileFormat(const char *filename) {
	unsigned int i;
	static const char *extension[] = { "j2k", "jp2", "j2c", "jpc" };
	static const int format[] = { J2K_CFMT, JP2_CFMT,  J2K_CFMT, J2K_CFMT };
	const char *ext = strrchr(filename, '.');
	if (ext == NULL) {
		return -1;
	}
	ext++;
	if (*ext) {
		for (i = 0; i < sizeof(format) / sizeof(*format); i++) {
			if (strcasecmp(ext, extension[i]) == 0) {
				return format[i];
			}
		}
	}

	return -1;
}

#define JP2_RFC3745_MAGIC 	 "\x00\x00\x00\x0c\x6a\x50\x20\x20\x0d\x0a\x87\x0a"
#define JP2_MAGIC 			 "\x0d\x0a\x87\x0a"
#define J2K_CODESTREAM_MAGIC "\xff\x4f\xff\x51"

int JP2Converter::infile_format(const char *fname) {
	FILE *reader;
	const char *s, *magic_s;
	int ext_format, magic_format;
	unsigned char buf[12];
	OPJ_SIZE_T l_nb_read;

	reader = fopen(fname, "rb");

	if (reader == NULL) {
		return -2;
	}

	memset(buf, 0, 12);
	l_nb_read = fread(buf, 1, 12, reader);
	fclose(reader);
	if (l_nb_read != 12) {
		return -1;
	}
	ext_format = getFileFormat(fname);
	if (memcmp(buf, JP2_RFC3745_MAGIC, 12) == 0
			|| memcmp(buf, JP2_MAGIC, 4) == 0) {
		magic_format = JP2_CFMT;
		magic_s = ".jp2";
	} else if (memcmp(buf, J2K_CODESTREAM_MAGIC, 4) == 0) {
		magic_format = J2K_CFMT;
		magic_s = ".j2k or .jpc or .j2c";
	} else {
		return -1;
	}

	if (magic_format == ext_format) {
		return ext_format;
	}

	if (strlen(fname) >= 4) {
		s = fname + strlen(fname) - 4;
		std::ostringstream buffer;
		buffer << "The extension of this file is incorrect.\n"
			<< "Found " << s << " while it should be " << magic_s << ".";
		addErrorString(buffer.str());
	}
	return magic_format;
}

KisImportExportErrorCode JP2Converter::buildImage(const QString &filename) {
	KisImportExportErrorCode res = ImportExportCodes::OK;
	const char *file_str = filename.toUtf8().data();
	opj_codec_t *l_codec = 0;
	opj_dparameters_t parameters;
	bool hasColorSpaceInfo = false;
	opj_stream_t *l_stream = NULL;
	opj_image_t *image = NULL;
	int pos = 0;
	KisHLineIteratorSP it = NULL;
	unsigned int numComponents = 0;
	unsigned int precision = 0;
	const KoColorSpace *colorSpace = 0;
	QVector<int> channelorder;
	KisPaintLayerSP layer;
	bool isSigned;
	int32_t signedCorrection = 0;
	uint32_t w=0, h=0;

	// decompression parameters
	opj_set_default_decoder_parameters(&parameters);
	// Determine the type
	parameters.decod_format = infile_format(file_str);
	if (parameters.decod_format == -1) {
		addErrorString("Not a JPEG 2000 file.");
		res = ImportExportCodes::FileFormatIncorrect;
		goto beach;
	}

	// Decode the file
	/* get a decoder handle */
	switch (parameters.decod_format) {
	case J2K_CFMT: {
		l_codec = opj_create_decompress(OPJ_CODEC_J2K);
		break;
	}
	case JP2_CFMT: {
		l_codec = opj_create_decompress(OPJ_CODEC_JP2);
		hasColorSpaceInfo = true;
		break;
	}
	}
	Q_ASSERT(l_codec);

	opj_codec_set_threads( l_codec,QThread::idealThreadCount() );

	/* setup the decoder decoding parameters using user parameters */
	opj_setup_decoder(l_codec, &parameters);

	l_stream = opj_stream_create_default_file_stream(file_str, 1);
	if (!l_stream) {
		addErrorString("Failed to create the stream");
		res = ImportExportCodes::ErrorWhileReading;
		goto beach;
	}

	// Setup an event handling
	opj_set_info_handler(l_codec, info_callback, this);
	opj_set_error_handler(l_codec, error_callback, this);
	opj_set_warning_handler(l_codec, warning_callback, this);

	if (!opj_read_header(l_stream, l_codec, &image)) {
		addErrorString("Failed to read the header");
		res = ImportExportCodes::ErrorWhileReading;
		goto beach;
	}

	/* Get the decoded image */
	if (!(opj_decode(l_codec, l_stream, image)
			&& opj_end_decompress(l_codec, l_stream))) {
		addErrorString("Failed to decode image");
		res = ImportExportCodes::ErrorWhileReading;
		goto beach;
	}

	// Look for the colorspace
	numComponents = image->numcomps;
	if (image->numcomps == 0) {
		addErrorString("Image must have at least one component");
		res = ImportExportCodes::Failure;
		goto beach;
	}
	precision = image->comps[0].prec;
	for (uint32_t i = 1; i < numComponents; ++i) {
		if (image->comps[i].prec != precision) {
			std::ostringstream buffer;
			buffer << "All components must have the same bit depth "
					<< precision;
			addErrorString(buffer.str());
			res = ImportExportCodes::FormatFeaturesUnsupported;
			goto beach;
		}
	}
	isSigned = false;
	for (uint32_t i = 0; i < numComponents; ++i) {
		if ((image->comps[i].dx != 1) || (image->comps[i].dy != 1)) {
			addErrorString("Sub-sampling not supported");
			res = ImportExportCodes::FormatFeaturesUnsupported;
			goto beach;
		}
		isSigned = isSigned || (image->comps[0].sgnd);
	}
	if (isSigned)
		signedCorrection = 1 << (precision - 1);

	dbgFile
	<< "Image has " << numComponents << " numComponents and a bit depth of "
			<< precision << " for color space " << image->color_space;
	channelorder = QVector<int>(numComponents);
	if (!hasColorSpaceInfo) {
		if (numComponents == 3) {
			image->color_space = OPJ_CLRSPC_SRGB;
		} else if (numComponents == 1) {
			image->color_space = OPJ_CLRSPC_GRAY;
		}
	}
	switch (image->color_space) {
	case OPJ_CLRSPC_UNKNOWN:
	case OPJ_CLRSPC_UNSPECIFIED:
		break;
	case OPJ_CLRSPC_SRGB: {
		if (precision == 16 || precision == 12) {
			colorSpace = KoColorSpaceRegistry::instance()->rgb16();
		} else if (precision == 8) {
			colorSpace = KoColorSpaceRegistry::instance()->rgb8();
		}
		if (numComponents != 3) {
			std::ostringstream buffer;
			buffer << "sRGB: number of numComponents " << numComponents
					<< " does not equal 3";
			addErrorString(buffer.str());
			res = ImportExportCodes::FormatFeaturesUnsupported;
			goto beach;
		}
		channelorder[0] = KoBgrU16Traits::red_pos;
		channelorder[1] = KoBgrU16Traits::green_pos;
		channelorder[2] = KoBgrU16Traits::blue_pos;
		break;
	}
	case OPJ_CLRSPC_GRAY: {
		if (precision == 16 || precision == 12) {
			colorSpace = KoColorSpaceRegistry::instance()->colorSpace(
					GrayAColorModelID.id(), Integer16BitsColorDepthID.id(), "");
		} else if (precision == 8) {
			colorSpace = KoColorSpaceRegistry::instance()->colorSpace(
					GrayAColorModelID.id(), Integer8BitsColorDepthID.id(), "");
		}
		if (numComponents != 1) {
			std::ostringstream buffer;
			buffer << "Grayscale: number of numComponents " << numComponents
					<< " greater than 1";
			addErrorString(buffer.str());
			res = ImportExportCodes::FormatFeaturesUnsupported;
			goto beach;
		}
		channelorder[0] = 0;
		break;
	}
	case OPJ_CLRSPC_SYCC:
		addErrorString("YUV color space not supported");
		res = ImportExportCodes::FormatColorSpaceUnsupported;
		goto beach;
		break;
	case OPJ_CLRSPC_EYCC:
		addErrorString("eYCC color space not supported");
		res = ImportExportCodes::FormatColorSpaceUnsupported;
		goto beach;
		break;
	case OPJ_CLRSPC_CMYK:
		addErrorString("CMYK color space not supported");
		res = ImportExportCodes::FormatColorSpaceUnsupported;
		goto beach;
		break;
	default:
		break;
	}

	if (!colorSpace) {
		addErrorString("No color space found for image");
		res = ImportExportCodes::FormatColorSpaceUnsupported;
		goto beach;
	}

	// Create the image
	w = (uint32_t)(image->x1 - image->x0);
	h = (uint32_t)(image->y1 - image->y0);
	if (m_image == 0) {
		m_image = new KisImage(m_doc->createUndoStore(), w, h,
				colorSpace, "built image");
	}

	// Create the layer
	layer = new KisPaintLayer(m_image, m_image->nextLayerName(),
			OPACITY_OPAQUE_U8);
	m_image->addNode(layer);

	// Set the data
	it = layer->paintDevice()->createHLineIteratorNG(0, 0, w);
	for (OPJ_UINT32 v = 0; v < image->y1; ++v) {
		if (precision == 16 || precision == 12) {
			do {
				quint16 *px = reinterpret_cast<quint16*>(it->rawData());
				for (uint32_t i = 0; i < numComponents; ++i) {
					px[channelorder[i]] = image->comps[i].data[pos]
							+ signedCorrection;
				}
				colorSpace->setOpacity(it->rawData(), OPACITY_OPAQUE_U8, 1);
				++pos;

			} while (it->nextPixel());
		} else if (precision == 8) {
			do {
				quint8 *px = it->rawData();
				for (uint32_t i = 0; i < numComponents; ++i) {
					px[channelorder[i]] = image->comps[i].data[pos]
							+ signedCorrection;
				}
				colorSpace->setOpacity(px, OPACITY_OPAQUE_U8, 1);
				++pos;

			} while (it->nextPixel());
		}
		it->nextRow();
	}

beach:
	if (l_stream)
		opj_stream_destroy(l_stream);
	if (l_codec)
		opj_destroy_codec(l_codec);
	if (image)
		opj_image_destroy(image);
	if (!err.empty())
		m_doc->setErrorMessage(i18n(err.c_str()));
	if (!warn.empty())
		m_doc->setWarningMessage(i18n(warn.c_str()));
	return res;
}

KisImageWSP JP2Converter::image() {
	return m_image;
}

KisImportExportErrorCode JP2Converter::buildFile(const QString &filename,
		KisPaintLayerSP layer, const JP2ConvertOptions &options) {
	(void) layer;
	(void) filename;
	(void) options;
	return ImportExportCodes::Failure;
}

void JP2Converter::cancel() {
	m_stop = true;
}

void JP2Converter::addWarningString(const std::string &str) {
	if (!warn.empty())
		warn += "\n";
	warn += str;
}
void JP2Converter::addInfoString(const std::string &str) {
	dbgFile
	<< str.c_str();
}

void JP2Converter::addErrorString(const std::string &str) {
	if (!err.empty())
		err += "\n";
	err += str;
}

