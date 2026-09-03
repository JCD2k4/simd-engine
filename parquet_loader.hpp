#pragma once

#include <cstdint>
#include <string>
#include <vector>

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
std::vector<double> load_double_column(const std::string& path,
                                       const std::string& column,
                                       double null_value = 0.0,
                                       ColumnInfo* info = nullptr);

// Same, narrowed to float — the layout the SSE/AVX kernels want.
std::vector<float> load_float_column(const std::string& path,
                                     const std::string& column,
                                     float null_value = 0.0f,
                                     ColumnInfo* info = nullptr);

// Generic form of the two above. T must be one of the types explicitly
// instantiated in parquet_loader.cpp (see the extern template list below);
// anything else fails to link rather than to compile.
template <typename T>
std::vector<T> load_column(const std::string& path,
                           const std::string& column,
                           T null_value = T{},
                           ColumnInfo* info = nullptr);

extern template std::vector<double>       load_column<double>(const std::string&, const std::string&, double, ColumnInfo*);
extern template std::vector<float>        load_column<float>(const std::string&, const std::string&, float, ColumnInfo*);
extern template std::vector<std::int64_t> load_column<std::int64_t>(const std::string&, const std::string&, std::int64_t, ColumnInfo*);
extern template std::vector<std::int32_t> load_column<std::int32_t>(const std::string&, const std::string&, std::int32_t, ColumnInfo*);

}  // namespace pql
