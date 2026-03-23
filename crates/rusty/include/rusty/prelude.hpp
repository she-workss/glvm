#pragma once

#include "cast.hpp"
#include "concepts.hpp"
#include "io.hpp"
#include "lib.hpp"
#include "macros.hpp"
#include "match.hpp"
#include "option.hpp"
#include "result.hpp"
#include "types.hpp"

#include <cassert>

namespace rusty::prelude {
using namespace rusty::types;
using namespace rusty::io;
using namespace rusty::option;
using namespace rusty::result;
using namespace rusty::cast;
using namespace rusty::concepts;
using namespace rusty::match;
using namespace rusty::macros;
using rusty::make_box;
using rusty::make_rc;
using rusty::PhantomData;
} // namespace rusty::prelude
