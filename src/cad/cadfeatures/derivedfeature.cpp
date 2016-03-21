 
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
 */

#include "derivedfeature.h"
#include "base/boost_include.h"
#include <boost/spirit/include/qi.hpp>

namespace qi = boost::spirit::qi;
namespace repo = boost::spirit::repository;
namespace phx   = boost::phoenix;

using namespace std;
using namespace boost;

namespace insight 
{
namespace cad 
{
  
  
defineType(DerivedFeature);


DerivedFeature::DerivedFeature(const NoParameters& nop)
: Feature(nop)
{}

DerivedFeature::DerivedFeature(ConstFeaturePtr basefeat)
: basefeat_(basefeat)
{

}

double DerivedFeature::density() const 
{ 
  if (density_)
    return density_->value(); 
  else
    return basefeat_->density();
}


double DerivedFeature::areaWeight() const 
{ 
  if (areaWeight_)
    return areaWeight_->value(); 
  else
    return basefeat_->areaWeight();
}

arma::mat DerivedFeature::modelCoG() const
{
  if (isRelocationFeature())
    return basefeat_->modelCoG();
  else
    return Feature::modelCoG();
}

double DerivedFeature::mass(double density_ovr, double aw_ovr) const
{
  double rho=density_ovr, aw=aw_ovr;
  if (density_ && (density_ovr<0.)) rho=density_->value();
  if (areaWeight_ && (aw_ovr<0.)) aw=areaWeight_->value();
  
  std::cout<<"DerivedFeature: "<<rho<<" reloc:"<<isRelocationFeature()<<std::endl;
  if (isRelocationFeature())
  {
    return basefeat_->mass(rho, aw);
  }
  else
  {
    return Feature::mass(rho, aw);
  }
}

}
}