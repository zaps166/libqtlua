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

class Value : public QObject
{
	Q_OBJECT

private slots:
	void test1();
	void test2();
	void test3();
	void test4();
	void test5();
	void test6();
};

void Value::test1()
{
	QtLua::State ls;

	ls["nill"] = QtLua::Value(&ls);
	ls["btrue"] = QtLua::Value(&ls, QtLua::Value::True);
	ls["bfalse"] = QtLua::Value(&ls, QtLua::Value::False);
	ls["int"] = QtLua::Value(&ls, 0);
	ls["double"] = QtLua::Value(&ls, 1.0);
	ls["str1"] = QtLua::Value(&ls, "hello");
	ls["str2"] = QtLua::Value(&ls, QtLua::String("hello"));

	QtLua::Value::List res = ls.exec_statements("return nill, btrue, bfalse, int, double, str1, str2");

	QCOMPARE(res.size(), 7);
	QCOMPARE(res[0].type(), QtLua::Value::TNil);
	QCOMPARE(res[1].to_boolean(), QtLua::Value::True);
	QCOMPARE(res[2].to_boolean(), QtLua::Value::False);
	QCOMPARE(res[3].to_number(), 0.0);
	QCOMPARE(res[4].to_number(), 1.0);
	QCOMPARE(res[5].to_string().constData(), "hello");
	QCOMPARE(res[6].to_string().constData(), "hello");
}

void Value::test2()
{
	QtLua::State ls;

	ls["btrue"] = QtLua::Value::True;
	ls["bfalse"] = QtLua::Value::False;
	ls["int"] = 0;
	ls["double"] = 1.0f;
	ls["str1"] = "hello";
	ls["str2"] = QtLua::String("hello");

	QtLua::Value::List res = ls.exec_statements("return btrue, bfalse, int, double, str1, str2");

	QCOMPARE(res.size(), 6);
	QCOMPARE(res[0].to_boolean(), QtLua::Value::True);
	QCOMPARE(res[1].to_boolean(), QtLua::Value::False);
	QCOMPARE(res[2].to_number(), 0.0);
	QCOMPARE(res[3].to_number(), 1.0);
	QCOMPARE(res[4].to_string().constData(), "hello");
	QCOMPARE(res[5].to_string().constData(), "hello");
}

void Value::test3()
{
	QtLua::State ls;

	ls["foo"] = 1.0;
	ls["bar"] = ls.at("foo");

	QtLua::Value::List res = ls.exec_statements("return bar");

	QCOMPARE(res.size(), 1);
	QCOMPARE(res[0].to_boolean(), QtLua::Value::True);
}

void Value::test4()
{
	QtLua::State ls;

	ls.exec_statements("var=\"foo\"; tbl={}; tbl.var=\"bar\"");

	QCOMPARE(ls.get_global("var").to_string().constData(), "foo");
	QCOMPARE(ls.get_global("tbl.var").to_string().constData(), "bar");
}

void Value::test5()
{
	QtLua::State ls;

	ls.set_global("var", QtLua::Value(&ls, "foo"));
	ls.set_global("tbl.var", QtLua::Value(&ls, "bar"));

	QCOMPARE(ls.exec_statements("return var").at(0).to_string().constData(), "foo");
	QCOMPARE(ls.exec_statements("return tbl.var").at(0).to_string().constData(), "bar");
}

void Value::test6()
{
	QtLua::State ls;

	ls.openlib(QtLua::MathLib);

	QtLua::Value num(&ls, 3.14159);
	QtLua::Value func = ls.get_global("math.cos");

	QCOMPARE(func.type_name().constData(), "lua::function");
	QVERIFY(func(num).at(0).to_number() + 1.0 < 0.001);
}

QTEST_APPLESS_MAIN(Value)

#include "tst_value.moc"
