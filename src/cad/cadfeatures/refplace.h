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

#ifndef INSIGHT_CAD_REFPLACE_H
#define INSIGHT_CAD_REFPLACE_H

#include "derivedfeature.h"

namespace insight {
namespace cad {

    
    
    
class Condition
{
public:
    double residual(const arma::mat& values) const;
    virtual double residual(const gp_Trsf& tr) const =0;
};


class CoincidentPoint
: public Condition
{
    VectorPtr p_org_,  p_targ_;
public:
    CoincidentPoint(VectorPtr p_org, VectorPtr p_targ);
    virtual double residual(const gp_Trsf& tr) const;
};


typedef boost::shared_ptr<Condition> ConditionPtr;
typedef std::vector<ConditionPtr> ConditionList;


class RefPlace
: public DerivedFeature
{
  FeaturePtr m_;
  
  ConditionList conditions_;
  
  boost::shared_ptr<gp_Trsf> trsf_;

public:
  declareType("RefPlace");
  RefPlace(const NoParameters& nop = NoParameters());
  RefPlace(FeaturePtr m, const gp_Ax2& cs);
  RefPlace(FeaturePtr m, ConditionList conditions);
  
  virtual void build();
  virtual void insertrule(parser::ISCADParser& ruleset) const;

  virtual bool isTransformationFeature() const { return true; }
  virtual gp_Trsf transformation() const;
};




}
}

#endif // INSIGHT_CAD_REFPLACE_H
