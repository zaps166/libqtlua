#ifndef QTLUAPIXMAP_HH_
#define QTLUAPIXMAP_HH_

#include "QtLua/UserData"

#include <QPixmap>

namespace QtLua {

  class Pixmap :
      public UserData,
      public QPixmap
  {
  public:
    QTLUA_REFTYPE(Pixmap)

  private:
    Value meta_index(State *ls, const Value &key);
    bool support(Value::Operation c) const;
    virtual String get_value_str() const;
  };

}

#endif
