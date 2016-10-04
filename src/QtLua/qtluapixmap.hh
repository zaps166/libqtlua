#ifndef QTLUAPIXMAP_HH_
#define QTLUAPIXMAP_HH_

#include "qtluauserdata.hh"

namespace QtLua {

  class Pixmap : public UserData
  {
  public:
    QTLUA_REFTYPE(Pixmap)

    QPixmap _pixmap;
  };

}

#endif
