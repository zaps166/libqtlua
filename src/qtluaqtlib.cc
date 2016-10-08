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

    Copyright (C) 2008-2013, Alexandre Becoulet <alexandre.becoulet@free.fr>

*/

#include <QFile>
#include <QWidget>

#include <QLayout>
#include <QBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QColorDialog>
#include <QFileDialog>
#include <QErrorMessage>
#include <QInputDialog>
#include <QMessageBox>
#include <QApplication>
#include <QTranslator>
#include <QActionGroup>
#include <QMainWindow>
#include <QDockWidget>
#include <QToolBar>
#include <QStackedWidget>
#include <QToolBar>
#include <QScrollArea>
#include <QSplitter>
#include <QMdiArea>

#if QT_VERSION < 0x050000
#include <QWorkspace>
#endif

#include <QAbstractItemView>
#include <QComboBox>

#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QDebug>

#include <QtLua/State>
#include <QtLua/Function>
#include <QtLua/Pixmap>
#include <QtLua/QHashProxy>

#include <internal/Method>
#include <internal/MetaCache>
#include <internal/QMetaObjectWrapper>
#include <internal/QObjectWrapper>
#include <internal/qtluaqtlib.hh>

namespace QtLua {

typedef QMap<String, QMetaObjectWrapper> qmetaobject_table_t;

class QMetaObjectTable
	: public QHashProxyRo<qmetaobject_table_t>,
	  public QObject
{
public:
	QMetaObjectTable()
		: QHashProxyRo<qmetaobject_table_t>(_mo_table)
	{
		for (const meta_object_table_s *me = meta_object_table; me->_mo; me++)
		{
			String name(me->_mo->className());
			name.replace(':', '_');
			_mo_table.insert(name, QMetaObjectWrapper(me->_mo, me->_creator));
		}

		_mo_table.insert("Qt", QMetaObjectWrapper(&staticQtMetaObject));
		_mo_table.insert("QSizePolicy", QMetaObjectWrapper(&QSizePolicy::staticMetaObject));
	}

	qmetaobject_table_t _mo_table;
};

static QMetaObjectTable qt_meta;

void qtlib_register_meta(const QMetaObject *mo, qobject_creator *creator)
{
	String name(mo->className());
	name.replace(':', '_');
	qt_meta._mo_table.insert(name, QMetaObjectWrapper(mo, creator));
}


////////////////////////////////////////////////// qobjects


QTLUA_FUNCTION(connect)
{
	Q_UNUSED(ls)
	meta_call_check_args(args, 3, 4, Value::TUserData, Value::TString, Value::TNone, Value::TString);

	QObjectWrapper::ptr sigqow = args[0].to_userdata_cast<QObjectWrapper>();

	String signame = args[1].to_string();
	QObject &sigobj = sigqow->get_object();

	int sigindex = sigobj.metaObject()->indexOfSignal(signame.constData());
	if (sigindex < 0)
		QTLUA_THROW(qt.connect, "No such signal '%'.", .arg(signame));

	switch (args.size())
	{
	case 3:
	{
		// connect qt signal to lua function
		sigqow->_lua_connect(sigindex, args[2]);
		break;
	}

	case 4:
	{
		// connect qt signal to qt slot
		String slotname = args[3].to_string();
		QObject &sloobj = args[2].to_userdata_cast<QObjectWrapper>()->get_object();

		int slotindex = sloobj.metaObject()->indexOfSlot(slotname.constData());
		if (slotindex < 0)
			QTLUA_THROW(qt.connect, "No such slot '%'.", .arg(slotname));

		if (!QMetaObject::checkConnectArgs(signame.constData(), slotname.constData()))
			QTLUA_THROW(qt.connect, "Incompatible argument types between signal '%' and slot '%'.",
						.arg(signame.constData()).arg(slotname.constData()));

		if (!QMetaObject::connect(&sigobj, sigindex, &sloobj, slotindex))
			QTLUA_THROW(qt.connect, "Unable to connect signal to slot.");
	}
	}

	return Value::List();
}


QTLUA_FUNCTION(disconnect)
{
	meta_call_check_args(args, 2, 4, Value::TUserData, Value::TString, Value::TNone, Value::TString);

	QObjectWrapper::ptr sigqow = args[0].to_userdata_cast<QObjectWrapper>();

	String signame = args[1].to_string();
	QObject &sigobj = sigqow->get_object();

	int sigindex = sigobj.metaObject()->indexOfSignal(signame.constData());
	if (sigindex < 0)
		QTLUA_THROW(qt.disconnect, "No such signal '%'.", .arg(signame));

	switch (args.size())
	{
	case 2:
		// disconnect qt signal from all lua functions
		sigqow->_lua_disconnect_all(sigindex);
		return Value::List();

	case 3:
		// disconnect qt signal from lua function
		return Value(ls, (Value::Bool)sigqow->_lua_disconnect(sigindex, args[2]));

	case 4:
	{
		// disconnect qt signal from qt slot
		String slotname = args[3].to_string();
		QObject &sloobj = args[2].to_userdata_cast<QObjectWrapper>()->get_object();

		int slotindex = sloobj.metaObject()->indexOfSlot(slotname.constData());
		if (slotindex < 0)
			QTLUA_THROW(qt.disconnect, "No such slot '%'.", .arg(slotname));

		return Value(ls, (Value::Bool)QMetaObject::disconnect(&sigobj, sigindex, &sloobj, slotindex));
	}
	}

	abort();
}


QTLUA_FUNCTION(connect_slots_by_name)
{
	QMetaObject::connectSlotsByName(get_arg_qobject<QObject>(args, 0));

	return Value(ls);
}

QTLUA_FUNCTION(meta_type)
{
	meta_call_check_args(args, 1, 1, Value::TNone);

	switch (args[0].type())
	{
	case Value::TString:
	{
		String n(args[0].to_string());
		if (int t = QMetaType::type(n.constData()))
			return Value(ls, t);
		QTLUA_THROW(qt.meta_type, "Unable to resolve Qt meta type '%'.", .arg(n));
	}

	case Value::TNumber:
	{
		int t = args[0].to_integer();
		if (const char *n = QMetaType::typeName(t))
			return Value(ls, n);
		QTLUA_THROW(qt.meta_type, "Unable to resolve Qt meta type handle '%'.", .arg(t));
	}

	default:
		QTLUA_THROW(qt.meta_type, "Bad argument type, string or number expected.");
		break;
	}
}


QTLUA_FUNCTION(new_qobject)
{
	QMetaObjectWrapper::ptr mow = get_arg_ud<QMetaObjectWrapper>(args, 0);

	return Value(ls, mow->create(args), true, true);
}


////////////////////////////////////////////////// ui


QTLUA_FUNCTION(layout_add)
{
	meta_call_check_args(args, 2, 0, Value::TUserData, Value::TNone);

	QObject *obj = get_arg_qobject<QObject>(args, 0);

	if (QFormLayout *la = dynamic_cast<QFormLayout *>(obj))
	{
		if (args[1].type() == Value::TString)
		{
			QObjectWrapper::ptr qow2 = get_arg_ud<QObjectWrapper>(args, 2);
			QObject &item2 = qow2->get_object();

			// QFormLayout::addRow ( const QString & labelText, QLayout * field )
			if (QLayout *li = dynamic_cast<QLayout *>(&item2))
			{
				qow2->set_delete(false);
				la->addRow(args[1].to_string(), li);
			}

			// QFormLayout::addRow ( const QString & labelText, QWidget * field )
			else if (QWidget *w2 = dynamic_cast<QWidget *>(&item2))
			{
				if (QLayout *ol = w2->layout())
					ol->removeWidget(w2);
				la->addRow(args[1].to_string(), w2);
			}
			else
				goto err;

			return QtLua::Value(ls);
		}
		else
		{
			QObjectWrapper::ptr qow = get_arg_ud<QObjectWrapper>(args, 1);
			QObject &item = qow->get_object();

			int row = get_arg<int>(args, 2);
			int col = get_arg<int>(args, 3);
			int col_span = get_arg<int>(args, 4, 1);
			if (col + col_span > 2)
				QTLUA_THROW(qt.ui.layout_add, "Bad QFormLayout spanning.");

			QFormLayout::ItemRole role = (col_span > 1 ? QFormLayout::SpanningRole : col ? QFormLayout::FieldRole : QFormLayout::LabelRole);

			// QFormLayout::setLayout ( int row, ItemRole role, QLayout * layout )
			if (QLayout *li = dynamic_cast<QLayout *>(&item))
			{
				qow->set_delete(false);
				la->setLayout(row, role, li);
			}

			// QFormLayout::setWidget ( int row, ItemRole role, QWidget * widget )
			else if (QWidget *w = dynamic_cast<QWidget *>(&item))
			{
				if (QLayout *ol = w->layout())
					ol->removeWidget(w);
				la->setWidget(row, role, w);
			}
			else
				goto err;

			return QtLua::Value(ls);
		}
	}

	if (QGridLayout *la = dynamic_cast<QGridLayout *>(obj))
	{
		QObjectWrapper::ptr qow = get_arg_ud<QObjectWrapper>(args, 1);
		QObject &item = qow->get_object();

		int row = get_arg<int>(args, 2);
		int col = get_arg<int>(args, 3);
		int row_span = get_arg<int>(args, 4, 1);
		int col_span = get_arg<int>(args, 5, 1);
		int align = get_arg<int>(args, 6, 0);

		// QGridLayout::addLayout ( QLayout * layout, int row, int column, int rowSpan, int columnSpan, Qt::Alignment alignment )
		if (QLayout *li = dynamic_cast<QLayout *>(&item))
		{
			qow->set_delete(false);
			la->addLayout(li, row, col, row_span, col_span, (Qt::Alignment)align);
		}

		// QGridLayout::addWidget ( QWidget * widget, int row, int column, int rowSpan, int columnSpan, Qt::Alignment alignment )
		else if (QWidget *w = dynamic_cast<QWidget *>(&item))
		{
			if (QLayout *ol = w->layout())
				ol->removeWidget(w);
			la->addWidget(w, row, col, row_span, col_span, (Qt::Alignment)align);
		}
		else
			goto err;

		return QtLua::Value(ls);
	}

	if (QBoxLayout *la = dynamic_cast<QBoxLayout *>(obj))
	{
		QObjectWrapper::ptr qow = get_arg_ud<QObjectWrapper>(args, 1);
		QObject &item = qow->get_object();

		if (QLayout *li = dynamic_cast<QLayout *>(&item))
		{
			qow->set_delete(false);
			la->addLayout(li);
		}

		else if (QWidget *w = dynamic_cast<QWidget *>(&item))
		{
			if (QLayout *ol = w->layout())
				ol->removeWidget(w);
			la->addWidget(w);
		}
		else
			goto err;

		return QtLua::Value(ls);
	}

	if (QWidget *w = dynamic_cast<QWidget *>(obj))
	{
		QLayout *la = get_arg_qobject<QLayout>(args, 1);
		delete w->layout();
		w->setLayout(la);

		return QtLua::Value(ls);
	}

err:
	QTLUA_THROW(qt.ui.layout_add, "Bad object type.");
}


QTLUA_FUNCTION(layout_spacer)
{
	meta_call_check_args(args, 3, 5, Value::TUserData, Value::TNumber, Value::TNumber, Value::TNumber, Value::TNumber);

	QLayout *la = args[0].to_qobject_cast<QLayout>();

	la->addItem(new QSpacerItem(get_arg<int>(args, 1),
								get_arg<int>(args, 2),
								(QSizePolicy::Policy)get_arg<int>(args, 3, QSizePolicy::Minimum),
								(QSizePolicy::Policy)get_arg<int>(args, 4, QSizePolicy::Minimum)));

	return QtLua::Value(ls);
}

QTLUA_FUNCTION(get_viewport)
{
	return QtLua::Value(ls, get_arg_qobject<QAbstractScrollArea>(args, 0)->viewport());
}

////////////////////////////////////////////////// translation


QTLUA_FUNCTION(tr)
{
	return Value(ls, QCoreApplication::translate(get_arg<String>(args, 0),
												 get_arg<String>(args, 1),
												 get_arg<String>(args, 2, ""),
#if QT_VERSION < 0x050000
												 QCoreApplication::UnicodeUTF8,
#endif
												 get_arg<int>(args, 3, -1)));
}


QTLUA_FUNCTION(translator)
{
	String filename(get_arg<String>(args, 0));
	QTranslator *qtr = new QTranslator();

	if (!qtr->load(filename))
	{
		delete qtr;
		QTLUA_THROW(qt.translator, "Unable to load the translation file '%'", .arg(filename));
	}

	QCoreApplication::installTranslator(qtr);
	return Value(ls, qtr, true, true);
}


////////////////////////////////////////////////// menus


QTLUA_FUNCTION(add_menu)
{
	meta_call_check_args(args, 2, 3, Value::TUserData, Value::TString, Value::TString);

	QObject *obj = args[0].to_qobject();
	String text = args[1].to_string();
	QObject *result;

	if (QMenu *menu = dynamic_cast<QMenu *>(obj))
		result = menu->addMenu(text);
	else if (QMenuBar *menubar = dynamic_cast<QMenuBar *>(obj))
		result = menubar->addMenu(text);
	else
		QTLUA_THROW(qt.ui.menu.add_menu, "Bad object type.");

	if (args.size() > 2)
		result->setObjectName(args[2]);

	return QtLua::Value(ls, result, true, true);
}


QTLUA_FUNCTION(add_separator)
{
	meta_call_check_args(args, 1, 2, Value::TUserData, Value::TString);

	QObject *obj = args[0].to_qobject();
	QObject *result;

	if (QMenu *menu = dynamic_cast<QMenu *>(obj))
		result = menu->addSeparator();
	else if (QToolBar *tb = dynamic_cast<QToolBar *>(obj))
		result = tb->addSeparator();
	else
		QTLUA_THROW(qt.ui.menu.add_separator, "Bad object type.");

	if (args.size() > 1)
		result->setObjectName(args[1]);

	return QtLua::Value(ls, result, true, true);
}


QTLUA_FUNCTION(add_action)
{
	meta_call_check_args(args, 2, 3, Value::TUserData, Value::TNone, Value::TString);

	QObject *obj = args[0].to_qobject();
	String text = args[1].to_string();
	QObject *result;

	if (QMenu *menu = dynamic_cast<QMenu *>(obj))
		result = menu->addAction(text);
	else if (QMenuBar *menubar = dynamic_cast<QMenuBar *>(obj))
		result = menubar->addAction(text);
	else if (QActionGroup *group = dynamic_cast<QActionGroup *>(obj))
		result = group->addAction(text);
	else if (QToolBar *tb = dynamic_cast<QToolBar *>(obj))
		result = tb->addAction(text);
	else
		QTLUA_THROW(qt.ui.menu.add_action, "Bad object type.");

	if (args.size() > 2)
		result->setObjectName(args[2].to_string());

	return QtLua::Value(ls, result, true, true);
}

QTLUA_FUNCTION(menu_attach)
{
	QObject *obj = get_arg_qobject<QObject>(args, 0);
	QObject *obj2 = get_arg_qobject<QObject>(args, 1);

	if (QAction *action = dynamic_cast<QAction *>(obj2))
	{
		if (QMenu *menu = dynamic_cast<QMenu *>(obj))
			menu->addAction(action);
		else if (QMenuBar *menubar = dynamic_cast<QMenuBar *>(obj))
			menubar->addAction(action);
		else if (QActionGroup *group = dynamic_cast<QActionGroup *>(obj))
			group->addAction(action);
		else if (QToolBar *tb = dynamic_cast<QToolBar *>(obj))
			tb->addAction(action);
		else
			goto err;
	}
	else if (QMenu *submenu = dynamic_cast<QMenu *>(obj2))
	{
		if (QMenu *menu = dynamic_cast<QMenu *>(obj))
			menu->addAction(submenu->menuAction());
		else if (QMenuBar *menubar = dynamic_cast<QMenuBar *>(obj))
			menubar->addAction(submenu->menuAction());
		else
			goto err;
	}
	else
		goto err;

	return QtLua::Value(ls);
err:
	QTLUA_THROW(qt.ui.menu.attach, "Can not attach a '%' object to a '%' object.",
				.arg(obj2->metaObject()->className())
					.arg(obj->metaObject()->className()));
}


QTLUA_FUNCTION(new_action_group)
{
	QAction *a[args.size()];

	for (int i = 0; i < args.size(); i++)
		a[i] = args[i].to_qobject_cast<QAction>();

	QActionGroup *result = new QActionGroup(0);
	for (int i = 0; i < args.size(); i++)
		result->addAction(a[i]);

	return QtLua::Value(ls, result, true, true);
}

QTLUA_FUNCTION(new_action)
{
	return QtLua::Value(ls, new QAction(get_arg_qobject<QObject>(args, 0)), true, true);
}

QTLUA_FUNCTION(new_menu)
{
	return QtLua::Value(ls, new QMenu(get_arg_qobject<QWidget>(args, 0)), true, true);
}

QTLUA_FUNCTION(remove)
{
	meta_call_check_args(args, 1, 2, Value::TUserData, Value::TUserData);

	QObject *obj = args[0].to_qobject();
	QObject *pobj;
	QAction *action;
	QMenu *menu = 0;

	if (args.size() > 1)
		pobj = args[1].to_qobject();
	else
		pobj = obj->parent();

	if ((action = dynamic_cast<QAction *>(obj)))
		;
	else if ((menu = dynamic_cast<QMenu *>(obj)))
		action = menu->menuAction();
	else
		QTLUA_THROW(qt.ui.menu.remove, "Bad object type.");

	if (QWidget *w = dynamic_cast<QWidget *>(pobj))
		w->removeAction(action);
	else if (QActionGroup *group = dynamic_cast<QActionGroup *>(pobj))
		group->removeAction(action);
	else
		QTLUA_THROW(qt.ui.menu.remove, "Bad QWidget object type.");

	return QtLua::Value(ls);
}

////////////////////////////////////////////////// main window

QTLUA_FUNCTION(ui_attach)
{
	QObject *obj = get_arg_qobject<QObject>(args, 0);
	QObject *obj2 = get_arg_qobject<QObject>(args, 1);

	if (QMainWindow *mainwin = dynamic_cast<QMainWindow *>(obj))
	{
		if (QMenuBar *menubar = dynamic_cast<QMenuBar *>(obj2))
			mainwin->setMenuBar(menubar);
		else if (QStatusBar *statusbar = dynamic_cast<QStatusBar *>(obj2))
			mainwin->setStatusBar(statusbar);
		else if (QToolBar *toolbar = dynamic_cast<QToolBar *>(obj2))
			mainwin->addToolBar(toolbar);
		else if (QDockWidget *dw = dynamic_cast<QDockWidget *>(obj2))
			mainwin->addDockWidget((Qt::DockWidgetArea)get_arg<int>(args, 2, Qt::LeftDockWidgetArea), dw);
		else if (QWidget *w = dynamic_cast<QWidget *>(obj2))
			mainwin->setCentralWidget(w);
		else
			goto err;
	}
	else if (QWidget *w = dynamic_cast<QWidget *>(obj2))
	{
		if (QDockWidget *dw = dynamic_cast<QDockWidget *>(obj))
			dw->setWidget(w);
		else if (QStackedWidget *x = dynamic_cast<QStackedWidget *>(obj))
			x->addWidget(w);
		else if (QToolBar *x = dynamic_cast<QToolBar *>(obj))
			x->addWidget(w);
		else if (QScrollArea *x = dynamic_cast<QScrollArea *>(obj))
			x->setWidget(w);
		else if (QSplitter *x = dynamic_cast<QSplitter *>(obj))
			x->addWidget(w);
		else if (QMdiArea *x = dynamic_cast<QMdiArea *>(obj))
			x->addSubWindow(w);
#if QT_VERSION < 0x050000
		else if (QWorkspace *x = dynamic_cast<QWorkspace *>(obj))
			x->addWindow(w);
#endif
		else
			goto err;
	}
	else
		goto err;

	return QtLua::Value(ls);
err:
	QTLUA_THROW(qt.ui.attach, "Can not attach a '%' to a '%' object.",
				.arg(obj2->metaObject()->className())
					.arg(obj->metaObject()->className()));
}

////////////////////////////////////////////////// dialogs

QTLUA_FUNCTION(get_existing_directory)
{
	return Value(ls, QFileDialog::getExistingDirectory(QApplication::activeWindow(),
													   get_arg<QString>(args, 0, ""),
													   get_arg<QString>(args, 1, ""),
													   (QFileDialog::Option)get_arg<int>(args, 2, QFileDialog::ShowDirsOnly)));
}


QTLUA_FUNCTION(get_open_filename)
{
	return Value(ls, QFileDialog::getOpenFileName(QApplication::activeWindow(),
												  get_arg<QString>(args, 0, ""),
												  get_arg<QString>(args, 1, ""),
												  get_arg<QString>(args, 2, ""), 0,
												  (QFileDialog::Option)get_arg<int>(args, 3, 0)));
}

QTLUA_FUNCTION(get_open_filenames)
{
	return Value(ls, QFileDialog::getOpenFileNames(QApplication::activeWindow(),
												   get_arg<QString>(args, 0, ""),
												   get_arg<QString>(args, 1, ""),
												   get_arg<QString>(args, 2, ""), 0,
												   (QFileDialog::Option)get_arg<int>(args, 3, 0)));
}


QTLUA_FUNCTION(get_save_filename)
{
	return Value(ls, QFileDialog::getSaveFileName(QApplication::activeWindow(),
												  get_arg<QString>(args, 0, ""),
												  get_arg<QString>(args, 1, ""),
												  get_arg<QString>(args, 2, ""), 0,
												  (QFileDialog::Option)get_arg<int>(args, 3, 0)));
}


QTLUA_FUNCTION(get_color)
{
	QColor init(Qt::white);

	if (args.count() >= 3)
		init = QColor(get_arg<int>(args, 0, 0), get_arg<int>(args, 1, 0), get_arg<int>(args, 2, 0));

	QColor c = QColorDialog::getColor(init, QApplication::activeWindow());

	return c.isValid() ? Value::List(Value(ls, c.red()), Value(ls, c.green()), Value(ls, c.blue()))
					   : Value::List();
}


QTLUA_FUNCTION(get_double)
{
	bool ok;
	double v = QInputDialog::getDouble(QApplication::activeWindow(),
									   get_arg<QString>(args, 0, ""),
									   get_arg<QString>(args, 1, ""),
									   get_arg<double>(args, 2, 0),
									   get_arg<double>(args, 3, -2147483647),
									   get_arg<double>(args, 4, 2147483647),
									   get_arg<int>(args, 5, 1),
									   &ok);
	return ok ? Value(ls, v) : Value(ls);
}


QTLUA_FUNCTION(get_integer)
{
	bool ok;
#if QT_VERSION < 0x050000
	int v = QInputDialog::getInteger(QApplication::activeWindow(),
#else
	int v = QInputDialog::getInt(QApplication::activeWindow(),
#endif
									 get_arg<QString>(args, 0, ""),
									 get_arg<QString>(args, 1, ""),
									 get_arg<int>(args, 2, 0),
									 get_arg<int>(args, 3, -2147483647),
									 get_arg<int>(args, 4, 2147483647),
									 get_arg<int>(args, 5, 1),
									 &ok);
	return ok ? Value(ls, v) : Value(ls);
}


QTLUA_FUNCTION(get_text)
{
	bool ok;
	QString v = QInputDialog::getText(QApplication::activeWindow(),
									  get_arg<QString>(args, 0, ""),
									  get_arg<QString>(args, 1, ""),
									  QLineEdit::Normal,
									  get_arg<QString>(args, 2, ""),
									  &ok);
	return ok ? Value(ls, v) : Value(ls);
}


QTLUA_FUNCTION(get_item)
{
	bool ok;
	QString v = QInputDialog::getItem(QApplication::activeWindow(),
									  get_arg<QString>(args, 3, ""),
									  get_arg<QString>(args, 4, ""),
									  get_arg<QList<QString> >(args, 0),
									  get_arg<int>(args, 1, 0),
									  get_arg<Value::Bool>(args, 2, Value::False),
									  &ok);
	return ok ? Value(ls, v) : Value(ls);
}


QTLUA_FUNCTION(msg_about)
{
	QMessageBox::about(QApplication::activeWindow(),
					   get_arg<QString>(args, 1, ""),
					   get_arg<QString>(args, 0));
	return Value(ls);
}


QTLUA_FUNCTION(msg_critical)
{
	return Value(ls, QMessageBox::critical(QApplication::activeWindow(),
										   get_arg<QString>(args, 1, ""),
										   get_arg<QString>(args, 0),
										   (QMessageBox::StandardButtons)get_arg<int>(args, 2, QMessageBox::Ok),
										   (QMessageBox::StandardButton)get_arg<int>(args, 3, QMessageBox::NoButton)));
}


QTLUA_FUNCTION(msg_information)
{
	return Value(ls, QMessageBox::information(QApplication::activeWindow(),
											  get_arg<QString>(args, 1, ""),
											  get_arg<QString>(args, 0),
											  (QMessageBox::StandardButtons)get_arg<int>(args, 2, QMessageBox::Ok),
											  (QMessageBox::StandardButton)get_arg<int>(args, 3, QMessageBox::NoButton)));
}


QTLUA_FUNCTION(msg_question)
{
	return Value(ls, QMessageBox::question(QApplication::activeWindow(),
										   get_arg<QString>(args, 1, ""),
										   get_arg<QString>(args, 0),
										   (QMessageBox::StandardButtons)get_arg<int>(args, 2, QMessageBox::Ok),
										   (QMessageBox::StandardButton)get_arg<int>(args, 3, QMessageBox::NoButton)));
}


QTLUA_FUNCTION(msg_warning)
{
	return Value(ls, QMessageBox::warning(QApplication::activeWindow(),
										  get_arg<QString>(args, 1, ""),
										  get_arg<QString>(args, 0),
										  (QMessageBox::StandardButtons)get_arg<int>(args, 2, QMessageBox::Ok),
										  (QMessageBox::StandardButton)get_arg<int>(args, 3, QMessageBox::NoButton)));
}

////////////////////////////////////////////////// pixmap

QTLUA_FUNCTION(pixmap_file)
{
	meta_call_check_args(args, 1, 0, Value::TString);
	Pixmap::ptr pixmap = QTLUA_REFNEW(Pixmap);
	pixmap->load(get_arg<String>(args, 0));
	if (!pixmap->isNull())
		return Value(ls, pixmap);
	return Value(ls);
}

QTLUA_FUNCTION(pixmap_data)
{
	meta_call_check_args(args, 1, 0, Value::TString);
	Pixmap::ptr pixmap = QTLUA_REFNEW(Pixmap);
	pixmap->loadFromData(get_arg<String>(args, 0));
	if (!pixmap->isNull())
		return Value(ls, pixmap);
	return Value(ls);
}


//////////////////////////////////////////////////

void qtluaopen_qt(State *ls)
{
	ls->set_global("qt.meta", Value(ls, qt_meta));

	QTLUA_FUNCTION_REGISTER(ls, "qt.", new_qobject);
	QTLUA_FUNCTION_REGISTER(ls, "qt.", connect);
	QTLUA_FUNCTION_REGISTER(ls, "qt.", connect_slots_by_name);
	QTLUA_FUNCTION_REGISTER(ls, "qt.", disconnect);
	QTLUA_FUNCTION_REGISTER(ls, "qt.", meta_type);

	QTLUA_FUNCTION_REGISTER(ls, "qt.", tr);
	QTLUA_FUNCTION_REGISTER(ls, "qt.", translator);

	QTLUA_FUNCTION_REGISTER(ls, "qt.ui.", layout_add);
	QTLUA_FUNCTION_REGISTER(ls, "qt.ui.", layout_spacer);
	QTLUA_FUNCTION_REGISTER(ls, "qt.ui.", get_viewport);
	QTLUA_FUNCTION_REGISTER2(ls, "qt.ui.attach", ui_attach);

	QTLUA_FUNCTION_REGISTER(ls, "qt.ui.menu.", add_menu);
	QTLUA_FUNCTION_REGISTER(ls, "qt.ui.menu.", add_separator);
	QTLUA_FUNCTION_REGISTER(ls, "qt.ui.menu.", add_action);
	QTLUA_FUNCTION_REGISTER2(ls, "qt.ui.menu.attach", menu_attach);
	QTLUA_FUNCTION_REGISTER(ls, "qt.ui.menu.", new_action_group);
	QTLUA_FUNCTION_REGISTER(ls, "qt.ui.menu.", new_action);
	QTLUA_FUNCTION_REGISTER(ls, "qt.ui.menu.", new_menu);
	QTLUA_FUNCTION_REGISTER(ls, "qt.ui.menu.", remove);

	QTLUA_FUNCTION_REGISTER(ls, "qt.dialog.", get_existing_directory);
	QTLUA_FUNCTION_REGISTER(ls, "qt.dialog.", get_open_filename);
	QTLUA_FUNCTION_REGISTER(ls, "qt.dialog.", get_open_filenames);
	QTLUA_FUNCTION_REGISTER(ls, "qt.dialog.", get_save_filename);
	QTLUA_FUNCTION_REGISTER(ls, "qt.dialog.", get_color);
	QTLUA_FUNCTION_REGISTER(ls, "qt.dialog.", get_double);
	QTLUA_FUNCTION_REGISTER(ls, "qt.dialog.", get_integer);
	QTLUA_FUNCTION_REGISTER(ls, "qt.dialog.", get_text);
	QTLUA_FUNCTION_REGISTER(ls, "qt.dialog.", get_item);
	QTLUA_FUNCTION_REGISTER(ls, "qt.dialog.", msg_about);
	QTLUA_FUNCTION_REGISTER(ls, "qt.dialog.", msg_critical);
	QTLUA_FUNCTION_REGISTER(ls, "qt.dialog.", msg_information);
	QTLUA_FUNCTION_REGISTER(ls, "qt.dialog.", msg_question);
	QTLUA_FUNCTION_REGISTER(ls, "qt.dialog.", msg_warning);

	QTLUA_FUNCTION_REGISTER2(ls, "qt.pixmap.from_file", pixmap_file);
	QTLUA_FUNCTION_REGISTER2(ls, "qt.pixmap.from_data", pixmap_data);
}

}
