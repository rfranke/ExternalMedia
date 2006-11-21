//============================================================================================//
//                                                                                            //
//                               Microsoft Visual C++ 6.0 Client                              //
//                               -------------------------------                              //
//                                                                                            //
//  This is an example of a client application in Microsoft Visual C++ 6.0 for FluidProp,     //
//  a COM server module for the calculation of fluid properties. FluidProp is a common        //
//  interface to GasMix, IF97, Refprop, StanMix, TPSI and is developed by Piero Colonna       //
//  and Teus van der Stelt.                                                                   //
//                                                                                            //
//  The class implemented in this file, TFluidProp, is as a wrapper class for the base        //
//  class IFluidProp_COM. TFluidProp hides specific COM server details like safe arrays       //
//  (SAFEARRAY) and binary strings (BSTR) in IFluidProp_COM. In the TFluidProp class          //
//  only standard C++ data types are used. This seriously facilitates working with the        //
//  COM server.                                                                               //
//                                                                                            //
//  Teus van der Stelt                                                                        //
//  Energy Technology Section                                                                 //
//  TU Delft                                                                                  //
//                                                                                            //
//  July, 2004, for FluidProp 1                                                               //
//  January, 2006, for FluidProp 2                                                            //
//                                                                                            //
//============================================================================================//


#include "FluidProp_IF.h"

#pragma comment(lib, "comsupp.lib")


// {F30D147D-1F7C-4092-B481-ADE326A2ECD5}
//
static const GUID CLSID_FluidProp =
{ 0xF30D147DL, 0x1F7C, 0x4092,
{ 0xB4, 0x81, 0xAD, 0xE3, 0x26, 0xA2, 0xEC, 0xD5 } };

// {2430EE09-2C1E-4A86-AB62-CB67AEF6E484}
//
static const IID IID_IFluidProp =
{ 0x2430EE09L, 0x2C1E, 0x4A86,
{ 0xAB, 0x62, 0xCB, 0x67, 0xAE, 0xF6, 0xE4, 0x84 } };


TFluidProp::TFluidProp()
{
   // Init OLE
   CoInitialize(0);

   // Retrieve class factory interface pointer to our FluidProp COM object
   HRESULT hr = CoGetClassObject( CLSID_FluidProp, CLSCTX_INPROC_SERVER, 0,
                                  IID_IClassFactory, (void**)&ClassFactory);

   // Have class factory make the object for us - then release factory
   if (SUCCEEDED(hr))
   {
      ClassFactory->CreateInstance( 0, IID_IFluidProp, (void**)&FluidProp_COM);
      ClassFactory->Release();
      ClassFactory = 0;
   }
}

TFluidProp::~TFluidProp()
{
   // Free FluidProp object
   FluidProp_COM->Release();

   // Unitialize OLE
   CoUninitialize();
}

void TFluidProp::CreateObject( string ModelName, string* ErrorMsg)
{
   BSTR BSTR_Model = _com_util::ConvertStringToBSTR( ModelName.c_str());
   BSTR BSTR_Error;

   FluidProp_COM->CreateObject( BSTR_Model, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);
}

void TFluidProp::ReleaseObjects()
{
   FluidProp_COM->ReleaseObjects();
}

void TFluidProp::SetFluid( string ModelName, int nComp, string* Comp, double* Conc, string* ErrorMsg)
{
   // _com_util::Convert model name to binary string
   BSTR BSTR_Model = _com_util::ConvertStringToBSTR( ModelName.c_str());

   // _com_util::Convert character array Comp via binary strings (BSTR) into an OLE SafeArray
   SAFEARRAYBOUND sa_bounds_Comp[1];
   sa_bounds_Comp[0].lLbound = 0;
   sa_bounds_Comp[0].cElements = nComp;

   SAFEARRAY FAR* sa_Comp;
   sa_Comp = SafeArrayCreate( VT_BSTR, 1, sa_bounds_Comp);
   for( long i = 0; i < (long)nComp; i++)
   {
      BSTR BSTR_Comp = _com_util::ConvertStringToBSTR( Comp[i].c_str());
      SafeArrayPutElement(sa_Comp, &i, BSTR_Comp);
   }

   // _com_util::Convert the double array Conc into an OLE SafeArray
   SAFEARRAYBOUND sa_bounds_Conc[1];
   sa_bounds_Conc[0].lLbound = 0;
   sa_bounds_Conc[0].cElements = nComp;

   SAFEARRAY FAR* sa_Conc;
   sa_Conc = SafeArrayCreate( VT_R8, 1, sa_bounds_Conc);
   for(long i = 0; i < (long)nComp; i++)
      SafeArrayPutElement(sa_Conc, &i, &Conc[i]);

   // Now load the fluid parameters for the model selected.
   BSTR BSTR_Error;
   FluidProp_COM->SetFluid( BSTR_Model, nComp, &sa_Comp, &sa_Conc, &BSTR_Error);

   // Error handling
   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);
}

