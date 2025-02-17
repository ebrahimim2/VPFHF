/*
   VFFlow.c
   Generic interface to flow solvers

     (c) 2010-2012 Blaise Bourdin, LSU. bourdin@lsu.edu
                    Keita Yoshioka, Chevron ETC
*/
#include "petsc.h"
#include "VFCartFE.h"
#include "VFCommon.h"
#include "VFFlow.h"
#include "VFFlow_FEM.h"
#include "VFFlow_SNESFEM.h"
#include "VFFlow_TSFEM.h"
#include "VFFlow_Fake.h"
#include "VFFlow_KSPMixedFEM.h"
#include "VFFlow_SNESMixedFEM.h"
#include "VFFlow_TSMixedFEM.h"
#include "VFHeat_SNESFEM.h"
#include "VFFlow_SNESStandardFEM.h"
#include "VFPermfield.h"



#undef __FUNCT__
#define __FUNCT__ "FlowSolverFinalize"
/*
   FlowSolverFinalize
   Keita Yoshioka yoshk@chevron.com
*/
extern PetscErrorCode FlowSolverFinalize(VFCtx *ctx,VFFields *fields)
{
  PetscErrorCode ierr;
  
  PetscFunctionBegin;
  
  ierr = VecDestroy(&ctx->RHSVelP);CHKERRQ(ierr);
  ierr = VecDestroy(&ctx->RHSVelPpre);CHKERRQ(ierr);
  ierr = VecDestroy(&ctx->FlowFunct);CHKERRQ(ierr);

  ierr = MatDestroy(&ctx->KVelP);CHKERRQ(ierr);
	ierr = MatDestroy(&ctx->KVelPlhs);CHKERRQ(ierr);
	ierr = MatDestroy(&ctx->JacVelP);CHKERRQ(ierr);
  
  ierr = VecDestroy(&ctx->RHSP);CHKERRQ(ierr);
	ierr = VecDestroy(&ctx->RHSPpre);CHKERRQ(ierr);
	ierr = VecDestroy(&ctx->PFunct);CHKERRQ(ierr);
  
  ierr = MatDestroy(&ctx->KP);CHKERRQ(ierr);
	ierr = MatDestroy(&ctx->KPlhs);CHKERRQ(ierr);
	ierr = MatDestroy(&ctx->JacP);CHKERRQ(ierr);
  
  switch (ctx->flowsolver) {
  case FLOWSOLVER_KSPMIXEDFEM:
    ierr = MixedFEMFlowSolverFinalize(ctx,fields);CHKERRQ(ierr);
    break;
  case FLOWSOLVER_TSMIXEDFEM:
    ierr = MixedFEMTSFlowSolverFinalize(ctx,fields);CHKERRQ(ierr);
    break;
  case FLOWSOLVER_SNESMIXEDFEM:
    ierr = MixedFEMSNESFlowSolverFinalize(ctx,fields);CHKERRQ(ierr);
    break;
  case FLOWSOLVER_SNESSTANDARDFEM:
    ierr = VFFlow_SNESStandardFEMFinalize(ctx,fields);CHKERRQ(ierr);
    break;
  case FLOWSOLVER_FEM:
    break;
  case FLOWSOLVER_TS:
    ierr = FEMTSFlowSolverFinalize(ctx,fields);CHKERRQ(ierr);
    break;
  case FLOWSOLVER_SNES:
    ierr = FEMSNESFlowSolverFinalize(ctx,fields);CHKERRQ(ierr);
    break;
  case FLOWSOLVER_FAKE:
    break;
  case FLOWSOLVER_READFROMFILES:
    break;
  case FLOWSOLVER_NONE:
    break;
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "FlowSolverInitialize"
extern PetscErrorCode FlowSolverInitialize(VFCtx *ctx,VFFields *fields)
{
  PetscErrorCode ierr;
  PetscMPIInt    comm_size;

  PetscFunctionBegin;
    
	ierr = GetFlowProp(&ctx->flowprop,&ctx->resprop,ctx->matprop,ctx,fields,ctx->nlayer);CHKERRQ(ierr);
  
  ierr = DMCreateMatrix(ctx->daScal,&ctx->KP);CHKERRQ(ierr);
  ierr = MatSetOption(ctx->KP,MAT_KEEP_NONZERO_PATTERN,PETSC_TRUE);CHKERRQ(ierr);
  ierr = MatZeroEntries(ctx->KP);CHKERRQ(ierr);

  ierr = DMCreateMatrix(ctx->daScal,&ctx->KPlhs);CHKERRQ(ierr);
  ierr = MatSetOption(ctx->KPlhs,MAT_KEEP_NONZERO_PATTERN,PETSC_TRUE);CHKERRQ(ierr);
  ierr = MatZeroEntries(ctx->KPlhs);CHKERRQ(ierr);

  ierr = DMCreateMatrix(ctx->daScal,&ctx->JacP);CHKERRQ(ierr);
  ierr = MatSetOption(ctx->JacP,MAT_KEEP_NONZERO_PATTERN,PETSC_TRUE);CHKERRQ(ierr);
  ierr = MatZeroEntries(ctx->JacP);CHKERRQ(ierr);  
  
  ierr = MPI_Comm_size(PETSC_COMM_WORLD,&comm_size);CHKERRQ(ierr);
  ierr = DMCreateMatrix(ctx->daFlow,&ctx->KVelP);CHKERRQ(ierr);
  ierr = MatSetOption(ctx->KVelP,MAT_KEEP_NONZERO_PATTERN,PETSC_TRUE);CHKERRQ(ierr);
  ierr = MatZeroEntries(ctx->KVelP);CHKERRQ(ierr);

  ierr = DMCreateMatrix(ctx->daFlow,&ctx->KVelPlhs);CHKERRQ(ierr);
  ierr = MatSetOption(ctx->KVelPlhs,MAT_KEEP_NONZERO_PATTERN,PETSC_TRUE);CHKERRQ(ierr);
  ierr = MatZeroEntries(ctx->KVelPlhs);CHKERRQ(ierr);
  
  ierr = DMCreateGlobalVector(ctx->daFlow,&ctx->RHSVelP);CHKERRQ(ierr);
  ierr = PetscObjectSetName((PetscObject)ctx->RHSVelP,"RHS of flow solver");CHKERRQ(ierr);
  ierr = VecSet(ctx->RHSVelP,0.);CHKERRQ(ierr);
  
  ierr = DMCreateGlobalVector(ctx->daFlow,&ctx->RHSVelPpre);CHKERRQ(ierr);
  ierr = PetscObjectSetName((PetscObject)ctx->RHSVelPpre,"Previous RHS of flow solver");CHKERRQ(ierr);
  ierr = VecSet(ctx->RHSVelPpre,0.);CHKERRQ(ierr);
  
  ierr = DMCreateMatrix(ctx->daFlow,&ctx->JacVelP);CHKERRQ(ierr);
	ierr = MatZeroEntries(ctx->JacVelP);CHKERRQ(ierr);
	ierr = MatSetOption(ctx->JacVelP,MAT_KEEP_NONZERO_PATTERN,PETSC_TRUE);CHKERRQ(ierr);
  
	ierr = DMCreateGlobalVector(ctx->daFlow,&ctx->FlowFunct);CHKERRQ(ierr);
	ierr = PetscObjectSetName((PetscObject)ctx->FlowFunct,"RHS of SNES flow solver");CHKERRQ(ierr);
	ierr = VecSet(ctx->FlowFunct,0.);CHKERRQ(ierr);
  
  ierr = DMCreateGlobalVector(ctx->daScal,&ctx->RHSP);CHKERRQ(ierr);
  ierr = PetscObjectSetName((PetscObject) ctx->RHSP,"RHS of Standard Flow FEM Formulation");CHKERRQ(ierr);
  ierr = VecSet(ctx->RHSP,0.);CHKERRQ(ierr);
  
	ierr = DMCreateGlobalVector(ctx->daScal,&ctx->RHSPpre);CHKERRQ(ierr);
  ierr = PetscObjectSetName((PetscObject) ctx->RHSPpre,"RHS of Standard Flow FEM Formulation");CHKERRQ(ierr);
  ierr = VecSet(ctx->RHSPpre,0.);CHKERRQ(ierr);
  
	ierr = DMCreateGlobalVector(ctx->daScal,&ctx->PFunct);CHKERRQ(ierr);
  ierr = PetscObjectSetName((PetscObject)ctx->PFunct,"Residual of Standard FEM Flow Formulation");CHKERRQ(ierr);
  ierr = VecSet(ctx->PFunct,0.);CHKERRQ(ierr);
  
  switch (ctx->flowsolver) {
  case FLOWSOLVER_KSPMIXEDFEM:
    ierr = MixedFEMFlowSolverInitialize(ctx,fields);CHKERRQ(ierr);
    break;
  case FLOWSOLVER_TSMIXEDFEM:
    ierr = MixedFEMTSFlowSolverInitialize(ctx,fields);CHKERRQ(ierr);
    break;
  case FLOWSOLVER_SNESMIXEDFEM:
    ierr = MixedFEMSNESFlowSolverInitialize(ctx,fields);CHKERRQ(ierr);
    break;
  case FLOWSOLVER_SNESSTANDARDFEM:
    ierr = VFFlow_SNESStandardFEMInitialize(ctx,fields);CHKERRQ(ierr);
    break;
  case FLOWSOLVER_FEM:
    break;
  case FLOWSOLVER_TS:
    ierr = FEMTSFlowSolverInitialize(ctx,fields);CHKERRQ(ierr);
    break;
  case FLOWSOLVER_SNES:
    ierr = FEMSNESFlowSolverInitialize(ctx,fields);CHKERRQ(ierr);
    break;
  case FLOWSOLVER_FAKE:
    break;
  case FLOWSOLVER_READFROMFILES:
    break;
  default:
    break;
  }
  PetscFunctionReturn(0);
}


/*
 VF_StepP: Does one time step of the flow solver selected in ctx.flowsolver
 */
#undef __FUNCT__
#define __FUNCT__ "VF_StepP"
extern PetscErrorCode VF_StepP(VFFields *fields,VFCtx *ctx)
{
  PetscErrorCode ierr;
  
  PetscFunctionBegin;
  switch (ctx->flowsolver) {
    case FLOWSOLVER_TS:
      ierr = FlowFEMTSSolve(ctx,fields);CHKERRQ(ierr);
      break;
    case FLOWSOLVER_FEM:
      /*
       ierr = VFFlow_FEM(ctx,fields);CHKERRQ(ierr);
       */
      break;
    case FLOWSOLVER_SNES:
      ierr = FlowFEMSNESSolve(ctx,fields);CHKERRQ(ierr);
      break;
    case FLOWSOLVER_KSPMIXEDFEM:
      ierr = MixedFlowFEMKSPSolve(ctx,fields);CHKERRQ(ierr);
      break;
    case FLOWSOLVER_TSMIXEDFEM:
      ierr = MixedFlowFEMTSSolve(ctx,fields);CHKERRQ(ierr);
      break;
    case FLOWSOLVER_SNESMIXEDFEM:
      ierr = MixedFlowFEMSNESSolve(ctx,fields);CHKERRQ(ierr);
      break;
    case FLOWSOLVER_SNESSTANDARDFEM:
    ierr = VF_FlowStandardFEMSNESSolve(ctx,fields);CHKERRQ(ierr);
      break;
    case FLOWSOLVER_FAKE:
      ierr = VFFlow_Fake(ctx,fields);CHKERRQ(ierr);
      break;
    case FLOWSOLVER_READFROMFILES:
      SETERRQ1(PETSC_COMM_WORLD,PETSC_ERR_SUP,"Reading from files not implemented yet, %s",__FUNCT__);
      break;
    default:
      break;
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "BCQInit"
extern PetscErrorCode BCQInit(VFBC *BCQ,VFCtx *ctx)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = VFBCCreate(BCQ,3);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "BCFracQInit"
extern PetscErrorCode BCFracQInit(VFBC *BCFracQ,VFCtx *ctx)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = VFBCCreate(BCFracQ,3);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "BCPInit"
extern PetscErrorCode BCPInit(VFBC *BCP,VFCtx *ctx)
{
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = VFBCCreate(BCP,1);CHKERRQ(ierr);


/*
  When positive value is given, boundary condiction has a numerical value
*/
/*
  if (ctx->BCpres[0] > -1.e-8) {
    BCP[0].face[X0] = FIXED;
  }
  if (ctx->BCpres[1] > -1.e-8) {
    BCP[0].face[X1] = FIXED;
  }
  if (ctx->BCpres[2] > -1.e-8) {
    BCP[0].face[Y0] = FIXED;
  }
  if (ctx->BCpres[3] > -1.e-8) {
    BCP[0].face[Y1] = FIXED;
  }
  if (ctx->BCpres[4] > -1.e-8) {
    BCP[0].face[Z0] = FIXED;
  }
  if (ctx->BCpres[5] > -1.e-8) {
    BCP[0].face[Z1] = FIXED;
  }
 */
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "SETBoundaryTerms_P"
extern PetscErrorCode SETBoundaryTerms_P(VFCtx *ctx, VFFields *fields)
{
  PetscErrorCode ierr;
  PetscReal      ***pressure_array;
  PetscInt       xs,xm,nx;
  PetscInt       ys,ym,ny;
  PetscInt       zs,zm,nz;
  PetscInt       i,j,k;

  PetscFunctionBegin;
  ierr = DMDAGetInfo(ctx->daScal,NULL,&nx,&ny,&nz,NULL,NULL,NULL,
                     NULL,NULL,NULL,NULL,NULL,NULL);CHKERRQ(ierr);
  ierr = DMDAGetCorners(ctx->daScal,&xs,&ys,&zs,&xm,&ym,&zm);CHKERRQ(ierr);
  ierr = DMDAVecGetArray(ctx->daScal,ctx->PresBCArray,&pressure_array);CHKERRQ(ierr);

  for (k = zs; k < zs+zm; k++) {
    for (j = ys; j < ys+ym; j++)
      for (i = xs; i < xs+xm; i++) {
        /* x == 0 */
        if (i == 0 && ctx->bcP[0].face[X0] == FIXED) pressure_array[k][j][i] = ctx->BCpres[X0];
        /* x == nx-1 */
        else if (i == nx-1 && ctx->bcP[0].face[X1] == FIXED) pressure_array[k][j][i] = ctx->BCpres[X1];
        /* y == 0 */
        else if (j == 0 && ctx->bcP[0].face[Y0] == FIXED) pressure_array[k][j][i] = ctx->BCpres[Y0];
        /*  y == ny-1 */
        else if (j == ny-1 && ctx->bcP[0].face[Y1] == FIXED) pressure_array[k][j][i] = ctx->BCpres[Y1];
        /*  z == 0 */
        else if (k == 0 && ctx->bcP[0].face[Z0] == FIXED) pressure_array[k][j][i] = ctx->BCpres[Z0];
        /*  z == nz-1 */
        else if (k == nz-1 && ctx->bcP[0].face[Z1] == FIXED) pressure_array[k][j][i] = ctx->BCpres[Z1];
        else pressure_array[k][j][i] = ctx->resprop.Pinit;
      }
  }
  ierr = DMDAVecRestoreArray(ctx->daScal,ctx->PresBCArray,&pressure_array);CHKERRQ(ierr);

  PetscFunctionReturn(0);
}


#undef __FUNCT__
#define __FUNCT__ "VecApplyPressureBC_SNES"
extern PetscErrorCode VecApplyPressureBC_SNES(Vec Func,Vec pressure, Vec BCF,VFBC *BC)
{
  PetscErrorCode ierr;
  PetscInt       xs,xm,nx;
  PetscInt       ys,ym,ny;
  PetscInt       zs,zm,nz;
  PetscInt       i,j,k;
  DM             da;
  PetscReal      ****func_array;
  PetscReal      ****BCF_array;
  PetscReal      ****pressure_array;
  PetscInt       dim,dof;

  PetscFunctionBegin;
  ierr = VecGetDM(Func,&da);CHKERRQ(ierr);
  ierr = DMDAGetInfo(da,&dim,&nx,&ny,&nz,NULL,NULL,NULL,
                     &dof,NULL,NULL,NULL,NULL,NULL);CHKERRQ(ierr);
  ierr = DMDAGetCorners(da,&xs,&ys,&zs,&xm,&ym,&zm);CHKERRQ(ierr);

  ierr = DMDAVecGetArrayDOF(da,Func,&func_array);CHKERRQ(ierr);
  ierr = DMDAVecGetArrayDOF(da,BCF,&BCF_array);CHKERRQ(ierr);
  ierr = DMDAVecGetArrayDOF(da,pressure,&pressure_array);CHKERRQ(ierr);

  /*
    Faces
  */
  if (xs == 0) {
    /*
      x == 0
    */
    i = 0;
    if (BC[0].face[X0] == FIXED)
      for (k = zs; k < zs + zm; k++)
        for (j = ys; j < ys + ym; j++) func_array[k][j][i][0] = pressure_array[k][j][i][0] - BCF_array[k][j][i][0];
  }
  if (xs + xm == nx) {
    /*
      x == nx-1
    */
    i = nx-1;
    if (BC[0].face[X1] == FIXED)
      for (k = zs; k < zs + zm; k++)
        for (j = ys; j < ys + ym; j++) func_array[k][j][i][0] =  pressure_array[k][j][i][0] - BCF_array[k][j][i][0];
  }
  if (ys == 0) {
    /*
      y == 0
    */
    j = 0;
    if (BC[0].face[Y0] == FIXED)
      for (k = zs; k < zs + zm; k++)
        for (i = xs; i < xs + xm; i++) func_array[k][j][i][0] =  pressure_array[k][j][i][0] - BCF_array[k][j][i][0];
  }
  if (ys + ym == ny) {
    /*
      y == ny-1
    */
    j = ny-1;
    if (BC[0].face[Y1] == FIXED)
      for (k = zs; k < zs + zm; k++)
        for (i = xs; i < xs + xm; i++) func_array[k][j][i][0] =  pressure_array[k][j][i][0] - BCF_array[k][j][i][0];
  }
  
  if (zs == 0) {
    /*
      z == 0
    */
    k = 0;
    if (BC[0].face[Z0] == FIXED)
      for (j = ys; j < ys + ym; j++)
        for (i = xs; i < xs + xm; i++) func_array[k][j][i][0] =  pressure_array[k][j][i][0] - BCF_array[k][j][i][0];
  }
  if (zs + zm == nz) {
    /*
      z == nz-1
    */
    k = nz-1;
    if (BC[0].face[Z1] == FIXED)
      for (j = ys; j < ys + ym; j++)
        for (i = xs; i < xs + xm; i++) func_array[k][j][i][0] =  pressure_array[k][j][i][0] - BCF_array[k][j][i][0];
  }
    

  ierr = DMDAVecRestoreArrayDOF(da,Func,&func_array);CHKERRQ(ierr);
  ierr = DMDAVecRestoreArrayDOF(da,BCF,&BCF_array);CHKERRQ(ierr);
  ierr = DMDAVecRestoreArrayDOF(da,pressure,&pressure_array);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "VecApplyPressureBC_FEM"
extern PetscErrorCode VecApplyPressureBC_FEM(Vec RHS,Vec BCF,VFBC *BC)
{
  PetscErrorCode ierr;
  PetscInt       xs,xm,nx;
  PetscInt       ys,ym,ny;
  PetscInt       zs,zm,nz;
  PetscInt       i,j,k,c;
  DM             da;
  PetscReal      ****RHS_array;
  PetscReal      ****BCF_array;
  PetscInt       dim,dof;

  PetscFunctionBegin;
  ierr = VecGetDM(RHS,&da);CHKERRQ(ierr);
  ierr = DMDAGetInfo(da,&dim,&nx,&ny,&nz,NULL,NULL,NULL,
                     &dof,NULL,NULL,NULL,NULL,NULL);CHKERRQ(ierr);
  ierr = DMDAGetCorners(da,&xs,&ys,&zs,&xm,&ym,&zm);CHKERRQ(ierr);

  if (dim == 2) {
    ierr = PetscMalloc(sizeof(PetscReal ***),&RHS_array);CHKERRQ(ierr);
    ierr = PetscMalloc(sizeof(PetscReal ***),&BCF_array);CHKERRQ(ierr);
    ierr = DMDAVecGetArrayDOF(da,RHS,&RHS_array[0]);CHKERRQ(ierr);
    ierr = DMDAVecGetArrayDOF(da,BCF,&BCF_array[0]);CHKERRQ(ierr);
  } else {
    ierr = DMDAVecGetArrayDOF(da,RHS,&RHS_array);CHKERRQ(ierr);
    ierr = DMDAVecGetArrayDOF(da,BCF,&BCF_array);CHKERRQ(ierr);
  }

  for (c = 0; c < dof; c++) {
    /*
      Faces
    */
    if (xs == 0) {
      /*
        x == 0
      */
      i = 0;
      if (BC[c].face[X0] == FIXED)
        for (k = zs; k < zs + zm; k++)
          for (j = ys; j < ys + ym; j++) RHS_array[k][j][i][c] = BCF_array[k][j][i][c];
    }
    if (xs + xm == nx) {
      /*
        x == nx-1
      */
      i = nx-1;
      if (BC[c].face[X1] == FIXED)
        for (k = zs; k < zs + zm; k++)
          for (j = ys; j < ys + ym; j++) RHS_array[k][j][i][c] = BCF_array[k][j][i][c];
    }
    if (ys == 0) {
      /*
        y == 0
      */
      j = 0;
      if (BC[c].face[Y0] == FIXED)
        for (k = zs; k < zs + zm; k++)
          for (i = xs; i < xs + xm; i++) RHS_array[k][j][i][c] = BCF_array[k][j][i][c];
    }
    if (ys + ym == ny) {
      /*
        y == ny-1
      */
      j = ny-1;
      if (BC[c].face[Y1] == FIXED)
        for (k = zs; k < zs + zm; k++)
          for (i = xs; i < xs + xm; i++) RHS_array[k][j][i][c] = BCF_array[k][j][i][c];
    }
    if (dim == 3) {
      if (zs == 0) {
        /*
          z == 0
        */
        k = 0;
        if (BC[c].face[Z0] == FIXED)
          for (j = ys; j < ys + ym; j++)
            for (i = xs; i < xs + xm; i++) RHS_array[k][j][i][c] = BCF_array[k][j][i][c];
      }
      if (zs + zm == nz) {
        /*
          z == nz-1
        */
        k = nz-1;
        if (BC[c].face[Z1] == FIXED)
          for (j = ys; j < ys + ym; j++)
            for (i = xs; i < xs + xm; i++) RHS_array[k][j][i][c] = BCF_array[k][j][i][c];
      }
    }
  }

  if (dim == 2) {
    ierr = DMDAVecRestoreArrayDOF(da,RHS,&RHS_array[0]);CHKERRQ(ierr);
    ierr = DMDAVecRestoreArrayDOF(da,BCF,&BCF_array[0]);CHKERRQ(ierr);
    ierr = PetscFree(RHS_array);CHKERRQ(ierr);
    ierr = PetscFree(BCF_array);CHKERRQ(ierr);
  } else {
    ierr = DMDAVecRestoreArrayDOF(da,RHS,&RHS_array);CHKERRQ(ierr);
    ierr = DMDAVecRestoreArrayDOF(da,BCF,&BCF_array);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "MatApplyPressureBC_FEM"
extern PetscErrorCode MatApplyPressureBC_FEM(Mat K,Mat M,VFBC *bcP)
{
  PetscErrorCode ierr;
  PetscInt       xs,xm,nx;
  PetscInt       ys,ym,ny;
  PetscInt       zs,zm,nz;
  PetscInt       i,j,k;
  MatStencil     *row;
  PetscReal      zero =0.0;
  PetscReal      one  =1.;
  PetscInt       numBC=0,l=0,dim;
  DM             da;

  PetscFunctionBegin;
  ierr = MatGetDM(K,&da);CHKERRQ(ierr);
  ierr = DMDAGetInfo(da,&dim,&nx,&ny,&nz,NULL,NULL,NULL,
                     NULL,NULL,NULL,NULL,NULL,NULL);CHKERRQ(ierr);
  ierr = DMDAGetCorners(da,&xs,&ys,&zs,&xm,&ym,&zm);CHKERRQ(ierr);

  if (xs == 0       && bcP[0].face[X0] == FIXED) numBC += ym * zm;
  if (xs + xm == nx && bcP[0].face[X1] == FIXED) numBC += ym * zm;
  if (ys == 0       && bcP[0].face[Y0] == FIXED) numBC += xm * zm;
  if (ys + ym == ny && bcP[0].face[Y1] == FIXED) numBC += xm * zm;
  if (zs == 0       && bcP[0].face[Z0] == FIXED && dim == 3) numBC += xm * ym;
  if (zs + zm == nz && bcP[0].face[Z1] == FIXED && dim == 3) numBC += xm * ym;


  ierr = PetscMalloc(numBC * sizeof(MatStencil),&row);CHKERRQ(ierr);
  /*
   Create an array of rows to be zeroed out
   */
  /*
   i == 0
   */
  if (xs == 0 && bcP[0].face[X0] == FIXED) {
    for (k = zs; k < zs + zm; k++)
      for (j = ys; j < ys + ym; j++) {
        row[l].i = 0; row[l].j = j; row[l].k = k; row[l].c = 0;
        l++;
      }
  }
  /*
   i == nx-1
  */
  if (xs + xm == nx && bcP[0].face[X1] == FIXED) {
    for (k = zs; k < zs + zm; k++)
      for (j = ys; j < ys + ym; j++) {
        row[l].i = nx-1; row[l].j = j; row[l].k = k; row[l].c = 0;
        l++;
      }
  }
  /*
   y == 0
  */
  if (ys == 0 && bcP[0].face[Y0] == FIXED) {
    for (k = zs; k < zs + zm; k++)
      for (i = xs; i < xs + xm; i++) {
        row[l].i = i; row[l].j = 0; row[l].k = k; row[l].c = 0;
        l++;
      }
  }
  /*
   y == ny-1
  */
  if (ys + ym == ny && bcP[0].face[Y1] == FIXED) {
    for (k = zs; k < zs + zm; k++)
      for (i = xs; i < xs + xm; i++) {
        row[l].i = i; row[l].j = ny-1; row[l].k = k; row[l].c = 0;
        l++;
      }
  }
  /*
   z == 0
  */
  if (zs == 0 && bcP[0].face[Z0] == FIXED) {
    for (j = ys; j < ys + ym; j++)
      for (i = xs; i < xs + xm; i++) {
        row[l].i = i; row[l].j = j; row[l].k = 0; row[l].c = 0;
        l++;
      }
  }
  /*
   z == nz-1
  */
  if (zs + zm == nz && bcP[0].face[Z1] == FIXED) {
    for (j = ys; j < ys + ym; j++)
      for (i = xs; i < xs + xm; i++) {
        row[l].i = i; row[l].j = j; row[l].k = nz-1; row[l].c = 0;
        l++;
      }
  }

  ierr = MatZeroRowsStencil(K,numBC,row,one,NULL,NULL);CHKERRQ(ierr);
  ierr = MatZeroRowsStencil(M,numBC,row,zero,NULL,NULL);CHKERRQ(ierr);
  ierr = PetscFree(row);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "VFFlow_FEM_MatKPAssembly3D_local"
/*
   VFFlow_FEM_MatPAssembly3D_local
*/

extern PetscErrorCode VFFlow_FEM_MatKPAssembly3D_local(PetscReal *Mat_local,VFFlowProp *flowprop,PetscReal ****perm_array, PetscInt ek,PetscInt ej,PetscInt ei,VFCartFEElement3D *e)
{
  PetscInt  g,i1,i2,j1,j2,k1,k2,l;
  PetscReal rho,mu;
  PetscReal kxx,kyy,kzz,kxy,kxz,kyz;
  PetscReal DCoef_P;

  PetscFunctionBegin;
/*
  The following properties should be changed to a function of pressure and temperature (and saturation for multi-phase)
*/
  rho     = flowprop->rho;
  mu      = flowprop->mu;
  DCoef_P = -1*rho/mu;
/*
  Permeability should be associated with each element later
*/
  kxx = perm_array[ek][ej][ei][0];
  kyy = perm_array[ek][ej][ei][1];
  kzz = perm_array[ek][ej][ei][2];
  kxy = perm_array[ek][ej][ei][3];
  kxz = perm_array[ek][ej][ei][4];
  kyz = perm_array[ek][ej][ei][5];

  for (l = 0,k1 = 0; k1 < e->nphiz; k1++)
    for (j1 = 0; j1 < e->nphiy; j1++) {
      for (i1 = 0; i1 < e->nphix; i1++)
        for (k2 = 0; k2 < e->nphiz; k2++) {
          for (j2 = 0; j2 < e->nphiy; j2++)
            for (i2 = 0; i2 < e->nphix; i2++,l++) {
              Mat_local[l] = 0.;
              for (g = 0; g < e->ng; g++)
                Mat_local[l] += e->weight[g]*DCoef_P*(kxx*e->dphi[k1][j1][i1][0][g]*e->dphi[k2][j2][i2][0][g]
                                                      +kyy*e->dphi[k1][j1][i1][1][g]*e->dphi[k2][j2][i2][1][g]
                                                      +kzz*e->dphi[k1][j1][i1][2][g]*e->dphi[k2][j2][i2][2][g]
                                                      +kxy*(e->dphi[k1][j1][i1][1][g]*e->dphi[k2][j2][i2][0][g]+e->dphi[k1][j1][i1][0][g]*e->dphi[k2][j2][i2][1][g])
                                                      +kxz*(e->dphi[k1][j1][i1][2][g]*e->dphi[k2][j2][i2][0][g]+e->dphi[k1][j1][i1][0][g]*e->dphi[k2][j2][i2][2][g])
                                                      +kyz*(e->dphi[k1][j1][i1][2][g]*e->dphi[k2][j2][i2][1][g]+e->dphi[k1][j1][i1][1][g]*e->dphi[k2][j2][i2][2][g]));
            }
        }
    }

  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "VFFlow_FEM_MatMPAssembly3D_local"
/*
   VFFlow_FEM_MassMatPAssembly3D_local
*/

extern PetscErrorCode VFFlow_FEM_MatMPAssembly3D_local(PetscReal *Mat_local,VFFlowProp *flowprop,PetscInt ek,PetscInt ej,PetscInt ei,PetscReal ACoef_P,VFCartFEElement3D *e)
{
  PetscInt  g,i1,i2,j1,j2,k1,k2,l;

  PetscFunctionBegin;
/*
  The following properties should be changed to a function of pressure and temperature (and saturation for multi-phase)
*/
  for (l = 0,k1 = 0; k1 < e->nphiz; k1++){
    for (j1 = 0; j1 < e->nphiy; j1++) {
      for (i1 = 0; i1 < e->nphix; i1++){
        for (k2 = 0; k2 < e->nphiz; k2++) {
          for (j2 = 0; j2 < e->nphiy; j2++){
            for (i2 = 0; i2 < e->nphix; i2++,l++) {
              Mat_local[l] = 0.;
              for (g = 0; g < e->ng; g++) Mat_local[l] += e->weight[g]*ACoef_P*e->phi[k1][j1][i1][g]*e->phi[k2][j2][i2][g];
            }
          }
		}
      }
	}
  }
  PetscFunctionReturn(0);
}
#undef __FUNCT__
#define __FUNCT__ "Flow_Vecg_FEM"
extern PetscErrorCode Flow_Vecg_FEM(PetscReal *Kg_local,VFCartFEElement3D *e,PetscInt ek,PetscInt ej,PetscInt ei,VFFlowProp flowpropty,PetscReal ****perm_array)
{
        PetscInt  i,j,k,l;
        PetscInt  eg;
        PetscReal mu,rho,gx,gy,gz;
        PetscReal kx,ky,kz,kxy,kxz,kyz;
        
        PetscFunctionBegin;
        rho     = flowpropty.rho;
        mu      = flowpropty.mu;
        gx      = flowpropty.g[0];
        gy      = flowpropty.g[1];
        gz      = flowpropty.g[2];
        
        kx  = perm_array[ek][ej][ei][0];
        ky  = perm_array[ek][ej][ei][1];
        kz  = perm_array[ek][ej][ei][2];
        kxy = perm_array[ek][ej][ei][3];
        kxz = perm_array[ek][ej][ei][4];
        kyz = perm_array[ek][ej][ei][5];
        
        for (l = 0,k = 0; k < e->nphiz; k++) {
                for (j = 0; j < e->nphiy; j++) {
                        for (i = 0; i < e->nphix; i++,l++) {
                                Kg_local[l] = 0.;
                                for (eg = 0; eg < e->ng; eg++) {
                                        /* Need to multiply by the permability when it is available*/
                                        Kg_local[l] += -0.5*rho/mu*((kx*gx+kxy*gy+kxz*gz)*e->dphi[k][j][i][0][eg]
                                                                                                                           +(kxy*gx+ky*gy+kyz*gz)*e->dphi[k][j][i][1][eg]
                                                                                                                           +(kxz*gx+kyz*gy+kz*gz)*e->dphi[k][j][i][2][eg])*e->weight[eg];
                                }
                        }
                }
        }
        PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "VecApplySourceTerms_FEM"
extern PetscErrorCode VecApplySourceTerms_FEM(PetscReal *Ks_local,PetscReal ***source_array,VFCartFEElement3D *e,PetscInt ek,PetscInt ej,PetscInt ei,VFCtx *ctx)
{
        PetscErrorCode ierr;
        PetscInt       i,j,k,l;
        PetscReal      *loc_source;
        PetscInt       eg;
        
        PetscFunctionBegin;
        ierr   = PetscMalloc(e->ng*sizeof(PetscReal),&loc_source);CHKERRQ(ierr);
        
        for (eg = 0; eg < e->ng; eg++) loc_source[eg] = 0.;
        for (k = 0; k < e->nphiz; k++) {
                for (j = 0; j < e->nphiy; j++) {
                        for (i = 0; i < e->nphix; i++) {
                                for (eg = 0; eg < e->ng; eg++) {
                                        loc_source[eg] += source_array[ek+k][ej+j][ei+i]*e->phi[k][j][i][eg];
                                }
                        }
                }
        }

        for (l = 0,k = 0; k < e->nphiz; k++) {
                for (j = 0; j < e->nphiy; j++) {
                        for (i = 0; i < e->nphix; i++,l++) {
                                Ks_local[l] = 0.;
                                for (eg = 0; eg < e->ng; eg++) {
                                        Ks_local[l] += -loc_source[eg]*e->phi[k][j][i][eg]*e->weight[eg];
                                }
                        }
                }
        }
        PetscFree(loc_source);
        PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "VecApplyWellFlowRate_FEM"
extern PetscErrorCode VecApplyWellFlowRate_FEM(PetscReal *RHS_local,PetscReal Q,PetscReal hwx,PetscReal hwy,PetscReal hwz)
{
	PetscReal	phi[2][2][2];
	PetscInt	i,j,k,l;

	PetscFunctionBegin;
	phi[0][0][0] = (1.-hwz)*(1.-hwy)*(1.-hwx);
	phi[0][0][1] = (1.-hwz)*(1.-hwy)*hwx;
	phi[0][1][0] = (1.-hwz)*hwy*(1.-hwx);
	phi[0][1][1] = (1.-hwz)*hwy*hwx;
	phi[1][0][0] = hwz*(1.-hwy)*(1.-hwx);
	phi[1][0][1] = hwz*(1.-hwy)*hwx;
	phi[1][1][0] = hwz*hwy*(1.-hwx);
	phi[1][1][1] = hwz*hwy*hwx;

	for(l = 0, k = 0; k < 2; k++){
		for(j = 0; j < 2; j++){
			for(i = 0; i < 2; i++, l++){		
				RHS_local[l] = Q*phi[k][j][i];
			}
		}
	}
	PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "FEMSNESMonitor"
extern PetscErrorCode FEMSNESMonitor(SNES snes,PetscInt its,PetscReal fnorm,void * ptr)
{
  PetscErrorCode ierr;
  PetscReal      norm,vmax,vmin;
  MPI_Comm       comm;
  Vec            solution;


  PetscFunctionBegin;
  ierr = SNESGetSolution(snes, &solution);CHKERRQ(ierr);
  ierr = VecNorm(solution,NORM_1,&norm);CHKERRQ(ierr);
  ierr = VecMax(solution,NULL,&vmax);CHKERRQ(ierr);
  ierr = VecMin(solution,NULL,&vmin);CHKERRQ(ierr);
  ierr = PetscObjectGetComm((PetscObject)snes,&comm);CHKERRQ(ierr);
  ierr = PetscPrintf(comm,"snes_iter_step %D :solution norm = %G, max sol. value  = %G, min sol. value = %G\n",its,norm,vmax,vmin);CHKERRQ(ierr);

  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "GetFlowProp"
extern PetscErrorCode GetFlowProp(VFFlowProp *flowprop,VFResProp *resprop,VFMatProp *matprop,VFCtx *ctx,VFFields *fields,PetscInt n)
{
  
  PetscErrorCode ierr;
  PetscInt       xs,xm,nx;
  PetscInt       ys,ym,ny;
  PetscInt       zs,zm,nz;
  PetscInt       nval;
  PetscInt       ek,ej,ei,c,i;
  PetscReal      ****perm_array;
  Vec            perm_local;
  PetscReal      ***m_inv_array;
  Vec            m_inv_local;
  PetscReal      ***k_dr_array;
  Vec            k_dr_local;
  PetscReal      *kx,*ky,*kz,*g,*prop;
  Vec             pmult_local;
	PetscReal       ***pmult_array;

  PetscFunctionBegin;
  ierr = DMDAGetInfo(ctx->daScalCell,NULL,&nx,&ny,&nz,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);CHKERRQ(ierr);
  ierr = DMDAGetCorners(ctx->daScalCell,&xs,&ys,&zs,&xm,&ym,&zm);CHKERRQ(ierr);
  ierr = PetscMalloc(n * sizeof(PetscReal),&prop);CHKERRQ(ierr);
  ierr = PetscMalloc(n * sizeof(PetscReal),&kx);CHKERRQ(ierr);
  ierr = PetscMalloc(n * sizeof(PetscReal),&ky);CHKERRQ(ierr);
  ierr = PetscMalloc(n * sizeof(PetscReal),&kz);CHKERRQ(ierr);
  ierr = PetscMalloc(3 * sizeof(PetscReal),&g);CHKERRQ(ierr);
  flowprop->Kf    = 1.;
  flowprop->Cp    = 1.;                                         
  for(i = 0; i < n; i++){
    if(nx == 2 || ny == 2 || nz == 2){
      prop[i] = (1-matprop[i].nu)*matprop[i].E/((1+matprop[i].nu)*(1-2*matprop[i].nu));
      prop[i] = matprop[i].E/(2*(1+matprop[i].nu)*(1-2*matprop[i].nu));
    }
    else{
      prop[i] = matprop[i].E/(3*(1-2*matprop[i].nu));
    }
  }
  ierr = DMGetLocalVector(ctx->daScalCell,&k_dr_local);CHKERRQ(ierr);
  ierr = DMGlobalToLocalBegin(ctx->daScalCell,ctx->K_dr,INSERT_VALUES,k_dr_local);CHKERRQ(ierr);
  ierr = DMGlobalToLocalEnd(ctx->daScalCell,ctx->K_dr,INSERT_VALUES,k_dr_local);CHKERRQ(ierr);
  ierr = DMDAVecGetArray(ctx->daScalCell,k_dr_local,&k_dr_array);CHKERRQ(ierr);
  for (ek = zs; ek < zs+zm; ek++){
    for (ej = ys; ej < ys+ym; ej++){
      for (ei = xs; ei < xs+xm; ei++){
        k_dr_array[ek][ej][ei] = prop[ctx->layer[ek]];
      }
    }
  }
  ierr = DMDAVecRestoreArray(ctx->daScalCell,k_dr_local,&k_dr_array);CHKERRQ(ierr);
	ierr = DMLocalToGlobalBegin(ctx->daScalCell,k_dr_local,INSERT_VALUES,ctx->K_dr);CHKERRQ(ierr);
	ierr = DMLocalToGlobalEnd(ctx->daScalCell,k_dr_local,INSERT_VALUES,ctx->K_dr);CHKERRQ(ierr);
  ierr = DMRestoreLocalVector(ctx->daScalCell,&k_dr_local);CHKERRQ(ierr);

  ierr = PetscOptionsBegin(PETSC_COMM_WORLD,NULL,"\n\nVF: flow property options:","");CHKERRQ(ierr);
  {
    ctx->pmult_vtol  = 0.2;
    ierr             = PetscOptionsReal("-pmultvtol","\n\t v-field tolerance for permeability multiplier","",ctx->pmult_vtol,&ctx->pmult_vtol,NULL);CHKERRQ(ierr);
    nval = n;
    for (i = 0; i < n; i++) kx[i] = 1.;
    ierr = PetscOptionsRealArray("-kx","\n\t Comma seperated X-component of permeability (default 1.)","",kx,&nval,NULL);CHKERRQ(ierr);
    if (nval != n && nval != 0) SETERRQ4(PETSC_COMM_WORLD,PETSC_ERR_USER,"ERROR: Expecting %i values for option %s, got only %i in %s\n",n,"-kx",nval,__FUNCT__);
    
    nval = n;
    for (i = 0; i < n; i++) ky[i] = 1.;
    ierr = PetscOptionsRealArray("-ky","\n\t Comma seperated Y-component of permeability (default 1.)","",ky,&nval,NULL);CHKERRQ(ierr);
    if (nval != n && nval != 0) SETERRQ4(PETSC_COMM_WORLD,PETSC_ERR_USER,"ERROR: Expecting %i values for option %s, got only %i in %s\n",n,"-ky",nval,__FUNCT__);
    
    nval = n;
    for (i = 0; i < n; i++) kz[i] = 1.;
    ierr = PetscOptionsRealArray("-kz","\n\t Comma seperated Z-component of permeability (default 1.)","",kz,&nval,NULL);CHKERRQ(ierr);
    if (nval != n && nval != 0) SETERRQ4(PETSC_COMM_WORLD,PETSC_ERR_USER,"ERROR: Expecting %i values for option %s, got only %i in %s\n",n,"-kz",nval,__FUNCT__);
    
    ierr = DMGetLocalVector(ctx->daScalCell,&pmult_local);CHKERRQ(ierr);
    ierr = DMGlobalToLocalBegin(ctx->daScalCell,fields->pmult,INSERT_VALUES,pmult_local);CHKERRQ(ierr);
    ierr = DMGlobalToLocalEnd(ctx->daScalCell,fields->pmult,INSERT_VALUES,pmult_local);CHKERRQ(ierr);
    ierr = DMDAVecGetArray(ctx->daScalCell,pmult_local,&pmult_array);CHKERRQ(ierr);

    ierr = DMGetLocalVector(ctx->daVFperm,&perm_local);CHKERRQ(ierr);
    ierr = DMGlobalToLocalBegin(ctx->daVFperm,fields->vfperm,INSERT_VALUES,perm_local);CHKERRQ(ierr);
    ierr = DMGlobalToLocalEnd(ctx->daVFperm,fields->vfperm,INSERT_VALUES,perm_local);CHKERRQ(ierr);
    ierr = DMDAVecGetArrayDOF(ctx->daVFperm,perm_local,&perm_array);CHKERRQ(ierr);
    
    for (ek = zs; ek < zs+zm; ek++){
      for (ej = ys; ej < ys+ym; ej++){
        for (ei = xs; ei < xs+xm; ei++){
          pmult_array[ek][ej][ei] = ctx->vfprop.permmult;
          for (c = 0; c < 3; c++){
            perm_array[ek][ej][ei][0] = kx[ctx->layer[ek]];
            perm_array[ek][ej][ei][1] = ky[ctx->layer[ek]];
            perm_array[ek][ej][ei][2] = kz[ctx->layer[ek]];
          }
          for (c = 3; c < 6; c++){
            perm_array[ek][ej][ei][c] = 0.;
          }
        }
      }
    }
    
    ierr = DMDAVecRestoreArray(ctx->daScalCell,pmult_local,&pmult_array);CHKERRQ(ierr);
    ierr = DMLocalToGlobalBegin(ctx->daScalCell,pmult_local,INSERT_VALUES,fields->pmult);CHKERRQ(ierr);
    ierr = DMLocalToGlobalEnd(ctx->daScalCell,pmult_local,INSERT_VALUES,fields->pmult);CHKERRQ(ierr);
    ierr = DMRestoreLocalVector(ctx->daScalCell,&pmult_local);CHKERRQ(ierr);

    ierr = DMDAVecRestoreArrayDOF(ctx->daVFperm,perm_local,&perm_array);CHKERRQ(ierr);
    ierr = DMLocalToGlobalBegin(ctx->daVFperm,perm_local,INSERT_VALUES,fields->vfperm);CHKERRQ(ierr);
    ierr = DMLocalToGlobalEnd(ctx->daVFperm,perm_local,INSERT_VALUES,fields->vfperm);CHKERRQ(ierr);
    ierr = DMRestoreLocalVector(ctx->daVFperm,&perm_local);CHKERRQ(ierr);
    ierr = VecCopy(fields->vfperm,ctx->Perm);CHKERRQ(ierr);
    
    flowprop->Kf        = 1.;
    ierr = PetscOptionsReal("-Kf","\n\tLiquid modulus ","",flowprop->Kf,&flowprop->Kf,NULL);CHKERRQ(ierr);
    for(i = 0; i < n; i++){
      prop[i] = matprop[i].phi/flowprop->Kf+(matprop[i].beta-matprop[i].phi)/matprop[i].Ks;
    }
    ierr = DMGetLocalVector(ctx->daScalCell,&m_inv_local);CHKERRQ(ierr);
    ierr = DMGlobalToLocalBegin(ctx->daScalCell,ctx->M_inv,INSERT_VALUES,m_inv_local);CHKERRQ(ierr);
    ierr = DMGlobalToLocalEnd(ctx->daScalCell,ctx->M_inv,INSERT_VALUES,m_inv_local);CHKERRQ(ierr);
    ierr = DMDAVecGetArray(ctx->daScalCell,m_inv_local,&m_inv_array);CHKERRQ(ierr);
    for (ek = zs; ek < zs+zm; ek++){
      for (ej = ys; ej < ys+ym; ej++){
        for (ei = xs; ei < xs+xm; ei++){
          m_inv_array[ek][ej][ei] = prop[ctx->layer[ek]];
        }
      }
    }
    ierr = DMDAVecRestoreArray(ctx->daScalCell,m_inv_local,&m_inv_array);CHKERRQ(ierr);
    ierr = DMLocalToGlobalBegin(ctx->daScalCell,m_inv_local,INSERT_VALUES,ctx->M_inv);CHKERRQ(ierr);
    ierr = DMLocalToGlobalEnd(ctx->daScalCell,m_inv_local,INSERT_VALUES,ctx->M_inv);CHKERRQ(ierr);
    ierr = DMRestoreLocalVector(ctx->daScalCell,&m_inv_local);CHKERRQ(ierr);

    flowprop->mu    = 1.;
    ierr = PetscOptionsReal("-mu","\n\tFluid viscosity ","",flowprop->mu,&flowprop->mu,NULL);CHKERRQ(ierr);
    
    flowprop->rho    = 1.;
    ierr = PetscOptionsReal("-rhof","\n\tFluid density ","", flowprop->rho,&flowprop->rho,NULL);CHKERRQ(ierr);
    
    for (i = 0; i < 3; i++) g[i] = 0.;
    nval = 3;
    ierr = PetscOptionsRealArray("-g","\n\t X,Y & Z components of gravity (default 0.), comma separated","",g,&nval,NULL);CHKERRQ(ierr);
    if (nval != 3 && nval != 0) SETERRQ3(PETSC_COMM_WORLD,PETSC_ERR_USER,"ERROR: Expecting 3 values for option %s, got only %i in %s\n","-k",nval,__FUNCT__);
    for (c = 0; c < 3; c++){
      flowprop->g[0]  = g[c];
    }
  }
  ierr = PetscOptionsEnd();CHKERRQ(ierr);
  ierr = PetscFree(kx);CHKERRQ(ierr);
  ierr = PetscFree(ky);CHKERRQ(ierr);
  ierr = PetscFree(kz);CHKERRQ(ierr);
  ierr = PetscFree(prop);CHKERRQ(ierr);
  ierr = PetscFree(g);CHKERRQ(ierr);

  PetscFunctionReturn(0);
}
