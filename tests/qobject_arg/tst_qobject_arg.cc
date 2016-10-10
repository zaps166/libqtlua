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
#include <QtLua/UserData>

struct MyObjectUD : public QObject
{
	Q_OBJECT
public:
	MyObjectUD()
		: QObject(0)
	{
	}

	Q_INVOKABLE MyObjectUD(int num, QObject *parent)
		: QObject(parent)
	{
		QCOMPARE(num, 42);
		QVERIFY(!parent);
	}

	void send(QtLua::UserData::ptr ud)
	{
		emit ud_arg(ud);
	}

	Q_INVOKABLE double foo(double a)
	{
		return a * 42;
	}

	QtLua::UserData::ptr _ud;

public slots:
	void ud_slot(QtLua::UserData::ptr ud)
	{
		_ud = ud;
	}

signals:
	void ud_arg(QtLua::UserData::ptr ud);
};

struct MyData : public QtLua::UserData
{
	MyData(double d)
		: _data(d)
	{
	}

	QtLua::Value meta_index(QtLua::State *ls, const QtLua::Value &key)
	{
		Q_UNUSED(key)
		return QtLua::Value(ls, _data);
	}

	double _data;
};

struct MyObjectQO : public QObject
{
	Q_OBJECT

public:
	MyObjectQO()
		: QObject(0)
		, _qo(0)
	{
	}

	void send(QObject *qo)
	{
		emit qo_arg(qo);
	}

	QObject *_qo;

public slots:
	void qo_slot(QObject *qo)
	{
		_qo = qo;
	}

signals:
	void qo_arg(QObject *o);
};

/**/

class QObjectArgs : public QObject
{
	Q_OBJECT

private slots:
	void test1();
	void test2();
	void test3();
};

void QObjectArgs::test1()
{
	QtLua::State ls;

	MyObjectUD *myobj = new MyObjectUD();

	ls.exec_statements("function f(obj, ud) v = ud; end");

	QVERIFY(ls.at("f").connect(myobj, "ud_arg(QtLua::UserData::ptr)"));
	ls.check_empty_stack();

	QCOMPARE(ls.at("v").type(), QtLua::Value::TNil);
	ls.check_empty_stack();

	myobj->send(QTLUA_REFNEW(MyData, 18.0));
	ls.check_empty_stack();

	QCOMPARE(ls.at("v").type(), QtLua::Value::TUserData);
	ls.check_empty_stack();

	QCOMPARE(ls.at("v").at(0).to_number(), 18.0);
	ls.check_empty_stack();

	QVERIFY(ls.at("f").disconnect(myobj, "ud_arg(QtLua::UserData::ptr)"));
	ls.check_empty_stack();

	ls["o"] = myobj;

	QVERIFY(!myobj->_ud.valid());
	ls.check_empty_stack();

	ls.exec_statements("o:ud_slot(v)");

	QCOMPARE(myobj->_ud.dynamiccast<MyData>()->_data, 18.0);
	ls.check_empty_stack();
}

void QObjectArgs::test2()
{
	QtLua::State ls;

	MyObjectQO *myobj = new MyObjectQO();

	ls.exec_statements("function f(obj, qo) v = qo; end");
	ls.check_empty_stack();

	QVERIFY(ls.at("f").connect(myobj, "qo_arg(QObject*)"));
	ls.check_empty_stack();

	QCOMPARE(ls.at("v").type(), QtLua::Value::TNil);
	ls.check_empty_stack();

	QObject *qo = new QObject();
	qo->setObjectName("qo");
	myobj->send(qo);

	QCOMPARE(ls.at("v").type(), QtLua::Value::TUserData);
	ls.check_empty_stack();

	QCOMPARE(ls.at("v").at("objectName").to_string().constData(), "qo");
	ls.check_empty_stack();

	//    ASSERT(ls["f"].disconnect(myobj, "qo_arg(QtLua::UserData::ptr)"));

	ls["o"] = myobj;

	QVERIFY(!myobj->_qo);
	ls.exec_statements("o:qo_slot(v)");
	QCOMPARE(myobj->_qo, qo);
}

void QObjectArgs::test3()
{
	QtLua::State ls;

	ls.openlib(QtLua::QtLib);
	ls.register_qobject_meta<MyObjectUD>();

	QtLua::Value::List r = ls.exec_statements("a = qt.new_qobject(qt.meta.MyObjectUD, 42, nil); return a;");
	QCOMPARE(r[0].type(), QtLua::Value::TUserData);

	r = ls.exec_statements("return a:foo(2)");
	QCOMPARE(r[0].to_number(), 84.0);
}

QTEST_APPLESS_MAIN(QObjectArgs)

#include "tst_qobject_arg.moc"
