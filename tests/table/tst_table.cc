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

class Table : public QObject
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

void Table::test1()
{
	QtLua::State ls;

	ls["t"] = QtLua::Value::new_table(&ls);
	ls.check_empty_stack();

	ls["t"]["a"] = 5;
	ls.check_empty_stack();

	ls.at("t")["b"] = ls.at("t").at("a");
	ls.check_empty_stack();

	ls["t"]["c"] = ls["t"]["b"].value();
	ls.check_empty_stack();

	QCOMPARE(ls.at("t").at("c").to_integer(), 5);
	ls.check_empty_stack();

	QCOMPARE(ls["t"]["c"].to_integer(), 5);
	ls.check_empty_stack();

	QCOMPARE(ls.get_global("t.c").to_integer(), 5);
	ls.check_empty_stack();

	QVERIFY(ls.get_global("t.d").is_nil());
	ls.check_empty_stack();

	bool err = false;
	try
	{
		ls.get_global("t.c.d");
	}
	catch (...)
	{
		err = true;
	}
	ls.check_empty_stack();
	QVERIFY(err);

	err = false;
	try
	{
		ls.set_global("t.c.d", QtLua::Value(&ls, 5));
	}
	catch (...)
	{
		err = true;
	}
	ls.check_empty_stack();
	QVERIFY(err);

	ls.set_global("t.d.a", QtLua::Value(&ls, 42));
	ls.check_empty_stack();

	QVERIFY(ls.get_global("t.d.a").to_integer() == 42);
	ls.check_empty_stack();
}

void Table::test2()
{
	int j;
	QtLua::State ls;

	ls.openlib(QtLua::QtLuaLib);

	ls.exec_statements("t={a=1, b=\"foo\", c=3}");
	ls.check_empty_stack();

	QCOMPARE(ls.at("t")["a"].value().to_integer(), 1);
	ls.check_empty_stack();

	QCOMPARE(ls["t"]["b"].value().to_string().constData(), "foo");
	ls.check_empty_stack();

	QCOMPARE(ls.at("t").at("c").to_integer(), 3);
	ls.check_empty_stack();

	QtLua::Value t = ls.at("t");
	ls.check_empty_stack();

	j = 0;
	for (QtLua::Value::iterator i = t.begin(); i != t.end(); i++, j++)
	{
		QCOMPARE(t.at(i.key()), i.value().value());
		ls.check_empty_stack();
	}
	QCOMPARE(j, 3);
	ls.check_empty_stack();

	j = 0;
	for (QtLua::Value::const_iterator i = t.begin(); i != t.end(); i++, j++)
	{
		QCOMPARE(t.at(i.key()), i.value());
		ls.check_empty_stack();
	}

	QCOMPARE(j, 3);
	ls.check_empty_stack();

	for (QtLua::Value::iterator i = t.begin(); i != t.end(); i++, j++)
	{
		*i = QtLua::String(i.key().to_string() + "_foo");
		ls.check_empty_stack();
	}

	QCOMPARE(ls.at("t").at("a").to_string().constData(), "a_foo");
	ls.check_empty_stack();

	QCOMPARE(ls.at("t").at("b").to_string().constData(), "b_foo");
	ls.check_empty_stack();

	QCOMPARE(ls.at("t").at("c").to_string().constData(), "c_foo");
	ls.check_empty_stack();

	QCOMPARE(ls.exec_statements("i=0; r={}; for key, value in each(t) do r[key]=value..\"bar\"; i=i+1 end; return i").at(0).to_integer(), 3);

	QCOMPARE(ls.at("r").at("a").to_string().constData(), "a_foobar");
	ls.check_empty_stack();

	QCOMPARE(ls.at("r").at("b").to_string().constData(), "b_foobar");
	ls.check_empty_stack();

	QCOMPARE(ls.at("r").at("c").to_string().constData(), "c_foobar");
	ls.check_empty_stack();
}

void Table::test3()
{
	QtLua::State ls;

	ls.openlib(QtLua::AllLibs);
	ls.exec_statements("t={}; setmetatable(t, { "
					   "__index = function() assert(false) end,"
					   "__newindex = function() assert(false) end"
					   " })");

	bool err = false;
	try
	{
		ls.set_global("t.c", QtLua::Value(&ls, 5));
	}
	catch (...)
	{
		err = true;
	}
	ls.check_empty_stack();
	QVERIFY(err);

	err = false;
	try
	{
		ls["t"]["a"] = 5;
	}
	catch (...)
	{
		err = true;
	}
	ls.check_empty_stack();
	QVERIFY(err);

	err = false;
	try
	{
		ls.get_global("t.c");
	}
	catch (...)
	{
		err = true;
	}
	ls.check_empty_stack();
	QVERIFY(err);

	err = false;
	try
	{
		ls.at("t").at("c").is_nil();
	}
	catch (...)
	{
		err = true;
	}
	ls.check_empty_stack();
	QVERIFY(err);

	err = false;
	try
	{
		ls["t"]["c"].is_nil();
	}
	catch (...)
	{
		err = true;
	}
	ls.check_empty_stack();
	QVERIFY(err);

	err = false;
	try
	{
		QtLua::Value t = ls.at("t");
		for (QtLua::Value::iterator i = t.begin(); i != t.end(); i++)
			;
	}
	catch (...)
	{
		err = true;
	}
	ls.check_empty_stack();
	QVERIFY(!err);
}

