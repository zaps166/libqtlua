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

    Copyright (C) 2008, Alexandre Becoulet <alexandre.becoulet@free.fr>

*/

#include <iostream>

#include <QtLua/State>
#include <QtLua/Function>

/* anchor 1 */
QTLUA_FUNCTION(foo)
{
	QtLua::String a = get_arg<QtLua::String>(args, 0);
	int b = get_arg<int>(args, 1, 42);

	Q_UNUSED(a);
	Q_UNUSED(b);

	return QtLua::Value(ls, "test");
}

int main()
{
	try
	{
		{
			QtLua::State state;

			QTLUA_FUNCTION_REGISTER(&state, "bar.", foo);

			state.openlib(QtLua::QtLuaLib);
			state.enable_qdebug_print(true);
			state.exec_statements("print(bar.foo(\"test\"))");
		}

		{
			QtLua::State state;

			static QtLua_Function_foo foo;

			QtLua::Value f(&state, foo);
		}
	}
	catch (QtLua::String &e)
	{
		std::cerr << e.constData() << std::endl;
	}

	return 0;
}
