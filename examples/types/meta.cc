/*
    This file is part of LibQtLua.

    LibQtLua is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LibQtLua is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with LibQtLua.  If not, see <http://www.gnu.org/licenses/>.

    Copyright (C) 2011, Alexandre Becoulet <alexandre.becoulet@free.fr>

*/

#include <iostream>

#include <QApplication>
#include <QMainWindow>

#include <QtLua/State>
#include <QtLua/Value>

#include "myobject.hh"

MyStructConvert mystruct_converter;
MyQMainwindowConvert qmainwindow_converter;

int main(int argc, char *argv[])
{
	try
	{
		QApplication app(argc, argv);

		QtLua::State state;

		state["myobject"] = QtLua::Value(&state, new MyQObject());

		state["mw"] = QtLua::Value(&state, new QMainWindow());
		state.exec_statements("myobject.mainwindow = mw");

		state.exec_statements("myobject.mystruct = {1, 2}");

		int a = state.exec_statements("return myobject.mystruct[1]")[0];

		std::cout << a << std::endl;
	}
	catch (QtLua::String &e)
	{
		std::cerr << e.constData() << std::endl;
	}
}