void TFluidProp::GetFluid( string* ModelName, int* nComp, string* Comp, double* Conc, bool CompInfo)
{
   // When CompInfo is true, the components and their concentrations in the mixture are returned
   // instead of the mixture name.

   long long_nComp = *nComp;

   // Convert character array Comp via binary strings (BSTR) into an OLE SafeArray
   // This needs to be done because in the COM interface Comp is an inout argument,
   // because for out arguments an assumed array size is not allowed....
   SAFEARRAYBOUND sa_bounds_Comp[1];
   sa_bounds_Comp[0].lLbound = 0;
   sa_bounds_Comp[0].cElements = long_nComp;

   SAFEARRAY FAR* sa_Comp;
   sa_Comp = SafeArrayCreate( VT_BSTR, 1, sa_bounds_Comp);
   for( long i = 0; i < long_nComp; i++)
   {
      BSTR BSTR_Comp = _com_util::ConvertStringToBSTR( Comp[i].c_str());
      SafeArrayPutElement(sa_Comp, &i, BSTR_Comp);
   }

   // Convert the double array Conc into an OLE SafeArray
   // This needs to be done because in the COM interface Conc is an inout argument,
   // because for out arguments an assumed array size is not allowed....
   SAFEARRAYBOUND sa_bounds_Conc[1];
   sa_bounds_Conc[0].lLbound = 0;
   sa_bounds_Conc[0].cElements = long_nComp;

   SAFEARRAY FAR* sa_Conc;
   sa_Conc = SafeArrayCreate( VT_R8, 1, sa_bounds_Conc);
   for(long i = 0; i < long_nComp; i++)
      SafeArrayPutElement(sa_Conc, &i, &Conc[i]);

   // Now retrieve the fluid parameters set wit SetFluid
   BSTR BSTR_Model = _com_util::ConvertStringToBSTR( " ");
   if ( CompInfo)
      long_nComp = -1;
   FluidProp_COM->GetFluid( BSTR_Model, &long_nComp, &sa_Comp, &sa_Conc);

   // Convert model name from binary string to string
   string TmpName = _com_util::ConvertBSTRToString( BSTR_Model);
   *ModelName = TmpName;//ConvertBSTRToString( BSTR_Model);

   // Convert from long to int
   *nComp = long_nComp;

   // Put the values in the string array Comp
   for(long i = 0; i < long_nComp; i++)
   {
      BSTR BSTR_Comp;
      SafeArrayGetElement( sa_Comp, &i, &BSTR_Comp);
      Comp[i] = _com_util::ConvertBSTRToString( BSTR_Comp);
   }

   // Put the values in the double array Conc
   for(long i = 0; i < long_nComp; i++)
      SafeArrayGetElement( sa_Conc, &i, &Conc[i]);

   // Destroy the SafeArrays
   SafeArrayDestroy( sa_Comp);
   SafeArrayDestroy( sa_Conc);
}

