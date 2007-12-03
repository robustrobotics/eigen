// This file is part of Eigen, a lightweight C++ template library
// for linear algebra. Eigen itself is part of the KDE project.
//
// Copyright (C) 2006-2007 Benoit Jacob <jacob@math.jussieu.fr>
//
// Eigen is free software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 or (at your option) any later version.
//
// Eigen is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along
// with Eigen; if not, write to the Free Software Foundation, Inc., 51
// Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. This exception does not invalidate any other reasons why a work
// based on this file might be covered by the GNU General Public License.

#include "main.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    bool has_set_repeat = false;
    bool has_set_seed = false;
    bool want_help = false;
    unsigned int seed;
    int repeat;
    
    QStringList args = QCoreApplication::instance()->arguments();
    args.takeFirst(); // throw away the first argument (path to executable)
    foreach(QString arg, args)
    {
      if(arg.startsWith("r"))
      {
        if(has_set_repeat)
        {
          qDebug() << "Argument" << arg << "conflicting with a former argument";
          return 1;
        }
        repeat = arg.remove(0, 1).toInt();
        has_set_repeat = true;
        if(repeat <= 0)
        {
          qDebug() << "Invalid \'repeat\' value" << arg;
          return 1;
        }
      }
      else if(arg.startsWith("s"))
      {
        if(has_set_seed)
        {
          qDebug() << "Argument" << arg << "conflicting with a former argument";
          return 1;
        }
        bool ok;
        seed = arg.remove(0, 1).toUInt(&ok);
        has_set_seed = true;
        if(!ok)
        {
          qDebug() << "Invalid \'seed\' value" << arg;
          return 1;
        }
      }
      else if(arg == "h" || arg == "-h" || arg.contains("help", Qt::CaseInsensitive))
      {
        want_help = true;
      }
      else
      {
        qDebug() << "Invalid command-line argument" << arg;
        return 1;
      }
    }
    
    if(want_help)
    {
      qDebug() << "This test application takes the following optional arguments:";
      qDebug() << "  rN     Repeat each test N times (default:" << DEFAULT_REPEAT << ")";
      qDebug() << "  sN     Use N as seed for random numbers (default: based on current time)";
      return 0;
    }
    
    if(!has_set_seed) seed = (unsigned int) time(NULL);
    if(!has_set_repeat) repeat = DEFAULT_REPEAT;
    
    qDebug() << "Initializing random number generator with seed" << seed;
    srand(seed);
    qDebug() << "Repeating each test" << repeat << "times";

    Eigen::EigenTest test(repeat);
    return QTest::qExec(&test, 1, argv);
}

#include "main.moc"
