/* WMF Metafile Function Description Table
 * Author: Stefan Taferner <taferner@kde.org>
 */
#ifndef metafunc_h
#define metafunc_h

class QWinMetaFile;

static const struct MetaFuncRec
{
    const char* name;
    unsigned short func;
    void ( QWinMetaFile::*method )( long, short* );
} metaFuncTab[] =
  {
      { "SETBKCOLOR",           0x0201, &QWinMetaFile::setBkColor },
      { "SETBKMODE",            0x0102, &QWinMetaFile::setBkMode },
      { "SETMAPMODE",           0x0103, &QWinMetaFile::noop },
      { "SETROP2",              0x0104, &QWinMetaFile::setRop },
      { "SETRELABS",            0x0105, &QWinMetaFile::noop },
      { "SETPOLYFILLMODE",      0x0106, &QWinMetaFile::setPolyFillMode },
      { "SETSTRETCHBLTMODE",    0x0107, &QWinMetaFile::noop },
      { "SETTEXTCHAREXTRA",     0x0108, &QWinMetaFile::noop },
      { "SETTEXTCOLOR",         0x0209, &QWinMetaFile::setTextColor },
      { "SETTEXTJUSTIFICATION", 0x020A, &QWinMetaFile::noop },
      { "SETWINDOWORG",         0x020B, &QWinMetaFile::setWindowOrg },
      { "SETWINDOWEXT",         0x020C, &QWinMetaFile::setWindowExt },
      { "SETVIEWPORTORG",       0x020D, &QWinMetaFile::noop },
      { "SETVIEWPORTEXT",       0x020E, &QWinMetaFile::noop },
      { "OFFSETWINDOWORG",      0x020F, &QWinMetaFile::noop },
      { "SCALEWINDOWEXT",       0x0410, &QWinMetaFile::noop },
      { "OFFSETVIEWPORTORG",    0x0211, &QWinMetaFile::noop },
      { "SCALEVIEWPORTEXT",     0x0412, &QWinMetaFile::noop },
      { "LINETO",               0x0213, &QWinMetaFile::lineTo },
      { "MOVETO",               0x0214, &QWinMetaFile::moveTo },
      { "EXCLUDECLIPRECT",      0x0415, &QWinMetaFile::excludeClipRect },
      { "INTERSECTCLIPRECT",    0x0416, &QWinMetaFile::intersectClipRect },
      { "ARC",                  0x0817, &QWinMetaFile::arc },
      { "ELLIPSE",              0x0418, &QWinMetaFile::ellipse },
      { "FLOODFILL",            0x0419, &QWinMetaFile::noop },
      { "PIE",                  0x081A, &QWinMetaFile::pie },
      { "RECTANGLE",            0x041B, &QWinMetaFile::rectangle },
      { "ROUNDRECT",            0x061C, &QWinMetaFile::roundRect },
      { "PATBLT",               0x061D, &QWinMetaFile::noop },
      { "SAVEDC",               0x001E, &QWinMetaFile::saveDC },
      { "SETPIXEL",             0x041F, &QWinMetaFile::setPixel },
      { "OFFSETCLIPRGN",        0x0220, &QWinMetaFile::noop },
      { "TEXTOUT",              0x0521, &QWinMetaFile::textOut },
      { "BITBLT",               0x0922, &QWinMetaFile::noop },
      { "STRETCHBLT",           0x0B23, &QWinMetaFile::noop },
      { "POLYGON",              0x0324, &QWinMetaFile::polygon },
      { "POLYLINE",             0x0325, &QWinMetaFile::polyline },
      { "ESCAPE",               0x0626, &QWinMetaFile::noop },
      { "RESTOREDC",            0x0127, &QWinMetaFile::restoreDC },
      { "FILLREGION",           0x0228, &QWinMetaFile::noop },
      { "FRAMEREGION",          0x0429, &QWinMetaFile::noop },
      { "INVERTREGION",         0x012A, &QWinMetaFile::noop },
      { "PAINTREGION",          0x012B, &QWinMetaFile::noop },
      { "SELECTCLIPREGION",     0x012C, &QWinMetaFile::noop },
      { "SELECTOBJECT",         0x012D, &QWinMetaFile::selectObject },
      { "SETTEXTALIGN",         0x012E, &QWinMetaFile::setTextAlign },
      { "CHORD",                0x0830, &QWinMetaFile::chord },
      { "SETMAPPERFLAGS",       0x0231, &QWinMetaFile::noop },
      { "EXTTEXTOUT",           0x0a32, &QWinMetaFile::extTextOut },
      { "SETDIBTODEV",          0x0d33, &QWinMetaFile::noop },
      { "SELECTPALETTE",        0x0234, &QWinMetaFile::noop },
      { "REALIZEPALETTE",       0x0035, &QWinMetaFile::noop },
      { "ANIMATEPALETTE",       0x0436, &QWinMetaFile::noop },
      { "SETPALENTRIES",        0x0037, &QWinMetaFile::noop },
      { "POLYPOLYGON",          0x0538, &QWinMetaFile::polyPolygon },
      { "RESIZEPALETTE",        0x0139, &QWinMetaFile::noop },
      { "DIBBITBLT",            0x0940, &QWinMetaFile::dibBitBlt },
      { "DIBSTRETCHBLT",        0x0b41, &QWinMetaFile::dibStretchBlt },
      { "DIBCREATEPATTERNBRUSH", 0x0142, &QWinMetaFile::dibCreatePatternBrush },
      { "STRETCHDIB",           0x0f43, &QWinMetaFile::stretchDib },
      { "EXTFLOODFILL",         0x0548, &QWinMetaFile::noop },
      { "DELETEOBJECT",         0x01f0, &QWinMetaFile::deleteObject },
      { "CREATEPALETTE",        0x00f7, &QWinMetaFile::createEmptyObject },
      { "CREATEPATTERNBRUSH",   0x01F9, &QWinMetaFile::createEmptyObject },
      { "CREATEPENINDIRECT",    0x02FA, &QWinMetaFile::createPenIndirect },
      { "CREATEFONTINDIRECT",   0x02FB, &QWinMetaFile::createFontIndirect },
      { "CREATEBRUSHINDIRECT",  0x02FC, &QWinMetaFile::createBrushIndirect },
      { "CREATEREGION",         0x06FF, &QWinMetaFile::createEmptyObject },
      { "END",                   0,      &QWinMetaFile::end },
      // always the latest in the table : in case of unknown function
      { NULL,                   0,      &QWinMetaFile::noop },
  };


#endif /*metafunc_h*/
