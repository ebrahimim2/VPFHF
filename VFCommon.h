#ifndef VFCOMMON_H
#define VFCOMMON_H
/*
 VFCommon.h
 (c) 2010-2013 Blaise Bourdin bourdin@lsu.edu
 */
static const char banner[] = "\n\nVPFHF: A Variational Phase-Field based Hydro-Fracking simulator.\n"
  "(c) 2010-2018 Blaise Bourdin, Louisiana State University. bourdin@lsu.edu\n\n"
  "# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND\n"
  "# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n"
  "# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\n"
  "# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR\n"
  "# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES\n"
  "# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\n"
  "# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND\n"
  "# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n"
  "# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n"
  "# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n\n"
  "# This software is released under the 2-clause BSD license (aka \"Simplified BSD\" \n"
  "# or \"FreeBSD\") license. See the LICENSE file in the root of the software distribution\n\n"
  "# See the CONTRIBUTORS.txt file for a list of contributors to this software.\n\n";

typedef struct {
	PetscReal   mu;             /* Fluid viscosity              */
	PetscReal   rho;            /* Fluid density                */
	PetscReal   g[3];
	PetscReal   Cp;		    /* Specific heat capacity       */
	PetscReal   Kf;             /* Modulus of liquid            */
} VFFlowProp;


/*
 all fields involved in the computations
 */
typedef struct {
	int numfields;
	Vec V;
	Vec VIrrev;
	Vec U;
	Vec BCU;
	Vec theta;
	Vec thetaRef;
	Vec pressure;
	Vec pmult;
	Vec VelnPress;
	Vec vfperm;
	Vec velocity;
	Vec VolCrackOpening;
	Vec VolLeakOffRate;
	Vec FlowBCArray;
	Vec PresBCArray;
	Vec fracpressure;
	Vec fracvelocity;
	Vec fracVelnPress;
  Vec pressure_old;
	Vec U_old;
	Vec V_old;
  Vec theta_old;
  Vec VelnPress_old;
  Vec velocity_old;
  Vec widthc;
	Vec width;
} VFFields;

/*
All static const are moved into the files where they are used in order to avoid annoying warnings
*/

typedef struct {
	PetscReal       E,nu;       /* Young modulus and poisson ratio */
	PetscReal       lambda,mu;  /* Lame coefficients               */
	PetscReal       alpha;      /* Linear thermal expansion coef.  */
	PetscReal       Gc;         /* Fracture toughness              */
	PetscReal       beta;       /* Biot's constant                 */
	PetscReal       rho;        /* density                         */
	PetscReal       phi;        /* porosity                        */
	PetscReal       Ks;         /* Bulk modulus of rock            */
	PetscReal       Cp;		    /* Specific heat capacity          */
} VFMatProp; //change them to Vec later

typedef struct {
	PetscReal        epsilon;
	PetscReal        eta;
	PetscReal        PCeta;
    PetscInt         atnum;
	PetscReal        atCv;
	PetscReal        irrevtol;
	PetscReal        permmax;
	PetscReal        permmult;
} VFProp;


/*
  enums. The matching field names used in PetscOptions are in VFCommon_private.h
*/
/* 
  This needs to be moved to VFWell.h once the headers are reorganized
*/
typedef enum {
	PRESSURE,
	RATE
} WellConstraint;

typedef enum {
	INJECTOR,
	PRODUCER
} WellType;

typedef enum {
  PENNY,
	RECT
} WellInFrac;

typedef enum {
	FRACTURE,
	ELASTICITY,
	NOMECH
} VFMechSolverType;

typedef enum {
	UNILATERAL_NONE,
	UNILATERAL_NOCOMPRESSION
} VFUnilateralType;

typedef enum {
  FLOWSOLVER_SNESSTANDARDFEM,
	FLOWSOLVER_TS,
	FLOWSOLVER_SNES,
	FLOWSOLVER_FEM,
	FLOWSOLVER_KSPMIXEDFEM,
	FLOWSOLVER_SNESMIXEDFEM,
	FLOWSOLVER_TSMIXEDFEM,
	FLOWSOLVER_FAKE,
	FLOWSOLVER_READFROMFILES,
	FLOWSOLVER_NONE
} VFFlowSolverType;

typedef enum {
	FIXEDSTRAIN,
	FIXEDSTRESS
} ResFlowMechCouplingType;

typedef enum {
	HEATSOLVER_SNESFEM,
} VFHeatSolverType;

typedef enum {
	FILEFORMAT_BIN,
	FILEFORMAT_VTK
} VFFileFormatType;

