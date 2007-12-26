/***************************************************************************
 *   Copyright (C) 2004 by Ewgenij Huebner                                  *
 *   huebner@uni-trier.de                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

int
pdip_initial_point_feasible_x(const gsl_matrix * A, const gsl_vector *b, gsl_vector *x);

int
pdip_initial_point_feasible_s(const gsl_matrix * C, const gsl_vector *d, const gsl_vector *x, gsl_vector *s);

int
pdip_initial_point_y(const gsl_matrix *Q, const gsl_vector *q, const gsl_matrix *A, const gsl_vector *x, gsl_vector *y);

int
pdip_initial_point_z(gsl_vector *z);

int
pdip_initial_point_strict_feasible(gsl_vector *x, gsl_vector *s);
