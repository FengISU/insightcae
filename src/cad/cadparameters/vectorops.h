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

#ifndef INSIGHT_CAD_VECTOROPS_H
#define INSIGHT_CAD_VECTOROPS_H

#include "cadparameter.h"

namespace insight 
{
namespace cad 
{

class VectorFromComponents
: public insight::cad::Vector
{
  ScalarPtr p1_, p2_, p3_;
public:
  VectorFromComponents(ScalarPtr p1, ScalarPtr p2, ScalarPtr p3);
  virtual arma::mat value() const;
};




class CrossMultipliedVector
: public insight::cad::Vector
{
  VectorPtr p1_, p2_;
public:
  CrossMultipliedVector(VectorPtr p1, VectorPtr p2);
  virtual arma::mat value() const;
};




class DotMultipliedVector
: public insight::cad::Scalar
{
  VectorPtr p1_, p2_;
public:
  DotMultipliedVector(VectorPtr p1, VectorPtr p2);
  virtual double value() const;
};




class ScalarMultipliedVector
: public insight::cad::Vector
{
  ScalarPtr p1_;
  VectorPtr p2_;
public:
  ScalarMultipliedVector(ScalarPtr p1, VectorPtr p2);
  virtual arma::mat value() const;
};




class ScalarDividedVector
: public insight::cad::Vector
{
  VectorPtr p1_;
  ScalarPtr p2_;
public:
  ScalarDividedVector(VectorPtr p1, ScalarPtr p2);
  virtual arma::mat value() const;
};




class AddedVector
: public insight::cad::Vector
{
  VectorPtr p1_, p2_;
public:
  AddedVector(VectorPtr p1, VectorPtr p2);
  virtual arma::mat value() const;
};



class SubtractedVector
: public insight::cad::Vector
{
  VectorPtr p1_, p2_;
public:
  SubtractedVector(VectorPtr p1, VectorPtr p2);
  virtual arma::mat value() const;
};


}
}

#endif // INSIGHT_CAD_VECTOROPS_H