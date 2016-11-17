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

#include <locale>
#include <QLocale>
#include <QDir>
#include <QSplashScreen>
#include <QDialog>
#include <QtGui/QApplication>
#include <QMessageBox>

#include "base/boost_include.h"

#include "base/exception.h"
#include "base/linearalgebra.h"

#include <qthread.h>


#include "isofcasebuilderwindow.h"


class ISOFApp: public QApplication
{
public:
  ISOFApp ( int &argc, char **argv )
    : QApplication ( argc, argv )
  {}

  ~ISOFApp( )
  {}

  bool notify ( QObject *rec, QEvent *ev )
  {
    try
      {
        return QApplication::notify ( rec, ev );
      }
//   catch (Standard_Failure e)
//   {
//     QMessageBox msgBox;
//     msgBox.setIcon(QMessageBox::Critical);
//     msgBox.setText("OpenCascade error: "+QString(e.GetMessageString()));
//
//     msgBox.exec();
//   }
    catch ( insight::Exception e )
      {
        std::cout << e << std::endl;

        QMessageBox msgBox;
        msgBox.setIcon ( QMessageBox::Critical );
        msgBox.setText ( QString ( e.as_string().c_str() ) );

        msgBox.exec();
      }

    return true;
  }
};

int main(int argc, char** argv)
{
  insight::UnhandledExceptionHandling ueh;
  insight::GSLExceptionHandling gsl_errtreatment;
  
  ISOFApp app(argc, argv);

  // After creation of application object!
  std::locale::global(std::locale::classic());
  QLocale::setDefault(QLocale::C);

  isofCaseBuilderWindow window;
  window.show();
  return app.exec();
}
