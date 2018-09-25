/*
*  TURBULENCE  
*  
*  Functions for defining Sub-grid scale (SGS) models for Large-Eddy Simulation.
*
*/

# include <stdlib.h>
#include <stdio.h>
#include <math.h>      /* sqrt()       */
#include "type.h"      /*the "real" type */
#include "def.h"       /* Definitions, parameters */
#include "global.h"    /* global variables */
#include "helpers.h"   /* Array3D*/
#include "turbulence.h"

real ***filter_( real ***U, real h[3] ){
/*
 *  Given the input in the form of a velocity component, produces filtered field, by the action of a linear filter, 
 *  either box filter, or gaussian filter or some other.
 *  Action of a linear filter on a 3D field is produced by three orthogonal passes trough field, using the 1D filter
 *  defined by array h[3] (the filter kernel), that is given as input parameter.
 *  More about that is given in Gaussian Blur Wikipedia article (https://en.wikipedia.org/wiki/Gaussian_blur):
 *  "A Gaussian blur effect is typically generated by convolving an image with a kernel of Gaussian values. In practice, 
 *   it is best to take advantage of the Gaussian blur’s separable property by dividing the process into two passes. In the 
 *   first pass, a one-dimensional kernel is used to blur the image in only the horizontal or vertical direction. In the 
 *   second pass, the same one-dimensional kernel is used to blur in the remaining direction. The resulting effect is 
 *   the same as convolving with a two-dimensional kernel in a single pass, but requires fewer calculations."
 *
 *   Use box filter:
 *   real boxFilterKer[3] = { 1./3., 1./3., 1./3. };
 *   real*** U_ = filter_( U2, boxFilterKer );
 *
 *   Use Gaussian filter:
 *   real gaussianFilterKer[3] = { 1./4., 2./4., 1./4. };
 *   real*** U_ = filter_( U2, gaussianFilterKer );
 *	
 */
    int i,j,k,l;

	real tmp;
    real ***U_, ***U__;

    U_  = Array3D( LEN+2, HIG+2, DEP+2 );
    U__ = Array3D( LEN+2, HIG+2, DEP+2 );

	for( i = 0; i < LENN; i++ ) {
		for( j = 1; j < HIGG; j++) { 
			for( k = 0; k < DEPP; k++ ) {

	            tmp = 0.;	
	            for( l = 0; l < 3; l++ ){

        			tmp += U[i][j+l-1][k]*h[l];
	            }

	            U__[i][j][k] = tmp;

			 } 
		} 
	} 

	for( i = 0; i < LENN; i++ ) {
		for( j = 0; j < HIGG; j++) { 
			for( k = 1; k < DEPP; k++ ) {

	            tmp = 0.;	
	            for( l = 0; l < 3; l++ ){

        			tmp += U__[i][j][k+l-1]*h[l];
	            }	

	            U_[i][j][k] = tmp;

			 } 
		} 
	}
	

	for( i = 1; i < LENN; i++ ) {
		for( j = 0; j < HIGG; j++) { 
			for( k = 0; k < DEPP; k++ ) {

	            tmp = 0.;	
	            for( l = 0; l < 3; l++ ){

            			tmp += U_[i+l-1][j][k]*h[l];

	            }	

	            U__[i][j][k] = tmp;

			 } 
		} 
	}

    free3D( U_ ,LEN+2, HIG+2 );

	return U__;
}


