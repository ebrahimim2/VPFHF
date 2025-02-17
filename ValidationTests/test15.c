/*
 test15.c: Solves for the displacement and v-field in a pressurized penny crack in 2d (Sneddon 2D)
 (c) 2010-2012 Chukwudi CHukwudozie cchukw1@tigers.lsu.edu
 
 ./test15 -n 201,201,2 -l 4,4,0.01 -epsilon 0.05 -length 0.2 -orientation 1 -Gc 0.6667
 ./test15 -n 101,101,2 -l 4,4,0.01 -epsilon 0.05 -length 0.2 -orientation 1 -Gc 0.6667
 */

#include "petsc.h"
#include "VFCartFE.h"
#include "VFCommon.h"
#include "VFMech.h"
#include "VFFlow.h"

VFCtx               ctx;
VFFields            fields;

#undef __FUNCT__
#define __FUNCT__ "main"
int main(int argc,char **argv)
{
	VFFields            fields;
	PetscErrorCode      ierr;
	PetscReal           length = .425;
	PetscReal           center[3]={0.,0.,.5};
	PetscInt            orientation=2;
	PetscInt            nopts=3;
	PetscInt            i,j,k,nx,ny,nz,xs,xm,ys,ym,zs,zm;
	PetscReal           ****coords_array;
	PetscReal           ***v_array;  
	PetscReal           BBmin[3],BBmax[3];
	PetscReal           x,y,z;  
	PetscReal           ElasticEnergy = 0;
	PetscReal           InsituWork = 0;
	PetscReal           SurfaceEnergy = 0;
	char                filename[FILENAME_MAX];
	PetscReal            lx,ly,lz;
	PetscReal           p = 1e-2;
	PetscReal           errV=1e+10,errP;
	Vec                 Vold;
	PetscReal           p_epsilon = 1.e-4;
	PetscInt            altminit=1;

	
	ierr = PetscInitialize(&argc,&argv,(char*)0,banner);CHKERRQ(ierr);
	ierr = VFInitialize(&ctx,&fields);CHKERRQ(ierr);
	
	ierr = PetscOptionsGetReal(NULL,"-length",&length,NULL);CHKERRQ(ierr);
	ierr = PetscOptionsGetRealArray(NULL,"-center",&center[0],&nopts,NULL);CHKERRQ(ierr);
	
	ierr = PetscOptionsGetInt(NULL,"-orientation",&orientation,NULL);CHKERRQ(ierr);
	ierr = DMDAGetInfo(ctx.daScal,NULL,&nx,&ny,&nz,NULL,NULL,NULL,
					   NULL,NULL,NULL,NULL,NULL,NULL);CHKERRQ(ierr);
	ierr = DMDAGetCorners(ctx.daScal,&xs,&ys,&zs,&xm,&ym,&zm);CHKERRQ(ierr);
	ierr = DMDAGetBoundingBox(ctx.daScal,BBmin,BBmax);CHKERRQ(ierr);
	
	ierr = DMDAVecGetArrayDOF(ctx.daVect,ctx.coordinates,&coords_array);CHKERRQ(ierr);
	ierr = VecSet(fields.VIrrev,1.0);CHKERRQ(ierr);
	ierr = DMDAVecGetArray(ctx.daScal,fields.VIrrev,&v_array);CHKERRQ(ierr);    
	lz = BBmax[2];
	ly = BBmax[1];
	lx = BBmax[0];	
	/*
	 Reset all BC for U and V
	 */
	for (i = 0; i < 6; i++) {
		ctx.bcV[0].face[i]=NONE;
		for (j = 0; j < 3; j++) {
			ctx.bcU[j].face[i] = NONE;
		}
	}
	for (i = 0; i < 12; i++) {
		ctx.bcV[0].edge[i]=NONE;
		for (j = 0; j < 3; j++) {
			ctx.bcU[j].edge[i] = NONE;
		}
	}
	for (i = 0; i < 8; i++) {
		ctx.bcV[0].vertex[i]=NONE;
		for (j = 0; j < 3; j++) {
			ctx.bcU[j].vertex[i] = NONE;
		}
	}
	switch (orientation) {
		case 1:
			ierr = PetscPrintf(PETSC_COMM_WORLD,"Building a line crack of length %g at (%g,%g,%g) with normal vector <0,1,0>\n",
							   length,center[0],center[1],center[2]);CHKERRQ(ierr);		  
			/*	face X0	*/
			ctx.bcU[0].face[X0]= ZERO;
			ctx.bcU[1].face[X0]= ZERO;
			ctx.bcU[2].face[X0]= ZERO;
			/*	face X1	*/
			ctx.bcU[0].face[X1]= ZERO;
			ctx.bcU[1].face[X1]= ZERO;
			ctx.bcU[2].face[X1]= ZERO;
			/*	face Y0	*/
			ctx.bcU[0].face[Y0]= ZERO;
			ctx.bcU[1].face[Y0]= ZERO;
			ctx.bcU[2].face[Y0]= ZERO;
			/*	face Y1	*/
			ctx.bcU[0].face[Y1]= ZERO;		  
			ctx.bcU[1].face[Y1]= ZERO;		  
			ctx.bcU[2].face[Y1]= ZERO;		  
			/*	face Z0	*/
			ctx.bcU[2].face[Z0]= ZERO;
			/*	face Z1	*/
			ctx.bcU[2].face[Z1]= ZERO;
			/* BCV*/
			ctx.bcV[0].face[X0] = ONE;
			ctx.bcV[0].face[X1] = ONE;
			ctx.bcV[0].face[Y0] = ONE;
			ctx.bcV[0].face[Y1] = ONE;
			for (k = zs; k < zs+zm; k++) {
				for (j = ys; j < ys+ym; j++) {
					for (i = xs; i < xs+xm; i++) { 
						if ( ((j == ny/2) || (j == ny/2-1) ) && (coords_array[k][j][i][0] > lx/2.-length) && (coords_array[k][j][i][0] < lx/2.+length ) ) {
							v_array[k][j][i] = 0.;
						}
					}
				}
			}      
			break;
		case 2:
			ierr = PetscPrintf(PETSC_COMM_WORLD,"Building a line crack of length %g at (%g,%g,%g) with normal vector <0,1,0>\n",
							   length,center[0],center[1],center[2]);CHKERRQ(ierr);		  
			/*	face X0	*/
			ctx.bcU[0].face[X0]= ZERO;
			ctx.bcU[2].face[X0]= ZERO;
			/*	face X1	*/
			ctx.bcU[0].face[X1]= ZERO;
			ctx.bcU[2].face[X1]= ZERO;
			/*	face Y0	*/
			ctx.bcU[1].face[Y0]= ZERO;
			ctx.bcU[2].face[Y0]= ZERO;
			/*	face Y1	*/
			ctx.bcU[1].face[Y1]= ZERO;
			ctx.bcU[2].face[Y1]= ZERO;		  
			/*	face Z0	*/
			ctx.bcU[2].face[Z0]= ZERO;
			/*	face Z1	*/
			ctx.bcU[2].face[Z1]= ZERO;
			/* BCV*/
			ctx.bcV[0].face[X0] = ONE;
			ctx.bcV[0].face[X1] = ONE;
			ctx.bcV[0].face[Y0] = ONE;
			ctx.bcV[0].face[Y1] = ONE;
			for (k = zs; k < zs+zm; k++) {
				for (j = ys; j < ys+ym; j++) {
					for (i = xs; i < xs+xm; i++) { 
						if ( ((j == ny/2) || (j == ny/2-1)) && (coords_array[k][j][i][0] > lx/2.-length) && (coords_array[k][j][i][0] < lx/2.+length ) ) {
							v_array[k][j][i] = 0.;
						}
					}
				}
			}      
			break;
		case 3:
			ierr = PetscPrintf(PETSC_COMM_WORLD,"Building a line crack of length %g at (%g,%g,%g) with normal vector <0,1,0>\n",
							   length,center[0],center[1],center[2]);CHKERRQ(ierr);		  
			/*	face X0	*/
			ctx.bcU[0].face[X0]= ZERO;
			ctx.bcU[1].face[X0]= ZERO;
			ctx.bcU[2].face[X0]= ZERO;
			/*	face X1	*/
			ctx.bcU[0].face[X1]= ZERO;
			ctx.bcU[1].face[X1]= ZERO;
			ctx.bcU[2].face[X1]= ZERO;
			/*	face Y0	*/
			ctx.bcU[1].face[Y0]= ZERO;
			ctx.bcU[2].face[Y0]= ZERO;
			/*	face Y1	*/
			ctx.bcU[1].face[Y1]= ZERO;		  
			ctx.bcU[2].face[Y1]= ZERO;		  
			/*	face Z0	*/
			ctx.bcU[2].face[Z0]= ZERO;
			/*	face Z1	*/
			ctx.bcU[2].face[Z1]= ZERO;
			/* BCV*/
			ctx.bcV[0].face[X0] = ONE;
			ctx.bcV[0].face[X1] = ONE;
			ctx.bcV[0].face[Y0] = ONE;
			ctx.bcV[0].face[Y1] = ONE;
			for (k = zs; k < zs+zm; k++) {
				for (j = ys; j < ys+ym; j++) {
					for (i = xs; i < xs+xm; i++) { 
						if ( ((j == ny/2) || (j == ny/2-1)) && (coords_array[k][j][i][0] > lx/2.-length) && (coords_array[k][j][i][0] < lx/2.+length ) ) {
							v_array[k][j][i] = 0.;
						}
					}
				}
			}      
			break;
		case 4:
			ierr = PetscPrintf(PETSC_COMM_WORLD,"Building a line crack of length %g at (%g,%g,%g) with normal vector <0,1,0>\n",
							   length,center[0],center[1],center[2]);CHKERRQ(ierr);		  
			/*	face X0	*/
			ctx.bcU[0].face[X0]= ZERO;
			ctx.bcU[2].face[X0]= ZERO;
			/*	face X1	*/
			ctx.bcU[0].face[X1]= ZERO;
			ctx.bcU[2].face[X1]= ZERO;
			/*	face Y0	*/
			ctx.bcU[0].face[Y0]= ZERO;
			ctx.bcU[1].face[Y0]= ZERO;
			ctx.bcU[2].face[Y0]= ZERO;
			/*	face Y1	*/
			ctx.bcU[0].face[Y1]= ZERO;		  
			ctx.bcU[1].face[Y1]= ZERO;		  
			ctx.bcU[2].face[Y1]= ZERO;		  
			/*	face Z0	*/
			ctx.bcU[2].face[Z0]= ZERO;
			/*	face Z1	*/
			ctx.bcU[2].face[Z1]= ZERO;
			/* BCV*/
			ctx.bcV[0].face[X0] = ONE;
			ctx.bcV[0].face[X1] = ONE;
			ctx.bcV[0].face[Y0] = ONE;
			ctx.bcV[0].face[Y1] = ONE;
			for (k = zs; k < zs+zm; k++) {
				for (j = ys; j < ys+ym; j++) {
					for (i = xs; i < xs+xm; i++) { 
						if ( ((j == ny/2) || (j == ny/2-1)) && (coords_array[k][j][i][0] > lx/2.-length) && (coords_array[k][j][i][0] < lx/2.+length ) ) {
							v_array[k][j][i] = 0.;
						}
					}
				}
			}      
			break;
		case 5:
			ierr = PetscPrintf(PETSC_COMM_WORLD,"Building a line crack of length %g at (%g,%g,%g) with normal vector <0,1,0>\n",
							   length,center[0],center[1],center[2]);CHKERRQ(ierr);		  
			/*	face X0	*/
			ctx.bcU[0].face[X0]= ZERO;
			ctx.bcU[2].face[X0]= ZERO;
			/*	face X1	*/
			ctx.bcU[0].face[X1]= ZERO;
			ctx.bcU[2].face[X1]= ZERO;
			/*	face Y0	*/
			/*.....FREE.......*/
			/*	face Y1	*/
			/*.....FREE.......*/
			/*	face Z0	*/
			ctx.bcU[2].face[Z0]= ZERO;
			/*	face Z1	*/
			ctx.bcU[2].face[Z1]= ZERO;
			/* BCV*/
			ctx.bcV[0].face[X0] = ONE;
			ctx.bcV[0].face[X1] = ONE;
			ctx.bcV[0].face[Y0] = ONE;
			ctx.bcV[0].face[Y1] = ONE;
			for (k = zs; k < zs+zm; k++) {
				for (j = ys; j < ys+ym; j++) {
					for (i = xs; i < xs+xm; i++) { 
						if ( ((j == ny/2) || (j == ny/2-1)) && (coords_array[k][j][i][0] > lx/2.-length) && (coords_array[k][j][i][0] < lx/2.+length ) ) {
							v_array[k][j][i] = 0.;
						}
					}
				}
			}      
			break;
		case 6:
			ierr = PetscPrintf(PETSC_COMM_WORLD,"Building a line crack of length %g at (%g,%g,%g) with normal vector <0,1,0>\n",
							   length,center[0],center[1],center[2]);CHKERRQ(ierr);		  
			/*	face X0	*/
			ctx.bcU[0].face[X0]= ZERO;
			ctx.bcU[1].face[X0]= ZERO;
			ctx.bcU[2].face[X0]= ZERO;
			/*	face X1	*/
			ctx.bcU[0].face[X1]= ZERO;
			ctx.bcU[1].face[X1]= ZERO;
			ctx.bcU[2].face[X1]= ZERO;
			/*	face Y0	*/
			/*.....FREE.......*/
			/*	face Y1	*/
			/*.....FREE.......*/
			/*	face Z0	*/
			ctx.bcU[2].face[Z0]= ZERO;
			/*	face Z1	*/
			ctx.bcU[2].face[Z1]= ZERO;
			/* BCV*/
			ctx.bcV[0].face[X0] = ONE;
			ctx.bcV[0].face[X1] = ONE;
			ctx.bcV[0].face[Y0] = ONE;
			ctx.bcV[0].face[Y1] = ONE;
			for (k = zs; k < zs+zm; k++) {
				for (j = ys; j < ys+ym; j++) {
					for (i = xs; i < xs+xm; i++) { 
						if ( ((j == ny/2) || (j == ny/2-1)) && (coords_array[k][j][i][0] > lx/2.-length) && (coords_array[k][j][i][0] < lx/2.+length ) ) {
							v_array[k][j][i] = 0.;
						}
					}
				}
			}      
			break;
		default:
			SETERRQ1(PETSC_COMM_WORLD,PETSC_ERR_USER,"ERROR: Orientation should be one of {1,2,3,4,5,6}, got %i\n",orientation);
			break;
	} 
	ierr = DMDAVecRestoreArray(ctx.daScal,fields.VIrrev,&v_array);CHKERRQ(ierr);
	ierr = DMDAVecRestoreArrayDOF(ctx.daVect,ctx.coordinates,&coords_array);CHKERRQ(ierr);
	
	ierr = VecCopy(fields.VIrrev,fields.V);CHKERRQ(ierr);
	ierr = VFTimeStepPrepare(&ctx,&fields);CHKERRQ(ierr);
	ierr = VecSet(fields.theta,0.0);CHKERRQ(ierr);
	ierr = VecSet(fields.thetaRef,0.0);CHKERRQ(ierr);
	ierr = VecSet(fields.pressure,p);CHKERRQ(ierr);
		//ierr = BCUUpdate(&ctx.bcU[0],ctx.preset);CHKERRQ(ierr);
	ctx.matprop[0].beta = 0.;
	ctx.hasCrackPressure = PETSC_TRUE;	
	ierr = VecDuplicate(fields.V,&Vold);CHKERRQ(ierr);
	altminit = 0.;
	do {
		ierr = PetscPrintf(PETSC_COMM_WORLD,"  alt min step %i with errorV %g\n",altminit,errV);CHKERRQ(ierr);
		
		ierr = VecCopy(fields.V,Vold);CHKERRQ(ierr);
		ierr = VF_StepU(&fields,&ctx);
		ierr = VF_StepV(&fields,&ctx);
		ierr = VecAXPY(Vold,-1.,fields.V);CHKERRQ(ierr);
		ierr = VecNorm(Vold,NORM_INFINITY,&errV);CHKERRQ(ierr);
		altminit++;
	} while (errV >= ctx.altmintol && altminit <= ctx.altminmaxit);
	ctx.ElasticEnergy=0;
	ctx.InsituWork=0;
	ctx.PressureWork = 0.;
	ierr = VF_UEnergy3D(&ctx.ElasticEnergy,&ctx.InsituWork,&ctx.PressureWork,&fields,&ctx);CHKERRQ(ierr);
	ctx.TotalEnergy = ctx.ElasticEnergy - ctx.InsituWork - ctx.PressureWork;
	
	ierr = PetscPrintf(PETSC_COMM_WORLD,"Elastic Energy:            %e\n",ctx.ElasticEnergy);CHKERRQ(ierr);
	if (ctx.hasCrackPressure) {
		ierr = PetscPrintf(PETSC_COMM_WORLD,"Work of pressure forces:   %e\n",ctx.PressureWork);CHKERRQ(ierr);
	}
	if (ctx.hasInsitu) {
		ierr = PetscPrintf(PETSC_COMM_WORLD,"Work of surface forces:    %e\n",ctx.InsituWork);CHKERRQ(ierr);
	}
	ierr = PetscPrintf(PETSC_COMM_WORLD,"Total energy:              %e\n",ctx.ElasticEnergy-InsituWork-ctx.PressureWork);CHKERRQ(ierr);
	ierr = VolumetricCrackOpening(&ctx.CrackVolume, &ctx, &fields);CHKERRQ(ierr);   
	ierr = PetscPrintf(PETSC_COMM_WORLD,"\n\n Final Crack volume\t = %g, Pressure\t= %g\n\n", ctx.CrackVolume, p);CHKERRQ(ierr);
  PetscReal   functionvalue;
	ierr = TrialFunctionCompute(&functionvalue,&ctx,&fields);CHKERRQ(ierr);
  ierr = PetscPrintf(PETSC_COMM_WORLD,"\n\n\n\n FunctionValue\t = %g\n", functionvalue);CHKERRQ(ierr);

	/*
	 Save fields and write statistics about current run
	 */    
    switch (ctx.fileformat) {
		case FILEFORMAT_HDF5:       
			ierr = FieldsH5Write(&ctx,&fields);
			break;
		case FILEFORMAT_BIN:
			ierr = FieldsBinaryWrite(&ctx,&fields);
			break; 
    } 
	
	ierr = VFFinalize(&ctx,&fields);CHKERRQ(ierr);
	ierr = PetscFinalize();
	return(0);
}

