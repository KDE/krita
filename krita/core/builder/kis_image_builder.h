/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef KIS_IMAGE_BUILDER_H_
#define KIS_IMAGE_BUILDER_H_

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
	KisImageBuilder_RESULT_PATH = 500
};

enum KisImageBuilder_Step {
	KisImageBuilder_STEP_PREP,
	KisImageBuilder_STEP_LOADING,
	KisImageBuilder_STEP_SAVING,
	KisImageBuilder_STEP_DECODING,
	KisImageBuilder_STEP_TILING,
	KisImageBuilder_STEP_DONE,
	KisImageBuilder_STEP_ERROR
};

#endif // KIS_IMAGE_BUILDER_H_