void DynamicSmagorinsky (real ***rho, real ***ru, real ***rv, real ***rw, real ***mu_SGS) {
/*
 * Definition of space varying coefficient Cd in Smagirinsky SGS model using
 * the dynamic procedure of Germano and using Lilly modification.
 * At the moment we expect CONSERVATIVE VARIABLES as input parameters.
 *
 * The original function used as a reference is written by Zubin Lal, Uni Wiscosin-Madison
 *
 */
    int i,j,k;
    real FilterKer[3] = { 1./3., 1./3., 1./3. }; // Box filter kernel.
    real _trace13;
    real Cd;
    real du_dx, du_dy, du_dz, dv_dx, dv_dy, dv_dz, dw_dx, dw_dy, dw_dz; 
    real S11,S12,S13,S21,S22,S23,S31,S32,S33;
    real S11_,S12_,S13_,S21_,S22_,S23_,S31_,S32_,S33_;  
    real S;
    real S_;
    real M11,M12,M13,M21,M22,M23,M31,M32,M33;
    real MMMM;
    real LLMM;
    real rr;

    real ***magStrain;

    // Primitive variables
    real ***u, ***v, ***w;

    // Filtered Velocity components 
    // _ 
    // Ui
    real ***u_, ***v_, ***w_;

    //  ____
    //  UiUj
    real ***uu_, ***uv_, ***uw_;
    real ***vu_, ***vv_, ***vw_;
    real ***wu_, ***wv_, ***ww_;

    // The Leonard stress tensor field components
    real ***L11, ***L12, ***L13;
    real ***L21, ***L22, ***L23;
    real ***L31, ***L32, ***L33;

    real ***A11, ***A12, ***A13;
    real ***A21, ***A22, ***A23;
    real ***A31, ***A32, ***A33;

    real ***A11_, ***A12_, ***A13_;
    real ***A21_, ***A22_, ***A23_;
    real ***A31_, ***A32_, ***A33_;

    real ***B11_, ***B12_, ***B13_;
    real ***B21_, ***B22_, ***B23_;
    real ***B31_, ***B32_, ***B33_;

    magStrain = Array3D( LEN+2, HIG+2, DEP+2 );

    u = Array3D( LEN+2, HIG+2, DEP+2 );
    v = Array3D( LEN+2, HIG+2, DEP+2 );
    w = Array3D( LEN+2, HIG+2, DEP+2 );

    A11 = Array3D( LEN+2, HIG+2, DEP+2 );
    A12 = Array3D( LEN+2, HIG+2, DEP+2 );
    A13 = Array3D( LEN+2, HIG+2, DEP+2 );
    A21 = Array3D( LEN+2, HIG+2, DEP+2 );
    A22 = Array3D( LEN+2, HIG+2, DEP+2 );
    A23 = Array3D( LEN+2, HIG+2, DEP+2 );
    A31 = Array3D( LEN+2, HIG+2, DEP+2 );
    A32 = Array3D( LEN+2, HIG+2, DEP+2 );
    A33 = Array3D( LEN+2, HIG+2, DEP+2 );    

    B11_ = Array3D( LEN+2, HIG+2, DEP+2 );
    B12_ = Array3D( LEN+2, HIG+2, DEP+2 );
    B13_ = Array3D( LEN+2, HIG+2, DEP+2 );
    B21_ = Array3D( LEN+2, HIG+2, DEP+2 );
    B22_ = Array3D( LEN+2, HIG+2, DEP+2 );
    B23_ = Array3D( LEN+2, HIG+2, DEP+2 );
    B31_ = Array3D( LEN+2, HIG+2, DEP+2 );
    B32_ = Array3D( LEN+2, HIG+2, DEP+2 );
    B33_ = Array3D( LEN+2, HIG+2, DEP+2 );

    L11 = Array3D( LEN+2, HIG+2, DEP+2 );
    L12 = Array3D( LEN+2, HIG+2, DEP+2 );
    L13 = Array3D( LEN+2, HIG+2, DEP+2 );
    L21 = Array3D( LEN+2, HIG+2, DEP+2 );
    L22 = Array3D( LEN+2, HIG+2, DEP+2 );
    L23 = Array3D( LEN+2, HIG+2, DEP+2 );
    L31 = Array3D( LEN+2, HIG+2, DEP+2 );
    L32 = Array3D( LEN+2, HIG+2, DEP+2 );
    L33 = Array3D( LEN+2, HIG+2, DEP+2 );


    // Change from given Conservative variables to Primitive variables
    for( i = 0; i < LENN+1; i++ ) {
        for( j = 0; j < HIGG+1; j++) { 
            for( k = 0; k < DEPP+1; k++ ) {

                rr = 1./rho[i][j][k];

                u[i][j][k] = ru[i][j][k]*rr;
                v[i][j][k] = rv[i][j][k]*rr;
                w[i][j][k] = rw[i][j][k]*rr;

            }
        }
    }

    // Unfiltered u_i*u_j complex 
    // NOTE: It is stored in allocated arrays of Leonard stress tensor components to save space!
	for( i = 0; i < LENN+1; i++ ) {
		for( j = 0; j < HIGG+1; j++) { 
			for( k = 0; k < DEPP+1; k++ ) {

                L11[i][j][k] = u[i][j][k]*u[i][j][k];
                L12[i][j][k] = u[i][j][k]*v[i][j][k];
                L13[i][j][k] = u[i][j][k]*w[i][j][k];

                L21[i][j][k] = v[i][j][k]*u[i][j][k];
                L22[i][j][k] = v[i][j][k]*v[i][j][k];
                L23[i][j][k] = v[i][j][k]*w[i][j][k];

                L31[i][j][k] = w[i][j][k]*u[i][j][k];
                L32[i][j][k] = w[i][j][k]*v[i][j][k];
                L33[i][j][k] = w[i][j][k]*w[i][j][k];
            }
        }
    }

    /* Filter cell center velocities */ 
    u_ = filter_(u, FilterKer);
    v_ = filter_(v, FilterKer);
    w_ = filter_(w, FilterKer);

    /* filter u_iu_j complex */ 
    uu_ = filter_(L11, FilterKer);
    uv_ = filter_(L12, FilterKer);
    uw_ = filter_(L13, FilterKer);

    vu_ = filter_(L21, FilterKer);
    vv_ = filter_(L22, FilterKer);
    vw_ = filter_(L23, FilterKer);

    wu_ = filter_(L31, FilterKer);
    wv_ = filter_(L32, FilterKer);
    ww_ = filter_(L33, FilterKer);

	for( i = 1; i < LENN; i++ ) {
		for( j = 1; j < HIGG; j++) { 
			for( k = 1; k < DEPP; k++ ) {

                // Leonard stresses: 
                // L_{ij} = \hat{u_{i} u_{j}} - \hat{u_{i}} \hat{u_{j}}
                // or  _____   _  _
                // L = Ui*Uj - Ui*Uj
             
                L11[i][j][k] = uu_[i][j][k] - u_[i][j][k]*u_[i][j][k];
                L12[i][j][k] = uv_[i][j][k] - u_[i][j][k]*v_[i][j][k];
                L13[i][j][k] = uw_[i][j][k] - u_[i][j][k]*w_[i][j][k];

                L21[i][j][k] = vu_[i][j][k] - v_[i][j][k]*u_[i][j][k];
                L22[i][j][k] = vv_[i][j][k] - v_[i][j][k]*v_[i][j][k];
                L23[i][j][k] = vw_[i][j][k] - v_[i][j][k]*w_[i][j][k];

                L31[i][j][k] = wu_[i][j][k] - w_[i][j][k]*u_[i][j][k];
                L32[i][j][k] = wv_[i][j][k] - w_[i][j][k]*v_[i][j][k];
                L33[i][j][k] = ww_[i][j][k] - w_[i][j][k]*w_[i][j][k];


                // Strain tensor: Sij
		        du_dx = ( u[i+1][j][k] - u[i-1][j][k] ) * _2deltaX;
		        du_dy = ( u[i][j+1][k] - u[i][j-1][k] ) * _2deltaY;
		        du_dz = ( u[i][j][k+1] - u[i][j][k-1] ) * _2deltaZ;

		        dv_dx = ( v[i+1][j][k] - v[i-1][j][k] ) * _2deltaX; 
		        dv_dy = ( v[i][j+1][k] - v[i][j-1][k] ) * _2deltaY;        
		        dv_dz = ( v[i][j][k+1] - v[i][j][k-1] ) * _2deltaZ;


		        dw_dx = ( w[i+1][j][k] - w[i-1][j][k] ) * _2deltaX;
		        dw_dy = ( w[i][j+1][k] - w[i][j-1][k] ) * _2deltaY;
		        dw_dz = ( w[i][j][k+1] - w[i][j][k-1] ) * _2deltaZ;

                S11 = du_dx;
                S12 = 0.5*(du_dy + dv_dx);
                S13 = 0.5*(du_dz + dw_dx);

                S21 = 0.5*(dv_dx + du_dy);
                S22 = dv_dy;
                S23 = 0.5*(dv_dz + dw_dy);

                S31 = 0.5*(dw_dx + du_dz);
                S32 = 0.5*(dw_dy + dv_dz);
                S33 = dw_dz;

                // |S| Strain tensor magnitude
                S = sqrt( 2*( S11*S11 + S12*S12+ S13*S13 
                            + S21*S21 + S22*S22 + S23*S23
                            + S31*S31 + S32*S32 + S33*S33 ) );

                /* more efficient:
                S12 = du_dy + dv_dx;
                S13 = du_dz + dw_dx;
                S23 = dv_dz + dw_dy;
                S = sqrt( 2. * ( du_dx * du_dx + dv_dy * dv_dy + dw_dz * dw_dz )
                              +  ( S12   * S12   + S13   * S13   + S23   * S23 ) );
                */

                magStrain[i][j][k] = S;

                //Filtered stress tensor components: 
                // __
                // Sij 
		        du_dx = ( u_[i+1][j][k] - u_[i-1][j][k] ) * _2deltaX;
		        du_dy = ( u_[i][j+1][k] - u_[i][j-1][k] ) * _2deltaY;
		        du_dz = ( u_[i][j][k+1] - u_[i][j][k-1] ) * _2deltaZ;

		        dv_dx = ( v_[i+1][j][k] - v_[i-1][j][k] ) * _2deltaX; 
		        dv_dy = ( v_[i][j+1][k] - v_[i][j-1][k] ) * _2deltaY;        
		        dv_dz = ( v_[i][j][k+1] - v_[i][j][k-1] ) * _2deltaZ;


		        dw_dx = ( w_[i+1][j][k] - w_[i-1][j][k] ) * _2deltaX;
		        dw_dy = ( w_[i][j+1][k] - w_[i][j-1][k] ) * _2deltaY;
		        dw_dz = ( w_[i][j][k+1] - w_[i][j][k-1] ) * _2deltaZ;

                S11_ = du_dx;
                S12_ = 0.5*(du_dy + dv_dx);
                S13_ = 0.5*(du_dz + dw_dx);

                S21_ = 0.5*(dv_dx + du_dy);
                S22_ = dv_dy;
                S23_ = 0.5*(dv_dz + dw_dy);

                S31_ = 0.5*(dw_dx + du_dz);
                S32_ = 0.5*(dw_dy + dv_dz);
                S33_ = dw_dz;

                // ___
                // |S|  Strain tensor magnitude using the test filtered velocities and its gradient
                S_ = sqrt( 2*( S11_*S11_ + S12_*S12_ + S13_*S13_
                             + S21_*S21_ + S22_*S22_ + S23_*S23_
                             + S31_*S31_ + S32_*S32_ + S33_*S33_ ) );
                /* more efficient:
                S12_ = du_dy + dv_dx;
                S13_ = du_dz + dw_dx;
                S23_ = dv_dz + dw_dy;
                S_ = sqrt( 2. * ( du_dx * du_dx + dv_dy * dv_dy + dw_dz * dw_dz )
                              +  ( S12   * S12   + S13   * S13   + S23   * S23 ) );
                */
                 
                // _     ___   __
                // Bij = |S| * Sij
                B11_[i][j][k] = S_ * S11_;
                B12_[i][j][k] = S_ * S12_;
                B13_[i][j][k] = S_ * S13_;

                B21_[i][j][k] = S_ * S21_;
                B22_[i][j][k] = S_ * S22_;
                B23_[i][j][k] = S_ * S23_;

                B31_[i][j][k] = S_ * S31_;
                B32_[i][j][k] = S_ * S32_;
                B33_[i][j][k] = S_ * S33_;

                // Aij  = |S|*Sij
                A11[i][j][k] = S * S11;
                A12[i][j][k] = S * S12;
                A13[i][j][k] = S * S13;

                A21[i][j][k] = S * S21;
                A22[i][j][k] = S * S22;
                A23[i][j][k] = S * S23;

                A31[i][j][k] = S * S31;
                A32[i][j][k] = S * S32;
                A33[i][j][k] = S * S33;
            }
        }
    }

    // __     _______           
    // Aij  = |S|*Sij
    A11_ = filter_(A11,FilterKer);
    A12_ = filter_(A12,FilterKer);
    A13_ = filter_(A13,FilterKer);

    A21_ = filter_(A21,FilterKer);
    A22_ = filter_(A22,FilterKer);
    A23_ = filter_(A23,FilterKer);

    A31_ = filter_(A31,FilterKer);
    A32_ = filter_(A32,FilterKer);
    A33_ = filter_(A33,FilterKer);

    for (i=1; i < LENN; i++) {
        for (j=1; j < HIGG; j++) {
            for (k=1; k < DEPP; k++) {

                // Mij = \hat{Delta}^2*Bij - Delta^2*\hat{Aij})
                // hat{Delta} = 2*Delta => pow(Delta_,2.0) = 4*pow(Delta,2.0)

                M11 = DD * ( 4*B11_[i][j][k] - A11_[i][j][k] );
                M12 = DD * ( 4*B12_[i][j][k] - A12_[i][j][k] );
                M13 = DD * ( 4*B13_[i][j][k] - A13_[i][j][k] );

                M21 = DD * ( 4*B21_[i][j][k] - A21_[i][j][k] );
                M22 = DD * ( 4*B22_[i][j][k] - A22_[i][j][k] );
                M23 = DD * ( 4*B23_[i][j][k] - A23_[i][j][k] );

                M31 = DD * ( 4*B31_[i][j][k] - A31_[i][j][k] );
                M32 = DD * ( 4*B32_[i][j][k] - A32_[i][j][k] );
                M33 = DD * ( 4*B33_[i][j][k] - A33_[i][j][k] );

                // Deviatoric part of L_{ij}^{d} = L_{ij} - 1/3*L_{kk}
				_trace13 =  -(1.0/3.0)*(L11[i][j][k] + L22[i][j][k] + L33[i][j][k]);
				L11[i][j][k] += _trace13;
				L22[i][j][k] += _trace13;
				L33[i][j][k] += _trace13;

                // L_{ij} : M_{ij}
                LLMM = (  L11[i][j][k]*M11 + L12[i][j][k]*M12 + L13[i][j][k]*M13
                        + L21[i][j][k]*M21 + L22[i][j][k]*M22 + L23[i][j][k]*M23
                        + L31[i][j][k]*M31 + L32[i][j][k]*M32 + L33[i][j][k]*M33 );

                // M_{ij} : M_{ij}
                MMMM = (  M11*M11 + M12*M12 + M13*M13
                        + M21*M21 + M22*M22 + M23*M23
                        + M31*M31 + M32*M32 + M33*M33);

                Cd = -0.5*(LLMM/(MMMM + small));

                // Clip values so that \mu_{SGS} \in [0,0.15]
                if ( Cd > 0.15 ){
                	Cd = 0.15;
                } else if( Cd < 0.0 ){
                	Cd = 0.0;
                } 

                mu_SGS[i][j][k] = rho[i][j][k]*Cd*DD*magStrain[i][j][k];
            }
        }
    }

    free3D( u,LEN+2, HIG+2  );
    free3D( v,LEN+2, HIG+2  );
    free3D( w,LEN+2, HIG+2  );

    free3D( u_,LEN+2, HIG+2  );
    free3D( v_,LEN+2, HIG+2  );
    free3D( w_,LEN+2, HIG+2  );

    free3D( uu_,LEN+2, HIG+2  );
    free3D( uv_,LEN+2, HIG+2  );
    free3D( uw_,LEN+2, HIG+2  );
    free3D( vu_,LEN+2, HIG+2  );
    free3D( vv_,LEN+2, HIG+2  );
    free3D( vw_,LEN+2, HIG+2  );
    free3D( wu_,LEN+2, HIG+2  );
    free3D( wv_,LEN+2, HIG+2 );
    free3D( ww_,LEN+2, HIG+2  );

    free3D( A33_,LEN+2, HIG+2  );
    free3D( A32_,LEN+2, HIG+2  );
    free3D( A31_,LEN+2, HIG+2  );
    free3D( A23_,LEN+2, HIG+2  );
    free3D( A22_,LEN+2, HIG+2  );
    free3D( A21_,LEN+2, HIG+2  );
    free3D( A11_,LEN+2, HIG+2  );
    free3D( A12_,LEN+2, HIG+2  );
    free3D( A13_,LEN+2, HIG+2  );

    free3D( A33,LEN+2, HIG+2  );
    free3D( A32,LEN+2, HIG+2  );
    free3D( A31,LEN+2, HIG+2  );
    free3D( A23,LEN+2, HIG+2  );
    free3D( A22,LEN+2, HIG+2  );
    free3D( A21,LEN+2, HIG+2  );
    free3D( A11,LEN+2, HIG+2  );
    free3D( A12,LEN+2, HIG+2  );
    free3D( A13,LEN+2, HIG+2  );

    free3D( L11,LEN+2, HIG+2  );
    free3D( L12,LEN+2, HIG+2  );
    free3D( L13,LEN+2, HIG+2  );
    free3D( L21,LEN+2, HIG+2  );
    free3D( L22,LEN+2, HIG+2  );
    free3D( L23,LEN+2, HIG+2  );
    free3D( L31,LEN+2, HIG+2  );
    free3D( L32,LEN+2, HIG+2  );
    free3D( L33,LEN+2, HIG+2  );

    free3D( B11_,LEN+2, HIG+2  );
    free3D( B12_,LEN+2, HIG+2  );
    free3D( B13_,LEN+2, HIG+2  );
    free3D( B21_,LEN+2, HIG+2  );
    free3D( B22_,LEN+2, HIG+2  );
    free3D( B23_,LEN+2, HIG+2  );
    free3D( B31_,LEN+2, HIG+2  );
    free3D( B32_,LEN+2, HIG+2  );
    free3D( B33_,LEN+2, HIG+2  );

    free3D( magStrain,LEN+2, HIG+2  );

}