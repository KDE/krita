void compositeCopyCyan(Q_INT32 stride,
		       QUANTUM *dst, 
		       Q_INT32 dststride,
		       QUANTUM *src, 
		       Q_INT32 srcstride,
		       Q_INT32 rows, 
		       Q_INT32 cols, 
		       QUANTUM opacity = OPACITY_OPAQUE)
{
	compositeCopyChannel(PIXEL_CYAN, stride, dst, dststride, src, srcstride, rows, cols, opacity);
}


void compositeCopyMagenta(Q_INT32 stride,
			  QUANTUM *dst, 
			  Q_INT32 dststride,
			  QUANTUM *src, 
			  Q_INT32 srcstride,
			  Q_INT32 rows, 
			  Q_INT32 cols, 
			  QUANTUM opacity = OPACITY_OPAQUE)
{
	compositeCopyChannel(PIXEL_MAGENTA, stride, dst, dststride, src, srcstride, rows, cols, opacity);

}


void compositeCopyYellow(Q_INT32 stride,
			 QUANTUM *dst, 
			 Q_INT32 dststride,
			 QUANTUM *src, 
			 Q_INT32 srcstride,
			 Q_INT32 rows, 
			 Q_INT32 cols, 
			 QUANTUM opacity = OPACITY_OPAQUE)
{
	compositeCopyChannel(PIXEL_YELLOW, stride, dst, dststride, src, srcstride, rows, cols, opacity);

}


void compositeCopyBlack(Q_INT32 stride,
			QUANTUM *dst, 
			Q_INT32 dststride,
			QUANTUM *src, 
			Q_INT32 srcstride,
			Q_INT32 rows, 
			Q_INT32 cols, 
			QUANTUM opacity = OPACITY_OPAQUE)
{
	compositeCopyChannel(PIXEL_BLACK, stride, dst, dststride, src, srcstride, rows, cols, opacity);
}

