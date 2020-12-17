/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "kis_legacy_importer.h"

/*
struct ExifToKMD {
    QString exivPrefix;
    QString namespaceUri;
    QString name;
};


static const ExifToKMD mappings[] = {
    { "Exif.Image.ImageWidth", "http://ns.adobe.com/tiff/1.0/", "ImageWidth" },
    { "Exif.Image.ImageLength", "http://ns.adobe.com/tiff/1.0/", "ImageLength" },
    { "Exif.Image.BitsPerSample", "http://ns.adobe.com/tiff/1.0/", "BitsPerSample" },
    { "Exif.Image.Compression", "http://ns.adobe.com/tiff/1.0/", "Compression" },
    { "Exif.Image.PhotometricInterpretation", "http://ns.adobe.com/tiff/1.0/", "PhotometricInterpretation" },
    { "Exif.Image.Orientation", "http://ns.adobe.com/tiff/1.0/", "Orientation" },
    { "Exif.Image.SamplesPerPixel", "http://ns.adobe.com/tiff/1.0/", "SamplesPerPixel" },
    { "Exif.Image.PlanarConfiguration", "http://ns.adobe.com/tiff/1.0/", "PlanarConfiguration" },
    { "Exif.Image.YCbCrSubSampling", "http://ns.adobe.com/tiff/1.0/", "YCbCrSubSampling" },
    { "Exif.Image.YCbCrPositioning", "http://ns.adobe.com/tiff/1.0/", "YCbCrPositioning" },
    { "Exif.Image.XResolution", "http://ns.adobe.com/tiff/1.0/", "XResolution" },
    { "Exif.Image.YResolution", "http://ns.adobe.com/tiff/1.0/", "YResolution" },
    { "Exif.Image.ResolutionUnit", "http://ns.adobe.com/tiff/1.0/", "ResolutionUnit" },
    { "Exif.Image.TransferFunction", "http://ns.adobe.com/tiff/1.0/", "TransferFunction" },
    { "Exif.Image.WhitePoint", "http://ns.adobe.com/tiff/1.0/", "WhitePoint" },
    { "Exif.Image.PrimaryChromaticities", "http://ns.adobe.com/tiff/1.0/", "PrimaryChromaticities" },
    { "Exif.Image.YCbCrCoefficients", "http://ns.adobe.com/tiff/1.0/", "YCbCrCoefficients" },
    { "Exif.Image.ReferenceBlackWhite", "http://ns.adobe.com/tiff/1.0/", "ReferenceBlackWhite" },
    { "Exif.Image.DateTime", "", "xmp:ModifyDate" },
    { "Exif.Image.ImageDescription", "", "dc:description" },
    { "Exif.Image.Make", "http://ns.adobe.com/tiff/1.0/", "Make" },
    { "Exif.Image.Model", "http://ns.adobe.com/tiff/1.0/", "Model" },
    { "Exif.Image.Software", "", "xmp:CreatorTool" },
    { "Exif.Image.Artist", "", "dc:creator" },
    { "Exif.Image.Copyright", "", "dc:rights" },
    { "Exif.Photo.ExifVersion", "http://ns.adobe.com/exif/1.0/", "ExifVersion" },
    { "Exif.Photo.FlashpixVersion", "http://ns.adobe.com/exif/1.0/", "FlashpixVersion" },
    { "Exif.Photo.ColorSpace", "http://ns.adobe.com/exif/1.0/", "ColorSpace" },
    { "Exif.Photo.ComponentsConfiguration", "http://ns.adobe.com/exif/1.0/", "ComponentsConfiguration" },
    { "Exif.Photo.CompressedBitsPerPixel", "http://ns.adobe.com/exif/1.0/", "CompressedBitsPerPixel" },
    { "Exif.Photo.PixelXDimension", "http://ns.adobe.com/exif/1.0/", "PixelXDimension" },
    { "Exif.Photo.PixelYDimension", "http://ns.adobe.com/exif/1.0/", "PixelYDimension" },
    { "Exif.Photo.UserComment", "http://ns.adobe.com/exif/1.0/", "UserComment" },
    { "Exif.Photo.RelatedSoundFile", "http://ns.adobe.com/exif/1.0/", "RelatedSoundFile" },
    { "Exif.Photo.DateTimeOriginal", "http://ns.adobe.com/exif/1.0/", "DateTimeOriginal" },
    { "Exif.Photo.DateTimeDigitized", "http://ns.adobe.com/exif/1.0/", "DateTimeDigitized" },
    { "Exif.Photo.ExposureTime", "http://ns.adobe.com/exif/1.0/", "ExposureTime" },
    { "Exif.Photo.FNumber", "http://ns.adobe.com/exif/1.0/", "FNumber" },
    { "Exif.Photo.ExposureProgram", "http://ns.adobe.com/exif/1.0/", "ExposureProgram" },
    { "Exif.Photo.SpectralSensitivity", "http://ns.adobe.com/exif/1.0/", "SpectralSensitivity" },
    { "Exif.Photo.ISOSpeedRatings", "http://ns.adobe.com/exif/1.0/", "ISOSpeedRatings" },
    { "Exif.Photo.OECF", "http://ns.adobe.com/exif/1.0/", "OECF" },
    { "Exif.Photo.ShutterSpeedValue", "http://ns.adobe.com/exif/1.0/", "ShutterSpeedValue" },
    { "Exif.Photo.ApertureValue", "http://ns.adobe.com/exif/1.0/", "ApertureValue" },
    { "Exif.Photo.BrightnessValue", "http://ns.adobe.com/exif/1.0/", "BrightnessValue" },
    { "Exif.Photo.ExposureBiasValue", "http://ns.adobe.com/exif/1.0/", "ExposureBiasValue" },
    { "Exif.Photo.MaxApertureValue", "http://ns.adobe.com/exif/1.0/", "MaxApertureValue" },
    { "Exif.Photo.", "http://ns.adobe.com/exif/1.0/", "" },
    { "Exif.Photo.", "http://ns.adobe.com/exif/1.0/", "" },
    { "Exif.Photo.", "http://ns.adobe.com/exif/1.0/", "" },
    { "Exif.Photo.", "http://ns.adobe.com/exif/1.0/", "" },
    { "Exif.Photo.", "http://ns.adobe.com/exif/1.0/", "" },
    { "Exif.Photo.", "http://ns.adobe.com/exif/1.0/", "" },
    { "Exif.Photo.", "http://ns.adobe.com/exif/1.0/", "" },
    { "Exif.Photo.", "http://ns.adobe.com/exif/1.0/", "" },
    { "Exif.Photo.", "http://ns.adobe.com/exif/1.0/", "" },
    { "Exif.Photo.", "http://ns.adobe.com/exif/1.0/", "" },
    { "Exif.Photo.", "http://ns.adobe.com/exif/1.0/", "" },
    { "Exif.Photo.", "http://ns.adobe.com/exif/1.0/", "" },
    { "Exif.Photo.", "http://ns.adobe.com/exif/1.0/", "" },
    { "Exif.Photo.", "http://ns.adobe.com/exif/1.0/", "" },
    { "Exif.Photo.", "http://ns.adobe.com/exif/1.0/", "" }
};*/
