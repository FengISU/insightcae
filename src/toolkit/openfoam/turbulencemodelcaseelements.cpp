/*
 * This file is part of Insight CAE, a workbench for Computer-Aided Engineering 
 * Copyright (C) 2014  Hannes Kroeger <hannes@kroegeronline.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */


#include "turbulencemodelcaseelements.h"
#include "openfoam/openfoamcase.h"
#include "openfoam/openfoamtools.h"

#include <utility>
#include "boost/assign.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/format.hpp"

using namespace std;
using namespace boost;
using namespace boost::filesystem;
using namespace boost::assign;
using namespace boost::fusion;

namespace insight
{



defineType(turbulenceModel);
defineFactoryTable(turbulenceModel, LIST(OpenFOAMCase& ofc), LIST(ofc));

turbulenceModel::turbulenceModel(OpenFOAMCase& c)
: OpenFOAMCaseElement(c, "turbulenceModel")
{
}



defineType(RASModel);

RASModel::RASModel(OpenFOAMCase& c)
: turbulenceModel(c)
{
}


void RASModel::addIntoDictionaries(OFdicts& dictionaries) const
{
  OFDictData::dict& turbProperties=dictionaries.addDictionaryIfNonexistent("constant/turbulenceProperties");
  turbProperties["simulationType"]="RASModel";
}

turbulenceModel::AccuracyRequirement RASModel::minAccuracyRequirement() const
{
  return AC_RANS;
}

defineType(LESModel);

LESModel::LESModel(OpenFOAMCase& c)
: turbulenceModel(c)
{
}


void LESModel::addIntoDictionaries(OFdicts& dictionaries) const
{
    OFDictData::dict& turbProperties=dictionaries.addDictionaryIfNonexistent("constant/turbulenceProperties");
    turbProperties["simulationType"]="LESModel";

    OFDictData::dict& controlDict=dictionaries.lookupDict("system/controlDict");
    controlDict.getList("libs").insertNoDuplicate( "\"libnuSgsABLRoughWallFunction.so\"" );
}

bool LESModel::addIntoFieldDictionary(const std::string& fieldname, const FieldInfo& fieldinfo, OFDictData::dict& BC, double roughness_z0) const
{
    if (fieldname == "k")
    {
        BC["type"]="fixedValue";
        BC["value"]="uniform 1e-10";
        return true;
    }
    else if (fieldname == "nuSgs")
    {
        if (roughness_z0>0.)
        {            
//             std::cout<<"inserting \"nuSgsABLRoughWallFunction\" with z0="<<roughness_z0<<std::endl;
            BC["type"]="nuSgsABLRoughWallFunction";
            BC["z0"]=boost::str(boost::format("uniform %g")%roughness_z0);
            BC["value"]="uniform 1e-10";
        }
        else
        {
//             std::cout<<"not inserting since z0="<<roughness_z0<<std::endl;
            BC["type"]="zeroGradient";
        }
        return true;
    }

    return false;
}

turbulenceModel::AccuracyRequirement LESModel::minAccuracyRequirement() const
{
  return AC_LES;
}

defineType(laminar_RASModel);
addToFactoryTable(turbulenceModel, laminar_RASModel);


addToFactoryTable(OpenFOAMCaseElement, laminar_RASModel);
addToStaticFunctionTable(OpenFOAMCaseElement, laminar_RASModel, defaultParameters);

laminar_RASModel::laminar_RASModel(OpenFOAMCase& c, const ParameterSet& ps)
: RASModel(c)
{}

  
void laminar_RASModel::addIntoDictionaries(OFdicts& dictionaries) const
{
  RASModel::addIntoDictionaries(dictionaries);
  
  OFDictData::dict& RASProperties=dictionaries.addDictionaryIfNonexistent("constant/RASProperties");
  RASProperties["RASModel"]="laminar";
  RASProperties["turbulence"]="true";
  RASProperties["printCoeffs"]="true";
  RASProperties.addSubDictIfNonexistent("laminarCoeffs");
}

bool laminar_RASModel::addIntoFieldDictionary(const std::string&, const FieldInfo&, OFDictData::dict&, double) const
{
  return false;
}

defineType(oneEqEddy_LESModel);
addToFactoryTable(turbulenceModel, oneEqEddy_LESModel);

addToFactoryTable(OpenFOAMCaseElement, oneEqEddy_LESModel);
addToStaticFunctionTable(OpenFOAMCaseElement, oneEqEddy_LESModel, defaultParameters);

void oneEqEddy_LESModel::addFields()
{
  OFcase().addField("k", 	FieldInfo(scalarField, 	dimKinEnergy, 	list_of(1e-10), volField ) );
  OFcase().addField("nuSgs", 	FieldInfo(scalarField, 	dimKinViscosity, 	list_of(1e-10), volField ) );
}
  

oneEqEddy_LESModel::oneEqEddy_LESModel(OpenFOAMCase& c, const ParameterSet& ps)
: LESModel(c)
{
  addFields();
}


void oneEqEddy_LESModel::addIntoDictionaries(OFdicts& dictionaries) const
{
  LESModel::addIntoDictionaries(dictionaries);
  
  OFDictData::dict& LESProperties=dictionaries.addDictionaryIfNonexistent("constant/LESProperties");
  LESProperties["printCoeffs"]=true;

  LESProperties["LESModel"]="oneEqEddy";
  //LESProperties["delta"]="cubeRootVol";
  LESProperties["delta"]="vanDriest";
  
  OFDictData::dict crvc;
  crvc["deltaCoeff"]=1.0;
  LESProperties["cubeRootVolCoeffs"]=crvc;
  
  OFDictData::dict vdc;
  vdc["deltaCoeff"]=1.0;
  vdc["delta"]="cubeRootVol";
  vdc["cubeRootVolCoeffs"]=crvc;
  LESProperties["vanDriestCoeffs"]=vdc;
  
  LESProperties.addSubDictIfNonexistent("laminarCoeffs");
}


defineType(dynOneEqEddy_LESModel);
addToFactoryTable(turbulenceModel, dynOneEqEddy_LESModel);

addToFactoryTable(OpenFOAMCaseElement, dynOneEqEddy_LESModel);
addToStaticFunctionTable(OpenFOAMCaseElement, dynOneEqEddy_LESModel, defaultParameters);

void dynOneEqEddy_LESModel::addFields()
{
  OFcase().addField("k", 	FieldInfo(scalarField, 	dimKinEnergy, 	list_of(1e-10), volField ) );
  OFcase().addField("nuSgs", 	FieldInfo(scalarField, 	dimKinViscosity, 	list_of(1e-10), volField ) );
}
  

dynOneEqEddy_LESModel::dynOneEqEddy_LESModel(OpenFOAMCase& c, const ParameterSet& ps)
: LESModel(c)
{
  addFields();
}


void dynOneEqEddy_LESModel::addIntoDictionaries(OFdicts& dictionaries) const
{
  LESModel::addIntoDictionaries(dictionaries);
  
  OFDictData::dict& LESProperties=dictionaries.addDictionaryIfNonexistent("constant/LESProperties");
  LESProperties["printCoeffs"]=true;

  LESProperties["LESModel"]="dynOneEqEddy";
  LESProperties["delta"]="cubeRootVol";
//   LESProperties["delta"]="vanDriest";
  
  OFDictData::dict doeec;
  doeec["filter"]="simple";
  LESProperties["dynOneEqEddyCoeffs"]=doeec;
  
  OFDictData::dict crvc;
  crvc["deltaCoeff"]=1.0;
  LESProperties["cubeRootVolCoeffs"]=crvc;
  
  OFDictData::dict vdc;
  vdc["deltaCoeff"]=1.0;
  vdc["delta"]="cubeRootVol";
  vdc["cubeRootVolCoeffs"]=crvc;
  LESProperties["vanDriestCoeffs"]=vdc;
  
  LESProperties.addSubDictIfNonexistent("laminarCoeffs");
}


defineType(dynSmagorinsky_LESModel);
addToFactoryTable(turbulenceModel, dynSmagorinsky_LESModel);

addToFactoryTable(OpenFOAMCaseElement, dynSmagorinsky_LESModel);
addToStaticFunctionTable(OpenFOAMCaseElement, dynSmagorinsky_LESModel, defaultParameters);

void dynSmagorinsky_LESModel::addFields()
{
  OFcase().addField("k", 	FieldInfo(scalarField, 	dimKinEnergy, 	list_of(1e-10), volField ) );
  OFcase().addField("nuSgs", 	FieldInfo(scalarField, 	dimKinViscosity, 	list_of(1e-10), volField ) );
}

dynSmagorinsky_LESModel::dynSmagorinsky_LESModel(OpenFOAMCase& c, const ParameterSet& ps)
: LESModel(c)
{
  addFields();
}



void dynSmagorinsky_LESModel::addIntoDictionaries(OFdicts& dictionaries) const
{
  LESModel::addIntoDictionaries(dictionaries);
  
  OFDictData::dict& LESProperties=dictionaries.addDictionaryIfNonexistent("constant/LESProperties");
  
  string modelName="dynSmagorinsky";
  if (OFversion()>160)
    modelName="homogeneousDynSmagorinsky";
  
  LESProperties["LESModel"]=modelName;
  //LESProperties["delta"]="cubeRootVol";
  LESProperties["delta"]="vanDriest";
  LESProperties["printCoeffs"]=true;
  
  OFDictData::dict crvc;
  crvc["deltaCoeff"]=1.0;
  LESProperties["cubeRootVolCoeffs"]=crvc;

  OFDictData::dict vdc;
  vdc["deltaCoeff"]=1.0;
  vdc["delta"]="cubeRootVol";
  vdc["cubeRootVolCoeffs"]=crvc;
  LESProperties["vanDriestCoeffs"]=vdc;

  OFDictData::dict& cd=LESProperties.addSubDictIfNonexistent(modelName+"Coeffs");
  cd["filter"]="simple";
}



defineType(kOmegaSST_RASModel);
addToFactoryTable(turbulenceModel, kOmegaSST_RASModel);

addToFactoryTable(OpenFOAMCaseElement, kOmegaSST_RASModel);
addToStaticFunctionTable(OpenFOAMCaseElement, kOmegaSST_RASModel, defaultParameters);

void kOmegaSST_RASModel::addFields()
{
  OFcase().addField("k", 	FieldInfo(scalarField, 	dimKinEnergy, 	list_of(1e-10), volField ) );
  OFcase().addField("omega", 	FieldInfo(scalarField, 	OFDictData::dimension(0, 0, -1), 	list_of(1.0), volField ) );
  if (OFcase().isCompressible())
  {
    OFcase().addField("mut", 	FieldInfo(scalarField, 	dimDynViscosity, 	list_of(1e-10), volField ) );
    OFcase().addField("alphat", 	FieldInfo(scalarField, 	dimDynViscosity, 	list_of(1e-10), volField ) );
  }
  else
  {
    OFcase().addField("nut", 	FieldInfo(scalarField, 	dimKinViscosity, 	list_of(1e-10), volField ) );
  }
}

kOmegaSST_RASModel::kOmegaSST_RASModel(OpenFOAMCase& c, const ParameterSet& ps)
: RASModel(c)
{
  addFields();
}

  
void kOmegaSST_RASModel::addIntoDictionaries(OFdicts& dictionaries) const
{
  RASModel::addIntoDictionaries(dictionaries);

  OFDictData::dict& RASProperties=dictionaries.addDictionaryIfNonexistent("constant/RASProperties");
  RASProperties["RASModel"]="kOmegaSST";
  RASProperties["turbulence"]="true";
  RASProperties["printCoeffs"]="true";
  RASProperties.addSubDictIfNonexistent("kOmegaSSTCoeffs");
}

bool kOmegaSST_RASModel::addIntoFieldDictionary(const std::string& fieldname, const FieldInfo& fieldinfo, OFDictData::dict& BC, double roughness_z0) const
{
  std::string pref="";
  if (OFcase().isCompressible()) pref="compressible::";
  
  if (fieldname == "k")
  {
    BC["type"]=OFDictData::data(pref+"kqRWallFunction");
    BC["value"]=OFDictData::data("uniform 1e-10");
    return true;
  }
  else if (fieldname == "omega")
  {
    BC["type"]=OFDictData::data(pref+"omegaWallFunction");
    BC["Cmu"]=0.09;
    BC["kappa"]=0.41;
    BC["E"]=9.8;
    BC["beta1"]=0.075;
    BC["value"]="uniform 1";
    return true;
  }
  else if (fieldname == "nut")
  {
      if (roughness_z0>0.)
      {
          BC["type"]="nutURoughWallFunction";
          double Cs=0.5;
          BC["roughnessConstant"]=Cs;
          BC["roughnessHeight"]=roughness_z0*9.793/Cs;
          BC["roughnessFactor"]=1.0;
          BC["value"]="uniform 1e-10";
      }
      else
      {
        BC["type"]=OFDictData::data("nutUWallFunction");
        BC["value"]=OFDictData::data("uniform 1e-10");
      }
    return true;
  }
  else if (fieldname == "mut")
  {
    BC["type"]=OFDictData::data("mutkWallFunction");
    BC["value"]=OFDictData::data("uniform 1e-10");
    return true;
  }
  else if (fieldname == "alphat")
  {
    BC["type"]=OFDictData::data(pref+"alphatWallFunction");
    BC["value"]=OFDictData::data("uniform 1e-10");
    return true;
  }
  
  return false;
}

void kEpsilonBase_RASModel::addFields()
{
  OFcase().addField("k", 	FieldInfo(scalarField, 	dimKinEnergy, 	list_of(1e-10), volField ) );
  OFcase().addField("epsilon", 	FieldInfo(scalarField, 	OFDictData::dimension(0, 2, -3), 	list_of(10.0), volField ) );
  if (OFcase().isCompressible())
  {
    OFcase().addField("mut", 	FieldInfo(scalarField, 	dimDynViscosity, 	list_of(1e-10), volField ) );
    OFcase().addField("alphat", 	FieldInfo(scalarField, 	dimDynViscosity, 	list_of(1e-10), volField ) );
  }
  else
  {
    OFcase().addField("nut", 	FieldInfo(scalarField, 	dimKinViscosity, 	list_of(1e-10), volField ) );
  }
}

kEpsilonBase_RASModel::kEpsilonBase_RASModel(OpenFOAMCase& c)
: RASModel(c)
{
  addFields();
}

  
void kEpsilonBase_RASModel::addIntoDictionaries(OFdicts& dictionaries) const
{
  RASModel::addIntoDictionaries(dictionaries);

  OFDictData::dict& RASProperties=dictionaries.addDictionaryIfNonexistent("constant/RASProperties");
  RASProperties["RASModel"]=this->type(); //"kEpsilon";
  RASProperties["turbulence"]="true";
  RASProperties["printCoeffs"]="true";
  RASProperties["kMin"]=1e-3;
  RASProperties["epsilonMin"]=1e-3;
  RASProperties.addSubDictIfNonexistent(type()+"Coeffs");
}

bool kEpsilonBase_RASModel::addIntoFieldDictionary(const std::string& fieldname, const FieldInfo& fieldinfo, OFDictData::dict& BC, double roughness_z0) const
{
    std::string pref="";
    if (OFcase().isCompressible()) pref="compressible::";

    if (fieldname == "k")
    {
        BC["type"]=OFDictData::data(pref+"kqRWallFunction");
        BC["value"]=OFDictData::data("uniform 1e-10");
        return true;
    }
    else if (fieldname == "epsilon")
    {
        BC["type"]=OFDictData::data(pref+"epsilonWallFunction");
        BC["value"]="uniform 10";
        return true;
    }
    else if (fieldname == "nut")
    {
        if (roughness_z0>0)
        {
            BC["type"]="nutkRoughWallFunction";
            double Cs=0.5;
            BC["Cs"]=Cs;
            BC["Ks"]=roughness_z0*9.793/Cs;
            BC["value"]="uniform 1e-10";
        }
        else
        {
            BC["type"]=OFDictData::data("nutkWallFunction");
            BC["value"]=OFDictData::data("uniform 1e-10");
        }
        return true;
    }
    else if (fieldname == "mut")
    {
        BC["type"]=OFDictData::data("mutkWallFunction");
        BC["value"]=OFDictData::data("uniform 1e-10");
        return true;
    }
    else if (fieldname == "alphat")
    {
        BC["type"]=OFDictData::data(pref+"alphatWallFunction");
        BC["value"]=OFDictData::data("uniform 1e-10");
        return true;
    }

    return false;
}


defineType(kEpsilon_RASModel);
addToFactoryTable(turbulenceModel, kEpsilon_RASModel);
addToFactoryTable(OpenFOAMCaseElement, kEpsilon_RASModel);
addToStaticFunctionTable(OpenFOAMCaseElement, kEpsilon_RASModel, defaultParameters);
kEpsilon_RASModel::kEpsilon_RASModel(OpenFOAMCase& c, const ParameterSet& ps): kEpsilonBase_RASModel(c) {}


defineType(realizablekEpsilon_RASModel);
addToFactoryTable(turbulenceModel, realizablekEpsilon_RASModel);
addToFactoryTable(OpenFOAMCaseElement, realizablekEpsilon_RASModel);
addToStaticFunctionTable(OpenFOAMCaseElement, realizablekEpsilon_RASModel, defaultParameters);
realizablekEpsilon_RASModel::realizablekEpsilon_RASModel(OpenFOAMCase& c, const ParameterSet& ps): kEpsilonBase_RASModel(c) {}


defineType(SpalartAllmaras_RASModel);
addToFactoryTable(turbulenceModel, SpalartAllmaras_RASModel);
addToFactoryTable(OpenFOAMCaseElement, SpalartAllmaras_RASModel);
addToStaticFunctionTable(OpenFOAMCaseElement, SpalartAllmaras_RASModel, defaultParameters);

void SpalartAllmaras_RASModel::addFields()
{
  OFcase().addField("nuTilda", 	FieldInfo(scalarField, 	dimKinViscosity, 	list_of(1e-10), volField ) );
  if (OFcase().isCompressible())
  {
    OFcase().addField("mut", 	FieldInfo(scalarField, 	dimDynViscosity, 	list_of(1e-10), volField ) );
    OFcase().addField("alphat", 	FieldInfo(scalarField, 	dimDynViscosity, 	list_of(1e-10), volField ) );
  }
  else
  {
    OFcase().addField("nut", 	FieldInfo(scalarField, 	dimKinViscosity, 	list_of(1e-10), volField ) );
  }
}

SpalartAllmaras_RASModel::SpalartAllmaras_RASModel(OpenFOAMCase& c, const ParameterSet& ps)
: RASModel(c)
{
  addFields();
}

  
void SpalartAllmaras_RASModel::addIntoDictionaries(OFdicts& dictionaries) const
{
  RASModel::addIntoDictionaries(dictionaries);

  OFDictData::dict& RASProperties=dictionaries.addDictionaryIfNonexistent("constant/RASProperties");
  RASProperties["RASModel"]="SpalartAllmaras";
  RASProperties["turbulence"]="true";
  RASProperties["printCoeffs"]="true";
  RASProperties.addSubDictIfNonexistent("SpalartAllmarasCoeffs");
}

bool SpalartAllmaras_RASModel::addIntoFieldDictionary(const std::string& fieldname, const FieldInfo& fieldinfo, OFDictData::dict& BC, double roughness_z0) const
{
    if (roughness_z0>0.)
        throw insight::Exception("SpalartAllmaras_RASModel: non-smooth walls are not supported!");
    
//   std::string pref="";
//   if (OFcase().isCompressible()) pref="compressible::";
  
  if (fieldname == "nuTilda")
  {
    BC["type"]=OFDictData::data("fixedValue");
    BC["value"]=OFDictData::data("uniform 0");
    return true;
  }
  else if (fieldname == "nut")
  {
    BC["type"]=OFDictData::data("nutUSpaldingWallFunction");
    BC["value"]=OFDictData::data("uniform 0");
    return true;
  }
  
  return false;
}

defineType(LEMOSHybrid_RASModel);
addToFactoryTable(turbulenceModel, LEMOSHybrid_RASModel);
addToFactoryTable(OpenFOAMCaseElement, LEMOSHybrid_RASModel);
addToStaticFunctionTable(OpenFOAMCaseElement, LEMOSHybrid_RASModel, defaultParameters);

void LEMOSHybrid_RASModel::addFields()
{
  OFcase().addField("kSgs", 	FieldInfo(scalarField, 	dimKinEnergy, 	list_of(1e-10), volField ) );
  OFcase().addField("nuSgs", 	FieldInfo(scalarField, 	dimKinViscosity, 	list_of(1e-10), volField ) );
  OFcase().addField("UAvgHyb", 	FieldInfo(vectorField, 	dimVelocity, 	list_of(0)(0)(0), volField ) );
}

LEMOSHybrid_RASModel::LEMOSHybrid_RASModel(OpenFOAMCase& c, const ParameterSet& ps)
: kOmegaSST_RASModel(c)
{
  addFields();
}



void LEMOSHybrid_RASModel::addIntoDictionaries(OFdicts& dictionaries) const
{
  // add k-O stuff first, we will overwrite afterwards, where necessary
  kOmegaSST_RASModel::addIntoDictionaries(dictionaries);
  
  OFDictData::dict& RASProperties=dictionaries.addDictionaryIfNonexistent("constant/RASProperties");

  string modelName="hybKOmegaSST2";

  if (OFversion()<230)
    throw insight::Exception("The LES model "+modelName+" is unsupported in the selected OF version!");
    
  RASProperties["RASModel"]=modelName;
  RASProperties["delta"]="maxEdge";
  RASProperties["printCoeffs"]=true;
  
  OFDictData::dict mec;
  mec["deltaCoeff"]=1.0;
  RASProperties["maxEdgeCoeffs"]=mec;
  
  OFDictData::dict& cd=RASProperties.addSubDictIfNonexistent(modelName+"Coeffs");
  cd["filter"]="simple";
  cd["x1"]=1.0;
  cd["x2"]=2.0;
  cd["Cint"]=1.0;
  cd["CN"]=1.0;

  cd["averagingTime"]=1;
  cd["fixedInterface"]=false;
  cd["useIDDESDelta"]=false;

  cd["delta"]="maxEdge";

  cd["cubeRootVolCoeffs"]=mec;
  cd["IDDESDeltaCoeffs"]=mec;
  cd["maxEdgeCoeffs"]=mec;

  OFDictData::dict& controlDict=dictionaries.addDictionaryIfNonexistent("system/controlDict");
  controlDict.getList("libs").push_back( OFDictData::data("\"libLEMOS-2.3.x.so\"") );  
}

bool LEMOSHybrid_RASModel::addIntoFieldDictionary(const std::string& fieldname, const FieldInfo& fieldinfo, OFDictData::dict& BC, double roughness_z0) const
{
  if (!kOmegaSST_RASModel::addIntoFieldDictionary(fieldname, fieldinfo, BC, roughness_z0))
  {
    if (fieldname == "kSgs")
    {
      BC["type"]="fixedValue";
      BC["value"]="uniform 0";
      return true;
    }
    else if (fieldname == "nuSgs")
    {
      BC["type"]="zeroGradient";
      return true;
    }
  }
  
  return false;
}

defineType(kOmegaSST_LowRe_RASModel);
addToFactoryTable(turbulenceModel, kOmegaSST_LowRe_RASModel);
addToFactoryTable(OpenFOAMCaseElement, kOmegaSST_LowRe_RASModel);
addToStaticFunctionTable(OpenFOAMCaseElement, kOmegaSST_LowRe_RASModel, defaultParameters);

kOmegaSST_LowRe_RASModel::kOmegaSST_LowRe_RASModel(OpenFOAMCase& c, const ParameterSet& ps)
: kOmegaSST_RASModel(c)
{}

  
void kOmegaSST_LowRe_RASModel::addIntoDictionaries(OFdicts& dictionaries) const
{
  RASModel::addIntoDictionaries(dictionaries);

  OFDictData::dict& RASProperties=dictionaries.addDictionaryIfNonexistent("constant/RASProperties");
  RASProperties["RASModel"]="kOmegaSST_LowRe";
  RASProperties["turbulence"]="true";
  RASProperties["printCoeffs"]="true";
  RASProperties.addSubDictIfNonexistent("kOmegaSST_LowReCoeffs");
}

bool kOmegaSST_LowRe_RASModel::addIntoFieldDictionary(const std::string& fieldname, const FieldInfo& fieldinfo, OFDictData::dict& BC, double roughness_z0) const
{
    if (roughness_z0>0.)
        throw insight::Exception("kOmegaSST_LowRe_RASModel: non-smooth walls are not supported!");
    
    if (fieldname == "k")
  {
    BC["type"]=OFDictData::data("fixedValue");
    BC["value"]="uniform "+str(format("%g") % 1e-10);
    return true;
  }
  else if ( fieldname == "omega")
  {
    BC["type"]=OFDictData::data("omegaWallFunction");
    BC["value"]="uniform "+str(format("%g") % 1e-10);
    return true;
  }
  return false;
}

defineType(kOmegaSST2_RASModel);
addToFactoryTable(turbulenceModel, kOmegaSST2_RASModel);
addToFactoryTable(OpenFOAMCaseElement, kOmegaSST2_RASModel);
addToStaticFunctionTable(OpenFOAMCaseElement, kOmegaSST2_RASModel, defaultParameters);

void kOmegaSST2_RASModel::addFields()
{
  OFcase().addField("nut", 	FieldInfo(scalarField, 	dimKinViscosity, 	list_of(1e-10), volField ) );
}

kOmegaSST2_RASModel::kOmegaSST2_RASModel(OpenFOAMCase& c, const ParameterSet& ps)
: kOmegaSST_RASModel(c)
{
  kOmegaSST2_RASModel::addFields();
}

  
void kOmegaSST2_RASModel::addIntoDictionaries(OFdicts& dictionaries) const
{
  RASModel::addIntoDictionaries(dictionaries);

  OFDictData::dict& RASProperties=dictionaries.addDictionaryIfNonexistent("constant/RASProperties");
  RASProperties["RASModel"]="kOmegaSST2";
  RASProperties["turbulence"]="true";
  RASProperties["printCoeffs"]="true";
  RASProperties.addSubDictIfNonexistent("kOmegaSST2");
  
  OFDictData::dict& controlDict=dictionaries.addDictionaryIfNonexistent("system/controlDict");
  controlDict.getList("libs").push_back( OFDictData::data("\"libkOmegaSST2.so\"") );
}

bool kOmegaSST2_RASModel::addIntoFieldDictionary(const std::string& fieldname, const FieldInfo& fieldinfo, OFDictData::dict& BC, double roughness_z0) const
{
    if (roughness_z0>0.)
        throw insight::Exception("kOmegaSST2_RASModel: non-smooth walls are not supported!");
    
    if (fieldname == "k")
  {
    BC["type"]="kqRWallFunction";
    BC["value"]="uniform 1e-10";
    return true;
  }
  else if (fieldname == "omega")
  {
    BC["type"]="hybridOmegaWallFunction2";
    BC["Cmu"]=0.09;
    BC["kappa"]=0.41;
    BC["E"]=9.8;
    BC["tw"]=0.057;
    BC["value"]="uniform 1";
    return true;
  }
  else if (fieldname == "nut")
  {
    BC["type"]="nutHybridWallFunction2";
    BC["Cmu"]=0.09;
    BC["kappa"]=0.41;
    BC["E"]=9.8;
    BC["value"]="uniform 1e-10";
    return true;
  }
  return false;
}

defineType(kOmegaHe_RASModel);
addToFactoryTable(turbulenceModel, kOmegaHe_RASModel);
addToFactoryTable(OpenFOAMCaseElement, kOmegaHe_RASModel);
addToStaticFunctionTable(OpenFOAMCaseElement, kOmegaHe_RASModel, defaultParameters);

kOmegaHe_RASModel::kOmegaHe_RASModel(OpenFOAMCase& c, const ParameterSet& ps)
: kOmegaSST_RASModel(c)
{}

  
void kOmegaHe_RASModel::addIntoDictionaries(OFdicts& dictionaries) const
{
  RASModel::addIntoDictionaries(dictionaries);

  OFDictData::dict& controlDict=dictionaries.addDictionaryIfNonexistent("system/controlDict");
  controlDict.getList("libs").push_back( OFDictData::data("\"libkOmegaHe.so\"") );

  OFDictData::dict& RASProperties=dictionaries.addDictionaryIfNonexistent("constant/RASProperties");
  RASProperties["RASModel"]="kOmegaHe";
  RASProperties["turbulence"]="true";
  RASProperties["printCoeffs"]="true";
  RASProperties.addSubDictIfNonexistent("kOmegaHeCoeffs");

  OFDictData::dict& fvSchemes=dictionaries.lookupDict("system/fvSchemes");
  OFDictData::dict& div=fvSchemes.subDict("divSchemes");
  div["div(nonlinear)"]="Gauss linear"; //pref+"Gauss upwind";
  div["div((nuEff*T(grad(U))))"]="Gauss linear";

}

bool kOmegaHe_RASModel::addIntoFieldDictionary(const std::string& fieldname, const FieldInfo& fieldinfo, OFDictData::dict& BC, double roughness_z0) const
{
    if (roughness_z0>0.)
        throw insight::Exception("kOmegaHe_RASModel: non-smooth walls are not supported!");
    
  if (fieldname == "k")
  {
    BC["type"]=OFDictData::data("fixedValue");
    BC["value"]="uniform "+str(format("%g") % 1e-10);
    return true;
  }
  else if ( fieldname == "omega")
  {
    BC["type"]=OFDictData::data("fixedValue");
    BC["value"]="uniform "+str(format("%g") % 1e-10);
    return true;
  }
  else if ( fieldname == "nut")
  {
    BC["type"]=OFDictData::data("calculated");
    BC["value"]="uniform "+str(format("%g") % 1e-10);
    return true;
  }
  return false;
}

defineType(LRR_RASModel);
addToFactoryTable(turbulenceModel, LRR_RASModel);
addToFactoryTable(OpenFOAMCaseElement, LRR_RASModel);
addToStaticFunctionTable(OpenFOAMCaseElement, LRR_RASModel, defaultParameters);

void LRR_RASModel::addFields()
{
  OFcase().addField("nut", 	FieldInfo(scalarField, 	dimKinViscosity, 			list_of(1e-10), volField ) );
  OFcase().addField("k", 	FieldInfo(scalarField, 	dimKinEnergy, 				list_of(1e-10), volField ) );
  OFcase().addField("epsilon", 	FieldInfo(scalarField, 	OFDictData::dimension(0, 2, -3), 	list_of(10.0), volField ) );
  OFcase().addField("R", 	FieldInfo(symmTensorField, OFDictData::dimension(0, 2, -2), 	list_of(1e-10)(1e-10)(1e-10)(1e-10)(1e-10)(1e-10), volField ) );
}

LRR_RASModel::LRR_RASModel(OpenFOAMCase& c, const ParameterSet& ps)
: RASModel(c)
{
  addFields();
}

  
void LRR_RASModel::addIntoDictionaries(OFdicts& dictionaries) const
{
  RASModel::addIntoDictionaries(dictionaries);

  OFDictData::dict& RASProperties=dictionaries.addDictionaryIfNonexistent("constant/RASProperties");
  RASProperties["RASModel"]="LRR";
  RASProperties["turbulence"]="true";
  RASProperties["printCoeffs"]="true";
  RASProperties.addSubDictIfNonexistent("LRRCoeffs");
}

bool LRR_RASModel::addIntoFieldDictionary(const std::string& fieldname, const FieldInfo& fieldinfo, OFDictData::dict& BC, double roughness_z0) const
{

//   std::string pref="";
//   if (OFcase().isCompressible()) pref="compressible::";

    if (fieldname == "k")
    {
        BC["type"]=OFDictData::data("kqRWallFunction");
        BC["value"]=OFDictData::data("uniform 1e-10");
        return true;
    }
    else if (fieldname == "epsilon")
    {
        BC["type"]=OFDictData::data("epsilonWallFunction");
//     BC["Cmu"]=0.09;
//     BC["kappa"]=0.41;
//     BC["E"]=9.8;
//     BC["beta1"]=0.075;
        BC["value"]="uniform 10";
        return true;
    }
    else if (fieldname == "nut")
    {
        if (roughness_z0>0)
        {
            BC["type"]="nutkRoughWallFunction";
            double Cs=0.5;
            BC["Cs"]=Cs;
            BC["Ks"]=roughness_z0*9.793/Cs;
            BC["value"]="uniform 1e-10";
        }
        else
        {
            BC["type"]=OFDictData::data("nutkWallFunction");
            BC["value"]=OFDictData::data("uniform 1e-10");
        }
        return true;
    }
    else if (fieldname == "R")
    {
        BC["type"]=OFDictData::data("kqRWallFunction");
        BC["value"]=OFDictData::data("uniform (1e-10 1e-10 1e-10 1e-10 1e-10 1e-10)");
        return true;
    }
//   else if (fieldname == "mut")
//   {
//     BC["type"]=OFDictData::data("mutkWallFunction");
//     BC["value"]=OFDictData::data("uniform 1e-10");
//     return true;
//   }
//   else if (fieldname == "alphat")
//   {
//     BC["type"]=OFDictData::data(pref+"alphatWallFunction");
//     BC["value"]=OFDictData::data("uniform 1e-10");
//     return true;
//   }

    return false;
}

}