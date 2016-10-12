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

#include "circularpattern.h"
#include "transform.h"
#include "base/boost_include.h"
#include <boost/spirit/include/qi.hpp>
namespace qi = boost::spirit::qi;
namespace repo = boost::spirit::repository;
namespace phx   = boost::phoenix;

using namespace std;
using namespace boost;

namespace insight {
namespace cad {


    
    
defineType(CircularPattern);
addToFactoryTable(Feature, CircularPattern, NoParameters);




CircularPattern::CircularPattern(const NoParameters& nop): Compound(nop)
{}



  
CircularPattern::CircularPattern(FeaturePtr m1, VectorPtr p0, VectorPtr axis, ScalarPtr n, bool center, const std::string& filterrule)
: m1_(m1),
  p0_(p0),
  axis_(axis),
  n_(n),
  center_(center),
  filterrule_(filterrule)
{
}




CircularPattern::CircularPattern(FeaturePtr m1, FeaturePtr otherpat)
: m1_(m1), otherpat_(otherpat)
{
}




FeaturePtr CircularPattern::create(FeaturePtr m1, VectorPtr p0, VectorPtr axis, ScalarPtr n, bool center, const std::string& filterrule)
{
    return FeaturePtr(new CircularPattern(m1, p0, axis, n, center, filterrule));
}




FeaturePtr CircularPattern::create(FeaturePtr m1, FeaturePtr otherpat)
{
    return FeaturePtr(new CircularPattern(m1, otherpat));
}




void CircularPattern::build()
{

    int n;
    arma::mat p0, axis;
    double delta_phi, phi0;
    
    if (otherpat_)
    {
//         boost::shared_ptr<CircularPattern> op = boost::dynamic_pointer_cast<CircularPattern, Feature>(otherpat_);
//         if (!op)
//         {
//             throw insight::Exception("Construction of CircularPattern with copied parameters: provided feature is not a circular pattern feature!");
//         }
        n=otherpat_->getDatumScalar("n");
        p0=otherpat_->getDatumPoint("p0");
        delta_phi=otherpat_->getDatumScalar("delta_phi");
        axis=otherpat_->getDatumVector("axis");
        phi0=otherpat_->getDatumScalar("phi0");
    }
    else
    {
        n = n_->value();
        p0=p0_->value();
        delta_phi=norm(axis_->value(), 2);
        axis=axis_->value()/delta_phi;
        phi0=0.0;
        if (center_) phi0=-0.5*delta_phi*double(n-1);
    }
    
    gp_Ax1 ax(to_Pnt(p0), to_Vec(axis));
    
    refvalues_["n"]=n;
    refvalues_["delta_phi"]=delta_phi;
    refvalues_["phi0"]=phi0;
    refpoints_["p0"]=p0;
    refvectors_["axis"]=axis;
    

    std::vector<std::string> rules;
    if (!filterrule_.empty())
        boost::split(rules, filterrule_, boost::is_any_of(","));

    int j=0;
    CompoundFeatureMap instances;

    for (int i=0; i<n; i++)
    {
        bool ok=true;
        try
        {
            BOOST_FOREACH(const std::string& r, rules)
            {
                if (boost::lexical_cast<int>(r)==(i+1)) ok=false;
            }
        }
        catch (...)
        {
            throw insight::Exception("CircularPattern: invalid filter expression! (was '"+filterrule_+"')");
        }

        if (ok)
        {
            gp_Trsf tr;
            tr.SetRotation(ax, phi0+delta_phi*double(i));
            
            components_[str( format("component%d") % (j+1) )] =
                FeaturePtr(new Transform(m1_, tr));
                
            j++;
        }
    }


    m1_->unsetLeaf();
    Compound::build();

}


void CircularPattern::insertrule(parser::ISCADParser& ruleset) const
{
  ruleset.modelstepFunctionRules.add
  (
    "CircularPattern",	
    typename parser::ISCADParser::ModelstepRulePtr(new typename parser::ISCADParser::ModelstepRule( 

//     ( 
      (
      '(' >> 
          ruleset.r_solidmodel_expression >> ',' 
        >> ruleset.r_vectorExpression >> ',' 
        >> ruleset.r_vectorExpression >> ',' 
        >> ruleset.r_scalarExpression 
        >> ( ( ',' >> qi::lit("centered") >> qi::attr(true) ) | qi::attr(false) ) 
        >> ( ( ',' >> qi::lit("not") >> ruleset.r_string ) | qi::attr(std::string()) ) 
        >> ')' 
      ) [ qi::_val = phx::bind(&CircularPattern::create, qi::_1, qi::_2, qi::_3, qi::_4, qi::_5, qi::_6) ]
      |
      (
      '(' >> 
          ruleset.r_solidmodel_expression >> ',' >> ruleset.r_solidmodel_expression 
        >> ')' 
      ) [ qi::_val = phx::bind(&CircularPattern::create, qi::_1, qi::_2) ]
//      ) 
    ))
  );
}


}
}
