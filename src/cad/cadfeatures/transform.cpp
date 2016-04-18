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

#include "transform.h"
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


defineType(Transform);
addToFactoryTable(Feature, Transform, NoParameters);

Transform::Transform(const NoParameters& nop)
: DerivedFeature(nop)
{}


// TopoDS_Shape Transform::makeTransform(const SolidModel& m1, const arma::mat& trans, const arma::mat& rot, double scale)
// {
//   gp_Trsf tr0, tr1, tr2;
// //   TopoDS_Shape intermediate_shape=m1;
// //   
//   tr0.SetScaleFactor(scale);
// //   intermediate_shape=BRepBuilderAPI_Transform(intermediate_shape, tr0).Shape();
// // 
//   tr1.SetTranslation(to_Vec(trans));  
// //   intermediate_shape=BRepBuilderAPI_Transform(intermediate_shape, tr1).Shape();
// // 
//   double phi=norm(rot, 2);
//   if (phi>1e-10)
//   {
//     gp_Vec axis=to_Vec(rot);
//     axis.Normalize();
//     tr2.SetRotation(gp_Ax1(gp_Pnt(0,0,0), axis), phi);
// //     intermediate_shape=BRepBuilderAPI_Transform(intermediate_shape, tr2).Shape();
//   }  
//   
//   gp_Trsf trcomp=tr2.Multiplied(tr1).Multiplied(tr0);
//   return makeTransform(m1, trcomp);
// // 
// //   // Apply rotation first, then translation
// //   return intermediate_shape;
// }
// 
// TopoDS_Shape Transform::makeTransform(const SolidModel& m1, const gp_Trsf& trsf)
// {
//   if (m1.hasExplicitCoG())
//   {
//     this->setCoGExplicitly( vec3(to_Pnt(m1.modelCoG()).Transformed(trsf)) );
//   }
//   if (m1.hasExplicitMass()) setMassExplicitly(m1.mass());
//   
//   // Transform all ref points and ref vectors
//   copyDatumsTransformed(m1, trsf);
//   
//   return BRepBuilderAPI_Transform(m1, trsf).Shape();
// }


Transform::Transform(FeaturePtr m1, VectorPtr trans, VectorPtr rot, ScalarPtr scale)
: DerivedFeature(m1),
  m1_(m1),
  trans_(trans),
  rot_(rot),
  sf_(scale)
{
//   setShape(makeTransform(m1, trans, rot, scale));
//   m1.unsetLeaf();
}

Transform::Transform(FeaturePtr m1, VectorPtr rot, VectorPtr rotorg)
: DerivedFeature(m1),
  m1_(m1),
  rot_(rot),
  rotorg_(rotorg)
{
//   setShape(makeTransform(m1, trans, rot, scale));
//   m1.unsetLeaf();
}

Transform::Transform(FeaturePtr m1, VectorPtr trans)
: DerivedFeature(m1),
  m1_(m1),
  trans_(trans)
{
//   setShape(makeTransform(m1, trans, vec3(0,0,0), 1.0));
//   m1.unsetLeaf();
}

Transform::Transform(FeaturePtr m1, ScalarPtr sf)
: DerivedFeature(m1),
  m1_(m1),
  sf_(sf)
{
//   setShape(makeTransform(m1, vec3(0,0,0), vec3(0,0,0), sf));
//   m1.unsetLeaf();
}

Transform::Transform(FeaturePtr m1, const gp_Trsf& trsf)
: DerivedFeature(m1),
  m1_(m1), trsf_(new gp_Trsf(trsf))
{
}

void Transform::build()
{
  gp_Trsf tr0, tr1, tr2;

  if (!trsf_)
  {
    if (sf_)
      tr0.SetScaleFactor(*sf_);

    if (trans_)
      tr1.SetTranslation(to_Vec(*trans_));  

    if (rot_)
    {
      double phi=norm(rot_->value(), 2);
      if (phi>1e-10)
      {
	gp_Vec axis=to_Vec(rot_->value());
	axis.Normalize();
	gp_Pnt rorg(0,0,0);
	if (rotorg_) rorg=to_Pnt(rotorg_->value());
	tr2.SetRotation(gp_Ax1(rorg, axis), phi);
      }  
    }
    
    trsf_.reset(new gp_Trsf(tr2.Multiplied(tr1).Multiplied(tr0)));
  }
  
  setShape(BRepBuilderAPI_Transform(*m1_, *trsf_).Shape());
  
  // Transform all ref points and ref vectors
  copyDatumsTransformed(*m1_, *trsf_);
}


void Transform::insertrule(parser::ISCADParser& ruleset) const
{
  ruleset.modelstepFunctionRules.add
  (
    "Transform",	
    typename parser::ISCADParser::ModelstepRulePtr(new typename parser::ISCADParser::ModelstepRule( 

    ( '(' > ruleset.r_solidmodel_expression > ',' > ruleset.r_vectorExpression > ',' 
	> ruleset.r_vectorExpression > 
	( 
	 (',' > ruleset.r_scalarExpression ) 
	 | 
	 qi::attr(ScalarPtr( new ConstantScalar(1.0)))
	) > ')' ) 
      [ qi::_val = phx::construct<FeaturePtr>(phx::new_<Transform>(qi::_1, qi::_2, qi::_3, qi::_4)) ]
      
    ))
  );
  
  ruleset.modelstepFunctionRules.add
  (
    "Rotate",
    typename parser::ISCADParser::ModelstepRulePtr(new typename parser::ISCADParser::ModelstepRule( 

    ( '(' > ruleset.r_solidmodel_expression > ',' > ruleset.r_vectorExpression > ',' 
	> ruleset.r_vectorExpression > 
	')' ) 
      [ qi::_val = phx::construct<FeaturePtr>(phx::new_<Transform>(qi::_1, qi::_2, qi::_3)) ]
      
    ))
  );
}

gp_Trsf Transform::transformation() const
{
  checkForBuildDuringAccess();
  return *trsf_;
}


}
}