void TFluidProp::GetFluidNames( string LongShort, string ModelName, int* nFluids, 
                                string* FluidNames, string* ErrorMsg)
{
   long long_nFluids;

   // Get available fluids
   BSTR BSTR_Model = _com_util::ConvertStringToBSTR( ModelName.c_str());
   BSTR BSTR_LongShort = _com_util::ConvertStringToBSTR( LongShort.c_str());
   BSTR BSTR_Error;

   // Convert character array FluidNames via binary strings (BSTR) into an OLE SafeArray
   // This needs to be done because in the COM interface FluidNames is an inout argument,
   // because Visual Basic is not able to deal out arrays....
   //
   SAFEARRAYBOUND sa_bounds_FluidNames[1];
   sa_bounds_FluidNames[0].lLbound = 0;
   sa_bounds_FluidNames[0].cElements = 250;
   SAFEARRAY* sa_FluidNames;
   sa_FluidNames = SafeArrayCreate( VT_BSTR, 1, sa_bounds_FluidNames);

   FluidProp_COM->GetFluidNames( BSTR_LongShort, BSTR_Model, &long_nFluids, &sa_FluidNames,
                                 &BSTR_Error);

   // Retrieve array with components from SafeArray
   for( long i = 0; i < long_nFluids; i++)
   {
      BSTR BSTR_Fluid;
      SafeArrayGetElement( sa_FluidNames, &i, &BSTR_Fluid);
      FluidNames[i] =  _com_util::ConvertBSTRToString( BSTR_Fluid);
   }
   *nFluids = long_nFluids;

   // Error handling
   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);
}