typedef struct {
	char           name[256];
	PetscReal      top[3];
	PetscReal      bottom[3];
	PetscReal      coords[3];
	PetscReal		   Qw;
	PetscReal		   Pw;
	PetscReal		   rw;
	WellConstraint condition;
	WellType		   type;
  PetscInt       wellfracindex;
  WellInFrac     fractype;
	/*
	 PetscReal    rate;
	 BCTYPE       BCV;
	 */
} VFWell;

typedef struct {
	char          name[256];
	PetscReal     center[3];
	PetscReal     r,phi,theta,thickness;
} VFPennyCrack;

typedef struct {
	char          name[256];
	PetscReal     corners[9];
	PetscReal     thickness;
	/*
	 Corner allocation:
	 
	 [6,7,8]
     +---------------------+
     |                     |
     |                     |
     |                     |
     +---------------------+
	 [0,1,2]               [3,4,5]
	 */
} VFRectangularCrack;

typedef struct {
	PetscReal         perm;     /* Permeability in m^2 muliply by 1e12 */
	PetscReal         por;      /* Porosity */
	PetscReal         Pinit;    /* Initial Pressure in MPa*/
	PetscReal         Tinit;    /* Initial Temperature in C*/
	PetscReal         relk;     /* Relative Permeability */
	PetscReal         visc;     /* Viscosity in cp */
	PetscReal         fdens;    /* Fluid Density in specific density*/
	PetscReal         rock_comp; /* Rock compressibility in 1/MPa */
	PetscReal         wat_comp; /* Water compressibility in 1/MPa */
	PetscReal         TCond_X;  /* Thermal Conductivity in x-direction */
	PetscReal         TCond_Y;  /* Thermal Conductivity in y-direction */
	PetscReal         TCond_Z;  /* Thermal Conductivity in z-direction */
	/*
	 change them to Vec later.
	 Instead, I would suggest keeping the structure this way and add a pointer to a resprop in the main context
	 We can read them in a file later, the difficulty is to initialize the files to something reasonable
	 */
} VFResProp;

