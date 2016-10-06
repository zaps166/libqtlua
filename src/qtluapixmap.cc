#include "internal/qtluapixmap.hh"
#include "QtLua/Function"

namespace QtLua {

  QTLUA_FUNCTION(pixmap_scale)
  {
    Q_UNUSED(ls)
    meta_call_check_args(args, 4, 0, Value::TUserData, Value::TTable, Value::TNumber, Value::TNumber);
    Pixmap::ptr pixmap = get_arg_ud<Pixmap>(args, 0);
    Value size = get_arg<Value>(args, 1);
    static_cast<QPixmap &>(*pixmap) = pixmap->scaled(size.at(1).to_integer(), size.at(2).to_integer(), (Qt::AspectRatioMode)get_arg<int>(args, 2), (Qt::TransformationMode)get_arg<int>(args, 3));
    return Value();
  }
  static QtLua_Function_pixmap_scale pixmap_scale;

  Value Pixmap::meta_index(State *ls, const Value &key)
  {
    if (key == "scale")
      return Value(ls, pixmap_scale);
    else if (key == "size")
      return Value(ls, size());
    return Value();
  }

}