double TFluidProp::Pressure( string InputSpec, double Input1, double Input2, string* ErrorMsg)
{
   double Output;

   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR(InputSpec.c_str());
   BSTR BSTR_Error;

   FluidProp_COM->Pressure( BSTR_InputSpec, Input1, Input2, &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::Temperature( string InputSpec, double Input1, double Input2, string* ErrorMsg)
{
   double Output;

   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR( InputSpec.c_str());
   BSTR BSTR_Error;

   FluidProp_COM->Temperature( BSTR_InputSpec, Input1, Input2, &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::SpecVolume( string InputSpec, double Input1, double Input2, string* ErrorMsg)
{
   double Output;

   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR( InputSpec.c_str());
   BSTR BSTR_Error;

   FluidProp_COM->SpecVolume( BSTR_InputSpec, Input1, Input2, &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::Density( string InputSpec, double Input1, double Input2, string* ErrorMsg)
{
   double Output;

   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR( InputSpec.c_str());
   BSTR BSTR_Error;

   FluidProp_COM->Density( BSTR_InputSpec, Input1, Input2, &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::Enthalpy( string InputSpec, double Input1, double Input2, string* ErrorMsg)
{
   double Output;

   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR( InputSpec.c_str());
   BSTR BSTR_Error;

   FluidProp_COM->Enthalpy( BSTR_InputSpec, Input1, Input2, &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::Entropy( string InputSpec, double Input1, double Input2, string* ErrorMsg)
{
   double Output;

   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR( InputSpec.c_str());
   BSTR BSTR_Error;

   FluidProp_COM->Entropy( BSTR_InputSpec, Input1, Input2, &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::IntEnergy( string InputSpec, double Input1, double Input2, string* ErrorMsg)
{
   double Output;

   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR( InputSpec.c_str());
   BSTR BSTR_Error;

   FluidProp_COM->IntEnergy( BSTR_InputSpec, Input1, Input2, &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::VaporQual( string InputSpec, double Input1, double Input2, string* ErrorMsg)
{
   double Output;

   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR( InputSpec.c_str());
   BSTR BSTR_Error;

   FluidProp_COM->VaporQual( BSTR_InputSpec, Input1, Input2, &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double* TFluidProp::LiquidCmp( string InputSpec, double Input1, double Input2, string* ErrorMsg)
{
   double* Output = new double[20];

   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR( InputSpec.c_str());
   BSTR BSTR_Error;

   // _com_util::Convert the double array Conc into an OLE SafeArray
   SAFEARRAYBOUND sa_bounds_Output[1];
   sa_bounds_Output[0].lLbound = 0;
   sa_bounds_Output[0].cElements = 20;
   SAFEARRAY* sa_Output;
   sa_Output = SafeArrayCreate( VT_R8, 1, sa_bounds_Output);

   FluidProp_COM->LiquidCmp( BSTR_InputSpec, Input1, Input2, &sa_Output, &BSTR_Error);

   // Retrieve array with concentrations from SafeArray
   for( long i = 0; i < (signed)sa_bounds_Output[0].cElements; i++)
      SafeArrayGetElement( sa_Output, &i, &Output[i]);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double* TFluidProp::VaporCmp( string InputSpec, double Input1, double Input2, string* ErrorMsg)
{
   double* Output = new double[20];

   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR( InputSpec.c_str());
   BSTR BSTR_Error;

   // _com_util::Convert the double array Conc into an OLE SafeArray
   SAFEARRAYBOUND sa_bounds_Output[1];
   sa_bounds_Output[0].lLbound = 0;
   sa_bounds_Output[0].cElements = 20;
   SAFEARRAY* sa_Output;
   sa_Output = SafeArrayCreate( VT_R8, 1, sa_bounds_Output);

   FluidProp_COM->VaporCmp( BSTR_InputSpec, Input1, Input2, &sa_Output, &BSTR_Error);

   // Retrieve array with concentrations from SafeArray
   for( long i = 0; i < (signed)sa_bounds_Output[0].cElements; i++)
      SafeArrayGetElement( sa_Output, &i, &Output[i]);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::HeatCapV( string InputSpec, double Input1, double Input2, string* ErrorMsg)
{
   double Output;

   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR( InputSpec.c_str());
   BSTR BSTR_Error;

   FluidProp_COM->HeatCapV( BSTR_InputSpec, Input1, Input2, &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::HeatCapP( string InputSpec, double Input1, double Input2, string* ErrorMsg)
{
   double Output;

   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR( InputSpec.c_str());
   BSTR BSTR_Error;

   FluidProp_COM->HeatCapP( BSTR_InputSpec, Input1, Input2, &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::SoundSpeed( string InputSpec, double Input1, double Input2, string* ErrorMsg)
{
   double Output;

   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR( InputSpec.c_str());
   BSTR BSTR_Error;

   FluidProp_COM->SoundSpeed( BSTR_InputSpec, Input1, Input2, &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::Alpha( string InputSpec, double Input1, double Input2, string* ErrorMsg)
{
   double Output;

   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR( InputSpec.c_str());
   BSTR BSTR_Error;

   FluidProp_COM->Alpha( BSTR_InputSpec, Input1, Input2, &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::Beta( string InputSpec, double Input1, double Input2, string* ErrorMsg)
{
   double Output;

   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR( InputSpec.c_str());
   BSTR BSTR_Error;

   FluidProp_COM->Beta( BSTR_InputSpec, Input1, Input2, &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::Chi( string InputSpec, double Input1, double Input2, string* ErrorMsg)
{
   double Output;

   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR( InputSpec.c_str());
   BSTR BSTR_Error;

   FluidProp_COM->Chi( BSTR_InputSpec, Input1, Input2, &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::Fi( string InputSpec, double Input1, double Input2, string* ErrorMsg)
{
   double Output;

   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR( InputSpec.c_str());
   BSTR BSTR_Error;

   FluidProp_COM->Fi( BSTR_InputSpec, Input1, Input2, &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::Ksi( string InputSpec, double Input1, double Input2, string* ErrorMsg)
{
   double Output;

   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR( InputSpec.c_str());
   BSTR BSTR_Error;

   FluidProp_COM->Ksi( BSTR_InputSpec, Input1, Input2, &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::Psi( string InputSpec, double Input1, double Input2, string* ErrorMsg)
{
   double Output;

   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR(InputSpec.c_str());
   BSTR BSTR_Error;

   FluidProp_COM->Psi( BSTR_InputSpec, Input1, Input2, &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::Gamma( string InputSpec, double Input1, double Input2, string* ErrorMsg)
{
   double Output;

   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR(InputSpec.c_str());
   BSTR BSTR_Error;

   FluidProp_COM->Gamma( BSTR_InputSpec, Input1, Input2, &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::Viscosity( string InputSpec, double Input1, double Input2, string* ErrorMsg)
{
   double Output;

   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR(InputSpec.c_str());
   BSTR BSTR_Error;

   FluidProp_COM->Viscosity( BSTR_InputSpec, Input1, Input2, &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::ThermCond( string InputSpec, double Input1, double Input2, string* ErrorMsg)
{
   double Output;

   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR(InputSpec.c_str());
   BSTR BSTR_Error;

   FluidProp_COM->ThermCond( BSTR_InputSpec, Input1, Input2, &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

void TFluidProp::AllProps( string InputSpec, double Input1, double Input2, double& P, double& T,
                           double& v, double& d, double& h, double& s, double& u, double& q,
                           double* x, double* y, double& cv, double& cp, double& c, double& alpha,
                           double& beta, double& chi, double& fi, double& ksi, double& psi,
                           double& gamma, double& eta, double& lambda, string* ErrorMsg)
{
   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR(InputSpec.c_str());
   BSTR BSTR_Error;

   // Convert double arrays with liquid and vapor phase compositions x and y into OLE SafeArrays
   SAFEARRAYBOUND sa_bounds_x[1], sa_bounds_y[1];
   sa_bounds_x[0].lLbound = 0;
   sa_bounds_y[0].lLbound = 0;
   sa_bounds_x[0].cElements = 20;
   sa_bounds_y[0].cElements = 20;
   SAFEARRAY *sa_x, *sa_y;
   sa_x = SafeArrayCreate( VT_R8, 1, sa_bounds_x);
   sa_y = SafeArrayCreate( VT_R8, 1, sa_bounds_y);

   FluidProp_COM->AllProps( BSTR_InputSpec, Input1, Input2, &P, &T, &v, &d, &h, &s, &u, &q, &sa_x,
                            &sa_y, &cv, &cp, &c, &alpha, &beta, &chi, &fi, &ksi, &psi, &gamma, &eta,
                            &lambda, &BSTR_Error);

   // Retrieve array with liquid and vapor phase compositions from SafeArrays
   for( long i = 0; i < (signed)sa_bounds_x[0].cElements; i++)
      SafeArrayGetElement( sa_x, &i, &x[i]);

   for( long i = 0; i < (signed)sa_bounds_y[0].cElements; i++)
      SafeArrayGetElement( sa_y, &i, &y[i]);

   // Destroy the SafeArrays
   SafeArrayDestroy( sa_x);
   SafeArrayDestroy( sa_y);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);
}

double TFluidProp::Solve( string FuncSpec, double FuncVal, string InputSpec, long Target,
                          double FixedVal, double MinVal, double MaxVal, string* ErrorMsg)
{
   double Output;

   BSTR BSTR_FuncSpec  = _com_util::ConvertStringToBSTR( FuncSpec.c_str());
   BSTR BSTR_InputSpec = _com_util::ConvertStringToBSTR(InputSpec.c_str());
   BSTR BSTR_Error;

   FluidProp_COM->Solve( BSTR_FuncSpec, FuncVal, BSTR_InputSpec, Target, FixedVal, MinVal,
                         MaxVal, &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::Mmol( string* ErrorMsg)
{
   double Output;

   BSTR BSTR_Error;

   FluidProp_COM->Mmol( &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::Tcrit( string* ErrorMsg)
{
   double Output;

   BSTR BSTR_Error;

   FluidProp_COM->Tcrit( &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::Pcrit( string* ErrorMsg)
{
   double Output;

   BSTR BSTR_Error;

   FluidProp_COM->Pcrit( &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::Tmin( string* ErrorMsg)
{
   double Output;

   BSTR BSTR_Error;

   FluidProp_COM->Tmin( &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

double TFluidProp::Tmax( string* ErrorMsg)
{
   double Output;

   BSTR BSTR_Error;

   FluidProp_COM->Tmax( &Output, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);

   return Output;
}

void TFluidProp::AllInfo( double& Mmol, double& Tcrit, double& Pcrit, double& Tmin, double& Tmax,
                          string* ErrorMsg)
{
   BSTR BSTR_Error;

   FluidProp_COM->AllInfo( &Mmol, &Tcrit, &Pcrit, &Tmin, &Tmax, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);
}

void TFluidProp::SetUnits( string UnitSet, string MassOrMole, string Properties, string Units,
                           string* ErrorMsg)
{
   BSTR BSTR_Error;
   BSTR BSTR_UnitSet    = _com_util::ConvertStringToBSTR( UnitSet.c_str());
   BSTR BSTR_MassOrMole = _com_util::ConvertStringToBSTR( MassOrMole.c_str());
   BSTR BSTR_Properties = _com_util::ConvertStringToBSTR( Properties.c_str());
   BSTR BSTR_Units      = _com_util::ConvertStringToBSTR( Units.c_str());

   FluidProp_COM->SetUnits( BSTR_UnitSet, BSTR_MassOrMole, BSTR_Properties, BSTR_Units, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);
}

void TFluidProp::SetRefState( double T_ref, double P_ref, string* ErrorMsg)
{
   BSTR BSTR_Error;

   FluidProp_COM->SetRefState( T_ref, P_ref, &BSTR_Error);

   *ErrorMsg = _com_util::ConvertBSTRToString( BSTR_Error);
}

//==================================================================================== EOF ===//


