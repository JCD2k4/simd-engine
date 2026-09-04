// Implementation unit for pql.parquet_loader.
//
// This is the only translation unit that sees Arrow. Its headers are legacy
// (non-modular), so they go in the global module fragment above the module
// declaration; nothing they declare leaks into the module's interface.

// IDE_INTELLISENSE is defined only by the editor, which can't load the module's
// .gcm; it reads the declarations from pql_api.hpp instead. See parquet_loader.cppm.
#ifndef IDE_INTELLISENSE
module;
#endif

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/reader.h>

#ifndef IDE_INTELLISENSE
module pql.parquet_loader;
#else
#include "pql_api.hpp"
#endif

namespace pql {
namespace {

// Arrow reports failures as Status/Result rather than exceptions; turn them into ours.
void check(const arrow::Status& st, const std::string& what) {
    if (!st.ok()) throw std::runtime_error(what + ": " + st.ToString());
}

std::unique_ptr<parquet::arrow::FileReader> open_reader(const std::string& path) {
    auto file = arrow::io::ReadableFile::Open(path, arrow::default_memory_pool());
    check(file.status(), "open " + path);

    auto reader = parquet::arrow::OpenFile(*file, arrow::default_memory_pool());
    check(reader.status(), "read parquet metadata from " + path);
    return std::move(*reader);
}

int field_index(const arrow::Schema& schema, const std::string& column,
                const std::string& path) {
    const int idx = schema.GetFieldIndex(column);
    if (idx < 0) {
        std::string msg = "column '" + column + "' not found in " + path + "; have:";
        for (const auto& f : schema.fields()) msg += " " + f->name();
        throw std::runtime_error(msg);
    }
    return idx;
}

// Copy one chunk's values into out, substituting null_value wherever the validity
// bitmap says the slot is null. ArrowArrayT is the concrete array class (e.g.
// arrow::DoubleArray) whose raw_values() we can walk directly.
template <typename OutT, typename ArrowArrayT>
void append_chunk(const arrow::Array& array, OutT null_value, std::vector<OutT>& out) {
    const auto& typed = static_cast<const ArrowArrayT&>(array);
    const auto* src = typed.raw_values();
    const int64_t n = typed.length();
    const int64_t start = static_cast<int64_t>(out.size());
    out.resize(start + n);

    if (typed.null_count() == 0) {
        // Dense chunk: straight copy (a memcpy when OutT matches the stored type).
        for (int64_t i = 0; i < n; ++i) out[start + i] = static_cast<OutT>(src[i]);
    } else {
        for (int64_t i = 0; i < n; ++i) {
            out[start + i] = typed.IsNull(i) ? null_value : static_cast<OutT>(src[i]);
        }
    }
}

// Read `column` out of `path` as a contiguous vector<T>. Internal to this unit;
// the four load_*_column entry points below are what the module exports.
template <typename T>
std::vector<T> read_column(const std::string& path, const std::string& column,
                           T null_value, ColumnInfo* info) {
    auto reader = open_reader(path);

    std::shared_ptr<arrow::Schema> schema;
    check(reader->GetSchema(&schema), "read schema from " + path);
    const int idx = field_index(*schema, column, path);

    std::shared_ptr<arrow::ChunkedArray> chunked;
    check(reader->ReadColumn(idx, &chunked), "read column '" + column + "'");

    std::vector<T> out;
    out.reserve(static_cast<size_t>(chunked->length()));

    for (const auto& chunk : chunked->chunks()) {
        switch (chunk->type_id()) {
            case arrow::Type::DOUBLE: append_chunk<T, arrow::DoubleArray>(*chunk, null_value, out); break;
            case arrow::Type::FLOAT:  append_chunk<T, arrow::FloatArray>(*chunk, null_value, out);  break;
            case arrow::Type::INT64:  append_chunk<T, arrow::Int64Array>(*chunk, null_value, out);  break;
            case arrow::Type::INT32:  append_chunk<T, arrow::Int32Array>(*chunk, null_value, out);  break;
            default:
                throw std::runtime_error("column '" + column + "' has non-numeric type " +
                                         chunk->type()->ToString());
        }
    }

    if (info) {
        info->num_rows   = chunked->length();
        info->null_count = chunked->null_count();
    }
    return out;
}

}  // namespace

std::vector<std::string> list_columns(const std::string& path) {
    auto reader = open_reader(path);
    std::shared_ptr<arrow::Schema> schema;
    check(reader->GetSchema(&schema), "read schema from " + path);

    std::vector<std::string> names;
    names.reserve(schema->fields().size());
    for (const auto& f : schema->fields()) names.push_back(f->name());
    return names;
}

std::vector<double> load_double_column(const std::string& path, const std::string& column,
                                       double null_value, ColumnInfo* info) {
    return read_column<double>(path, column, null_value, info);
}

std::vector<float> load_float_column(const std::string& path, const std::string& column,
                                     float null_value, ColumnInfo* info) {
    return read_column<float>(path, column, null_value, info);
}

std::vector<std::int64_t> load_int64_column(const std::string& path, const std::string& column,
                                            std::int64_t null_value, ColumnInfo* info) {
    return read_column<std::int64_t>(path, column, null_value, info);
}

std::vector<std::int32_t> load_int32_column(const std::string& path, const std::string& column,
                                            std::int32_t null_value, ColumnInfo* info) {
    return read_column<std::int32_t>(path, column, null_value, info);
}

}  // namespace pql
