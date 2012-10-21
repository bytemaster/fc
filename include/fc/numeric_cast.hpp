#pragma once

namespace fc {
  template<typename T, typename R>
  T numeric_cast( const R& v ) {
    // TODO: do something smarter here, check ranges, etc
    return static_cast<T>(v);
  }
}
