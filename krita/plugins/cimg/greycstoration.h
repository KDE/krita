/* GREYCstoration Gimp plugin
 * Copyright (C) 2005 Victor Stinner and David Tschumperlé
 *
 * This plug-in is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef GREYCSTORATION_CLASS_H
#define GREYCSTORATION_CLASS_H
//----------------------------------------------------------------------------
#include "CImg.h"
#include "image.h"
//----------------------------------------------------------------------------

class GREYCstoration
{
public:
	// parameters
	unsigned int nb_iter; // Number of smoothing iterations
	float dt;       // Time step
	float dlength; // Integration step
	float dtheta; // Angular step (in degrees)
	float sigma;  // Structure tensor blurring
	float power1; // Diffusion limiter along isophote
	float power2; // Diffusion limiter along gradient
	float gauss_prec; //  Precision of the gaussian function
	bool onormalize; // Output image normalization (in [0,255])
	bool linear; // Use linear interpolation for integration

private:
	// internal use
	bool restore;
	bool inpaint;
	bool resize;
	const char* visuflow;
	cimg_library::CImg<> dest, sum, W;
	cimg_library::CImg<> img, img0, flow,G;
	cimg_library::CImgl<> eigen;
	cimg_library::CImg<unsigned char> mask;

public:
	GREYCstoration();
	void load_picture(Image& image);
	void store_picture(Image& image);
	bool process();

private:
	// Compute smoothed structure tensor field G
	void compute_smoothed_tensor();
	
	// Compute normalized tensor field sqrt(T) in G
	void compute_normalized_tensor();
	
	// Compute LIC's along different angle projections a_\alpha
	void compute_LIC(int &counter);
	void compute_LIC_back_forward(int x, int y);
	void compute_W(float cost, float sint);
	
	// Average all the LIC's
	void compute_average_LIC();

	// Prepare datas
	bool prepare();
	bool prepare_restore();
	bool prepare_inpaint();
	bool prepare_resize();
	bool prepare_visuflow();

	// Check arguments
	bool check_args();

	// Clean up memory (CImg datas) to save memory
	void cleanup();
};

extern GREYCstoration greyc;
//----------------------------------------------------------------------------
#endif
