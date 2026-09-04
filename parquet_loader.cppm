// Module interface for the parquet loader.
//
// Deliberately Arrow-free: nothing in this file names an Arrow type, so importers
// don't pay for Arrow's headers and don't need its include path. All the Arrow
// work lives in the implementation unit, parquet_loader_impl.cpp.

// The declarations live in pql_api.hpp so IntelliSense -- which can't read GCC's
// .gcm files -- has something parseable. In a real build it is included in the
// module purview, so the entities are attached to this module exactly as if they
// were written out below. IDE_INTELLISENSE is defined only by the editor.
#ifndef IDE_INTELLISENSE

export module pql.parquet_loader;

import std;

#define PQL_MODULE_PURVIEW 1
export {
#include "pql_api.hpp"
}

#else
#include "pql_api.hpp"
#endif
