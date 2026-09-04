// Declarations for the pql.parquet_loader module.
//
// This file is textually included from parquet_loader.cppm inside the module
// purview, so these entities are attached to the module -- it is not a header
// you include to *use* the loader; `import pql.parquet_loader;` is.
//
// It exists as a separate file only so IntelliSense has something it can parse.
// ms-vscode.cpptools cannot read GCC's .gcm module files, so the benchmark
// sources include it directly when the editor defines IDE_INTELLISENSE.
#pragma once

// Inside the module purview `import std;` has already run, and #including a
// standard header there is ill-formed, so the includes are for the IDE path only.
#ifndef PQL_MODULE_PURVIEW
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>
#endif

namespace pql {

struct ColumnInfo {
    std::int64_t num_rows   = 0;    // values read, including nulls
    std::int64_t null_count = 0;    // how many of those were null (replaced by null_value)
};

// Names of every top-level column in the file, in schema order.
std::vector<std::string> list_columns(const std::string& path);

// Read one numeric column (DOUBLE / FLOAT / INT64 / INT32) into a flat, contiguous
// buffer. Nulls are replaced by null_value. Pass a ColumnInfo to find out how many
// rows came back and how many of them were null. Throws std::runtime_error on a
// missing file, unknown column name, or non-numeric column type.
std::vector<double>       load_double_column(const std::string& path, const std::string& column, double       null_value = 0.0,  ColumnInfo* info = nullptr);
std::vector<float>        load_float_column (const std::string& path, const std::string& column, float        null_value = 0.0f, ColumnInfo* info = nullptr);
std::vector<std::int64_t> load_int64_column (const std::string& path, const std::string& column, std::int64_t null_value = 0,    ColumnInfo* info = nullptr);
std::vector<std::int32_t> load_int32_column (const std::string& path, const std::string& column, std::int32_t null_value = 0,    ColumnInfo* info = nullptr);

// Generic form of the four above, for call sites that are themselves templates.
// Unsupported element types are a compile error here rather than a link error,
// which is what the old extern-template list in parquet_loader.hpp used to buy us.
template <typename T>
std::vector<T> load_column(const std::string& path, const std::string& column,
                           T null_value = T{}, ColumnInfo* info = nullptr) {
    if constexpr (std::is_same_v<T, double>)            return load_double_column(path, column, null_value, info);
    else if constexpr (std::is_same_v<T, float>)        return load_float_column (path, column, null_value, info);
    else if constexpr (std::is_same_v<T, std::int64_t>) return load_int64_column (path, column, null_value, info);
    else if constexpr (std::is_same_v<T, std::int32_t>) return load_int32_column (path, column, null_value, info);
    else static_assert(false, "pql::load_column: unsupported element type");
}

}  // namespace pql