void Table::test4()
{
	QtLua::State ls;

	ls.exec_statements("t=function() end");

	bool err = false;
	try
	{
		ls.set_global("t.c", QtLua::Value(&ls, 5));
	}
	catch (...)
	{
		err = true;
	}
	ls.check_empty_stack();
	QVERIFY(err);

	err = false;
	try
	{
		ls["t"]["a"] = 5;
	}
	catch (...)
	{
		err = true;
	}
	ls.check_empty_stack();
	QVERIFY(err);

	err = false;
	try
	{
		ls.get_global("t.c");
	}
	catch (...)
	{
		err = true;
	}
	ls.check_empty_stack();
	QVERIFY(err);

	err = false;
	try
	{
		ls.at("t").at("c").is_nil();
	}
	catch (...)
	{
		err = true;
	}
	ls.check_empty_stack();
	QVERIFY(err);

	err = false;
	try
	{
		ls["t"]["c"].is_nil();
	}
	catch (...)
	{
		err = true;
	}
	ls.check_empty_stack();
	QVERIFY(err);

	err = false;
	try
	{
		QtLua::Value t = ls.at("t");
		for (QtLua::Value::iterator i = t.begin(); i != t.end(); i++)
			;
	}
	catch (...)
	{
		err = true;
	}
	ls.check_empty_stack();
	QVERIFY(err);
}

void Table::test5()
{
	QtLua::State ls;
	ls.openlib(QtLua::AllLibs);

	ls.exec_statements("t = { 10, 20, 30, 40, 50, 60, 70, 80, 90 };"
					   "q = { };"
					   "s = { key = 42 };"
					   "e = function() end");

	QCOMPARE(ls["t"].len(), 9);
	ls.check_empty_stack();
	QCOMPARE(ls["q"].len(), 0);
	ls.check_empty_stack();
	QCOMPARE(ls["s"].len(), 0);
	ls.check_empty_stack();
	QVERIFY(ls["qt"]["meta"].len() > 0);
	ls.check_empty_stack();

	QVERIFY(!ls["t"].is_empty());
	ls.check_empty_stack();
	QVERIFY(ls["q"].is_empty());
	ls.check_empty_stack();
	QVERIFY(!ls["s"].is_empty());
	ls.check_empty_stack();
	QVERIFY(!ls["qt"]["meta"].is_empty());
	ls.check_empty_stack();

	bool err = false;
	try
	{
		ls["e"].len();
	}
	catch (...)
	{
		err = true;
	}
	ls.check_empty_stack();
	QVERIFY(err);

	err = false;
	try
	{
		ls["e"].is_empty();
	}
	catch (...)
	{
		err = true;
	}
	ls.check_empty_stack();
	QVERIFY(err);
}

void Table::test6()
{
	{
		const int values[] {
			10, 20, 30, 40, 50, 80, 90
		};

		QtLua::State ls;
		ls.openlib(QtLua::AllLibs);

		ls.exec_statements("t = {10, 20, 30, 40, 50, 60, 70, 80, 90};");
		QtLua::Value t = ls["t"];

		t.table_shift(6, -2);
		ls.check_empty_stack();
		for (QtLua::Value::iterator i = t.begin(); i != t.end(); i++)
			QCOMPARE(values[i.key().to_integer() - 1], i.value().to_integer());
	}

	{
		const int values[] {
			10, 20, 30, 40, 50, 60, 70, 80, 42, 42, 42, 90
		};

		QtLua::State ls;
		ls.openlib(QtLua::AllLibs);

		ls.exec_statements("t = {10, 20, 30, 40, 50, 60, 70, 80, 90};");
		QtLua::Value t = ls["t"];

		t.table_shift(9, 3, QtLua::Value(&ls, 42));
		ls.check_empty_stack();
		for (QtLua::Value::iterator i = t.begin(); i != t.end(); i++)
			QCOMPARE(values[i.key().to_integer() - 1], i.value().to_integer());
	}
}

QTEST_APPLESS_MAIN(Table)

#include "tst_table.moc"
