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

#include <QtTest>

#include <QtLua/State>
#include <QtLua/Value>
#include <QtLua/Function>

#define QVERIFY_NORET(statement) \
	QTest::qVerify((statement), #statement, "", __FILE__, __LINE__)

QTLUA_FUNCTION(test)
{
	int a = args[1].to_integer();

	if (a == 42)
		throw QtLua::String("bad value!");

	QVERIFY_NORET(args[0] == yield(ls));

	return QtLua::Value(ls, a * 2);
}

class Corutines : public QObject
{
	Q_OBJECT

private slots:
	void test1();
	void test2();
	void test3();
	void test4();
};

void Corutines::test1()
{
	QtLua::State ls;
	ls.openlib(QtLua::AllLibs);

	QtLua::Value co = ls.exec_statements("return coroutine.create(function(a) while (a < 100) do a = coroutine.yield(a) end return a+1 end )").at(0);

	QtLua::Value r = co(QtLua::Value(&ls, 7)).at(0);
	ls.check_empty_stack();
	QCOMPARE(r.to_integer(), 7);
	if (ls.lua_version() > 500)
		QVERIFY(!co.is_dead());
	ls.check_empty_stack();

	r = co(QtLua::Value(&ls, 22)).at(0);
	ls.check_empty_stack();
	QCOMPARE(r.to_integer(), 22);
	if (ls.lua_version() > 500)
		QVERIFY(!co.is_dead());
	ls.check_empty_stack();

	r = co(QtLua::Value(&ls, 142)).at(0);
	ls.check_empty_stack();
	QCOMPARE(r.to_integer(), 143);
	if (ls.lua_version() > 500)
		QVERIFY(co.is_dead());
	ls.check_empty_stack();

	bool err = false;
	try
	{
		r = co(QtLua::Value(&ls, 66)).at(0);
	}
	catch (...)
	{
		err = true;
	}
	ls.check_empty_stack();
	QVERIFY(err);
}

void Corutines::test2()
{
	QtLua::State ls;
	ls.openlib(QtLua::AllLibs);

	QtLua::Value m = ls.exec_statements("return function(a) while (a < 100) do a = coroutine.yield(a) end return a+1 end").at(0);

	QtLua::Value co = QtLua::Value::new_thread(&ls, m);

	QtLua::Value r = co(QtLua::Value(&ls, 7)).at(0);
	ls.check_empty_stack();
	QCOMPARE(r.to_integer(), 7);

	r = co(QtLua::Value(&ls, 22)).at(0);
	ls.check_empty_stack();
	QCOMPARE(r.to_integer(), 22);

	r = co(QtLua::Value(&ls, 142)).at(0);
	ls.check_empty_stack();
	QCOMPARE(r.to_integer(), 143);
}

void Corutines::test3()
{
	QtLua_Function_test test;
	QtLua::State ls;
	ls.openlib(QtLua::AllLibs);

	ls["test"] = test;
	QtLua::Value m = ls.exec_statements("return function(a, b) while (true) do b = test(a, b) + 1 end end").at(0);
	QtLua::Value co = QtLua::Value::new_thread(&ls, m);

	QCOMPARE(ls["test"](QtLua::Value(&ls), QtLua::Value(&ls, 8)).at(0).to_integer(), 16);
	ls.check_empty_stack();

	QCOMPARE(co(co, QtLua::Value(&ls, 7)).at(0).to_integer(), 14);
	ls.check_empty_stack();

	QtLua::String err;
	try
	{
		co(QtLua::Value(&ls, 41));
	}
	catch (const QtLua::String &s)
	{
		err = s;
	}
	ls.check_empty_stack();
	QVERIFY(err.endsWith(QtLua::String("bad value!")));
}

void Corutines::test4()
{
	QtLua_Function_test test;
	QtLua::State ls;
	ls.openlib(QtLua::AllLibs);

	ls["test"] = test;
	QtLua::Value co = ls.exec_statements("return coroutine.create(function(a, b) while (true) do b = test(a, b) + 1 end end)").at(0);

	if (ls.lua_version() > 500)
	{
		QCOMPARE(co(co, QtLua::Value(&ls, 7)).at(0).to_integer(), 14);
	}
	else
	{
		QCOMPARE(co(QtLua::Value(&ls, QtLua::Value::True), QtLua::Value(&ls, 7)).at(0).to_integer(), 14);
	}
	ls.check_empty_stack();
}

QTEST_APPLESS_MAIN(Corutines)

#include "tst_coroutines.moc"
