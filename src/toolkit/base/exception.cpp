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


#include "exception.h"
#include <execinfo.h>
#include <sstream>
#include <cstdlib>

using namespace std;

namespace insight
{
  
std::ostream& operator<<(std::ostream& os, const Exception& ex)
{
  os<<ex.message_<<endl;
  if (ex.strace_!="")
    os<<ex.strace_<<endl;
  return os;
}


Exception::Exception(const std::string& msg, bool strace)
{
  message_=msg;
  if (strace)
  {
    int num=10;
    void *array[num];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, num);

    // print out all the frames to stderr
    char **str=backtrace_symbols(array, size);

    ostringstream oss;
    for (size_t i=0; i<size; i++)
    {
      oss<<str[i]<<endl;
      //free(str[i]);
    }
    //free(str);

    strace_=oss.str();
  }
  else
    strace_="";
}

Exception::~Exception()
{
}

Exception::operator std::string() const
{
  if (strace_!="")
    return message_+"\n"+strace_;
  else
    return message_;
}

void Warning(const std::string& msg)
{
  std::cout<<msg<<std::endl;
}

}