#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "lin_fit.h"
#define RP_BUF_SIZE 16384


/* fit/linear.c
 * 
 * Copyright (C) 2000, 2007 Brian Gough


 Hunter Akins 
 8/31/2017
 Note:
 * Modified this one a bit: 
   1. Changed double type to float
   2. Got rid of goodness of fit stuff
 
 * Goal was to remove gsl dependency and make more suited to my rmt application

 End of note.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


/* Fit the data (x_i, y_i) to the linear relationship 

   Y = c0 + c1 x

   returning, 

   c0, c1  --  coefficients
   cov00, cov01, cov11  --  variance-covariance matrix of c0 and c1,
   sumsq   --   sum of squares of residuals 

   This fit can be used in the case where the errors for the data are
   uknown, but assumed equal for all points. The resulting
   variance-covariance matrix estimates the error in the coefficients
   from the observed variance of the points around the best fit line.
*/

/* make a domain for the linear fit, simply the numbers 0, 1, ..., bufsize */
void MakeDomain(int bufsize, float *ip) {
	int i = 0;
	for (i = 0; i < bufsize; i++) {
		*(ip + i) = (float) i;
	}
}


int gsl_fit_linear (const float *x, const float *y, const size_t n, float *c0, float *c1) {
	float m_x = 0, m_y = 0, m_dx2 = 0, m_dxdy = 0;

	size_t i;

	for (i = 0; i < n; i++)
	{
		m_x += (x[i] - m_x) / (i + 1.0);
		m_y += (y[i] - m_y) / (i + 1.0);
	}

	for (i = 0; i < n; i++)
	{
		const float dx = x[i] - m_x;
		const float dy = y[i] - m_y;

		m_dx2 += (dx * dx - m_dx2) / (i + 1.0);
		m_dxdy += (dx * dy - m_dxdy) / (i + 1.0);
	}

	/* In terms of y = a + b x */

	
	float b = m_dxdy / m_dx2;
	float a = m_y - m_x * b;

	*c0 = a;
	*c1 = b;

	return 0;

}

int LinearFilter(float * domain, float * data, size_t n, float c0, float c1) {
	size_t i;
	MakeDomain(n, domain);
	gsl_fit_linear(domain, data, n, &c0, &c1);
	for (i = 0; i < n; i ++) {
		*(data + i) = *(data + i) - c0 - c1*i;
	}
	return 0;
}	

