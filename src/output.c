/***********
*  OUTPUT  *   Outputs some specified flowfield parameter to file "input.dat"
***********/

#include <stdio.h>     /* printf() etc.*/
#include <math.h>      /* sqrt()       */
#include <stdlib.h>    /* atoi()       */

#include "type.h"
#include "def.h"      /* Definitions, parameters */
#include "global.h"   /* global variables */

#include "output.h"

void Output( void )
{

unsigned int i, j, k;
char str[80];
double xc, yc, zc;
double R, U, V, W, P, T;
double omega12, omega13, omega23, Omega;

/* matrix of velocity derivatives */
real 
dv_dx, dw_dx, 
du_dy, dw_dy,
du_dz, dv_dz;

FILE *pF;
// FILE *pF1;


// 	sprintf( str, "%d", step );
// 	if ( ( pF = fopen( str, "w" ) ) == NULL ) { printf( "Cannot open file"); exit(1); }
// 	fprintf( pF, "%d\n%d\n%f\n%f\n", LEN, HIG, deltaX, deltaY );

// 	sprintf( str, "Z-Vort-t%d", step );
// 	if ( ( pF1 = fopen( str, "w" ) ) == NULL ) { printf( "Cannot open file"); exit(1); }

// 	k = DEP/2;
// 	for ( j = 1; j < HIGG; j++ ) {
// 		for ( i = 1; i < LENN; i++ ) {
// 			dv_dx = ( U3[i+1][j][k]/U1[i+1][j][k] - U3[i-1][j][k]/U1[i-1][j][k] ) * _2deltaX;
// 			dw_dx = ( U4[i+1][j][k]/U1[i+1][j][k] - U4[i-1][j][k]/U1[i-1][j][k] ) * _2deltaX;
// 			du_dy = ( U2[i][j+1][k]/U1[i][j+1][k] - U2[i][j-1][k]/U1[i][j-1][k] ) * _2deltaY;
// 			dw_dy = ( U4[i][j+1][k]/U1[i][j+1][k] - U4[i][j-1][k]/U1[i][j-1][k] ) * _2deltaY;
// 			du_dz = ( U2[i][j][k+1]/U1[i][j][k+1] - U2[i][j][k-1]/U1[i][j][k-1] ) * _2deltaZ;
// 			dv_dz = ( U3[i][j][k+1]/U1[i][j][k+1] - U3[i][j][k-1]/U1[i][j][k-1] ) * _2deltaZ;
// 			U = 0.5 * ( dw_dy - dv_dz );
// 			V = 0.5 * ( du_dz - dw_dx );
// 			W = 0.5 * ( dv_dx - du_dy );
            
//       /* Upisi Vort. Mag. i u zasebnom fajlu Z-Vorticity*/
// 			fprintf( pF, "%f \n", sqrt( U * U + V * V + W * W ) );
//       fprintf( pF1, "%f \n", W );

// 		} /* end for */
// 	} /* end for */
// 	fclose( pF );
//   fclose( pF1 );

  /*-- Open and write results in tecplot file --*/
  sprintf( str, "%d.plt", step );
  if ((pF = fopen(str, "w")) == NULL) {
  fprintf(stderr, "cannot open file \"%s\"\n", "Tecplot-file");
  exit(-1);
  }

  fprintf(pF, "title     = \" 3-D compressible case \"\n");
  fprintf(pF, "variables = \" x \"\n");
  fprintf(pF, "\"y\"\n");
  fprintf(pF, "\"z\"\n");
  fprintf(pF, "\"rho\"\n");
  fprintf(pF, "\"u\"\n");
  fprintf(pF, "\"v\"\n");
  fprintf(pF, "\"w\"\n");
  fprintf(pF, "\"p\"\n");
  fprintf(pF, "\"T\"\n");
  fprintf(pF, "\"Vort. mag.\"\n");
  fprintf(pF, "zone t=\" \"\n");
  fprintf(pF, "i=%d, j=%d, k=%d, f=point\n", LEN, HIG, DEP);

  for (k = 1; k < DEPP; k++) {
    for (j = 1; j < HIGG; j++) {
      for (i = 1; i < LENN; i++) {

        xc = (i-1)*deltaX + 0.5*deltaX;
        yc = (j-1)*deltaY + 0.5*deltaY;
        zc = (k-1)*deltaZ + 0.5*deltaZ;

        R = U1[i][j][k];
        U = U2[i][j][k]/R;
        V = U3[i][j][k]/R;
        W = U4[i][j][k]/R;
        P = ( U5[i][j][k] - 0.5 * R * ( U * U + V * V + W * W ) ) * K_1;
        T = P / ( R_VOZD * R );

  
        // Vorticity magnitude
        dv_dx = ( U3[i+1][j][k]/U1[i+1][j][k] - U3[i-1][j][k]/U1[i-1][j][k] ) * _2deltaX;
        dw_dx = ( U4[i+1][j][k]/U1[i+1][j][k] - U4[i-1][j][k]/U1[i-1][j][k] ) * _2deltaX;
        du_dy = ( U2[i][j+1][k]/U1[i][j+1][k] - U2[i][j-1][k]/U1[i][j-1][k] ) * _2deltaY;
        dw_dy = ( U4[i][j+1][k]/U1[i][j+1][k] - U4[i][j-1][k]/U1[i][j-1][k] ) * _2deltaY;
        du_dz = ( U2[i][j][k+1]/U1[i][j][k+1] - U2[i][j][k-1]/U1[i][j][k-1] ) * _2deltaZ;
        dv_dz = ( U3[i][j][k+1]/U1[i][j][k+1] - U3[i][j][k-1]/U1[i][j][k-1] ) * _2deltaZ;
        omega23 = 0.5 * ( dw_dy - dv_dz );
        omega13 = 0.5 * ( du_dz - dw_dx );
        omega12 = 0.5 * ( dv_dx - du_dy );
        Omega = sqrt( omega12 * omega12 + omega13 * omega13 + omega23 * omega23 );

        fprintf(pF, "%g %g %g %g %g %g %g %g %g %g\n", xc, yc, zc, R, U, V, W, P, T, Omega);
        
      }
    }
  }
  fclose(pF);

} /* end Output() */