typedef struct {
	PetscBool           printhelp;
	PetscInt            nlayer;
	PetscReal          *layersep;
	PetscInt           *layer;         /* dim=nz+1. gives the layer number of a cell  */
	VFBC                bcU[3];
	VFBC                bcV[1];
	VFBC                bcP[1];
	VFBC                bcT[1];
	DM                  daVect;
	DM                  daScal;
	VFCartFEElement3D    e3D;
	VFCartFEElement2D    e2D;
	char                prefix[PETSC_MAX_PATH_LEN];
	Vec                 coordinates;
	PetscInt            verbose;
	SNES                snesV;
	SNES                snesU;
	/*
	 Global variables for regular FEM Flow
	 */
	Mat                 KP; //stifness
	Mat                 KPlhs; // mass matrix
	Mat                 JacP;
	TS                  tsP;
	PC                  pcP;
	KSP                 kspP;
	SNES                snesP;
	Vec                 RHSP;
	Vec                 RHSPpre;
	Vec                 pressure_old;
	Vec                 PFunct;
	Vec                 PresBC;
	PetscReal           CrackVolume;
	PetscReal           LeakOffRate;
	/*
	 Global variables for Mixed Darcy Flow
  */
	Mat                 KVelP;
	PC                  pcVelP;
	KSP                 kspVelP;
	DM                  daFlow;
	DM                  daVFperm;
	VFFlowProp          flowprop;
	Vec                 RHSVelP;
	Vec                 RHSVelPpre;
	Vec                 Source;
	DM                  daScalCell;
	Mat                 KVelPlhs;
	Mat                 JacVelP;
	TS                  tsVelP;
	SNES                snesVelP;
	Vec                 FlowFunct;
	Vec                 PreFlowFields;
	Vec                 Perm;
	Vec                 FlowBC;
	VFBC                bcQ[3];
	PetscBool           hasFlowWells;
	PetscBool           hasFluidSources;
	Vec                 VelBCArray;
	Vec				          PresBCArray;
	
	/*
	 Global Variables for Heat Transfer
  */
	Mat                 KT;
	Mat                 KTlhs;
	PC                  pcT;
	KSP                 kspT;
	Mat                 JacT;
	Vec                 PreHeatFields;
	Vec                 RHST;
	Vec                 HeatFunct;
	Vec                 RHSTpre;
	SNES                snesT;
	Vec                 HeatBC;
	/*
	 SNES solver for Pressure (or flow - T&P)
	 */
	SNES                snesF;
	
	PetscReal           altmintol;
	PetscInt            altminmaxit;
	VFMatProp          *matprop;
	VFResProp           resprop;
	VFProp              vfprop;
	PetscReal           insitumin[6];
	PetscReal           insitumax[6];
	PetscBool           hasInsitu;
	PetscBool           hasCrackPressure;
	PetscReal           BCpres[6];
	PetscReal           BCtheta[6];
	PetscInt            SrcLoc[3];
	PetscReal           SrcRate;
	VFUnilateralType    unilateral;
	VFFlowSolverType    flowsolver;
	VFHeatSolverType    heatsolver;
	VFMechSolverType    mechsolver;
	VFFileFormatType    fileformat;
	PetscViewer         energyviewer;
	PetscViewer         XDMFviewer;
	PetscReal           timevalue;
	PetscReal           current_time;
	PetscReal           dt;
	PetscInt            timestep;
	PetscReal           mintimevalue;
	PetscReal           maxtimevalue;
	PetscInt            maxtimestep;
	PetscInt            maxiterations;
	PetscReal           ElasticEnergy;
	PetscReal           SurfaceEnergy;
	PetscReal           InsituWork;
	PetscReal           PressureWork;
	PetscReal           TotalEnergy;
	PetscInt            numWells;
	VFWell             *well;
	PetscInt            numPennyCracks;
	VFPennyCrack       *pennycrack;
	PetscInt            numRectangularCracks;
	VFRectangularCrack *rectangularcrack;
	Vec                 HeatSource;
	Vec					        HeatFluxBCArray;
	PetscBool           hasHeatSources;
	Vec                 prevT;
	Vec                 Cond;           /*thermal conductivity in ctx*/
	VFBC                bcQT[3];        /*heat flux in heat equation*/
	Vec					        TBCArray;
	Mat                 KFracVelP;
	Mat                 JacFracVelP;
	SNES                snesFracVelP;
	Vec                 FracResidual;
	Vec                 RHSFracVelP;
	Vec                 RHSFracVelPpre;
	Mat                 KFracVelPlhs;
	Vec                 PreFracFlowFields;
  Vec                 FracVelBCArray;
  VFBC                bcFracQ[3];
  Vec                 V;
  Vec                 U_old;
  Vec                 U;
  PetscBool           FlowDisplCoupling;
  ResFlowMechCouplingType ResFlowMechCoupling;
  PetscBool           FractureFlowCoupling;
  Vec                 V_old;
  VFWell              *fracwell;
  PetscInt            numfracWells;
  Vec                 RegFracWellFlowRate;
  DM                  daVectCell;
	VFFields            *fields;
  Mat                 KFFT;
  Vec                 FFTIn;
  Vec                 FFTOut;
  CartFEElement3D     s3D;
	CartFEElement2D     s2D;
	CartFEElement1D     s1D;
  VFCartFEElement1D    e1D;
  Vec widthc_old;
  DM                  daWScalCell;
  PetscReal           WidthIntLenght;
  DM                  daWScal;
  DM                  daWVect;
  PetscReal           theta;          /* Time descritization parameter               */
  Vec                 M_inv;
  Vec                 K_dr;
  PetscReal           pmult_vtol;          /* v threshold for pmult              */
  PetscReal           width_tol;          /* tolerance for tip removal              */
  PetscBool           removeTipEffect;
} VFCtx;

extern PetscErrorCode VFCtxGet(VFCtx *ctx);
extern PetscErrorCode VFInitialize(VFCtx *ctx,VFFields *fields);
extern PetscErrorCode VFGeometryInitialize(VFCtx *ctx);
extern PetscErrorCode VFBCInitialize(VFCtx *ctx);

extern PetscErrorCode VFFieldsInitialize(VFCtx *ctx,VFFields *fields);
extern PetscErrorCode VFSolversInitialize(VFCtx *ctx);
extern PetscErrorCode VFTimeStepPrepare(VFCtx *ctx,VFFields *fields);
extern PetscErrorCode VFElasticityTimeStep(VFCtx *ctx,VFFields *fields);
extern PetscErrorCode VFFractureTimeStep(VFCtx *ctx,VFFields *fields);
extern PetscErrorCode VFFinalize(VFCtx *ctx,VFFields *fields);

extern PetscErrorCode VFPropGet(VFProp *vfprop);
extern PetscErrorCode VFMatPropGet(VFMatProp *matprop,PetscInt n);
extern PetscErrorCode VFLayerInit(VFCtx *ctx);
extern PetscErrorCode VFResPropGet(VFResProp *resprop);

extern PetscErrorCode VecViewVTKDof(DM da,Vec X,PetscViewer viewer);
extern PetscErrorCode FieldsH5Write(VFCtx *ctx,VFFields *fields);
extern PetscErrorCode FieldsBinaryWrite(VFCtx *ctx,VFFields *fields);
extern PetscErrorCode FieldsVTKWrite(VFCtx *ctx,VFFields *fields,const char nodalName[], const char cellName[]);

extern PetscErrorCode PermUpdate(Vec V,Vec Pmult,VFProp *vfprop,VFCtx *ctx);

#endif /* VFCOMMON_H */
