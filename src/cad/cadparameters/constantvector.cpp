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

#include "constantvector.h"
#include "base/linearalgebra.h"

namespace insight 
{
namespace cad
{

insight::cad::ConstantVector::ConstantVector(const arma::mat& value)
: value_(value)
{}


arma::mat insight::cad::ConstantVector::value() const
{
  return value_;
}

VectorPtr matconst(const arma::mat& m)
{
  return VectorPtr(new ConstantVector(m));
}

VectorPtr vec3const(double x, double y, double z)
{
  return VectorPtr(new ConstantVector(vec3(x, y, z)));
}

}
}