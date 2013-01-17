
#include "NodeJSTestC.h"

namespace v8 {
  class Object;
}

namespace Mod {
  void copyToV8(v8::Object& target, const Sample& src);
}
