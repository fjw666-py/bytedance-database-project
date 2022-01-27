/*****************************************************************************
 * Copyright (c) 2022, Bytedance Inc. All rights reserved.
 *
 * @file: csv_data_loader.cc
 *
 * @description: CsvDataLoader implementation.
 *
 * @created_at: 2021/12
 *
 * Managed by database team @ByteDance.
 *****************************************************************************/

#include "csv_data_loader.h"
#include "csv-parser/csv.h"

using namespace csv;

namespace bytedance_db_project {

CsvDataLoader::CsvDataLoader(std::string file_path, int32_t num_cols)
    : file_path_(file_path), num_cols_(num_cols) {}

CsvDataLoader::~CsvDataLoader() {

  //不是很懂！
  if (nullptr != rows_) {
    const int32_t num_rows = rows_->size();
    for (int32_t i = 0; i < num_rows; i++) {
      delete[] rows_->at(i);
    }
    delete rows_;
  }
}

int32_t CsvDataLoader::GetNumCols() { return num_cols_; }

std::vector<char *> CsvDataLoader::GetRows() {
  std::lock_guard<std::mutex> lock(mtx_);
  
  // already loaded, just return
  if (nullptr != rows_) {
    return *rows_;
  }

  // load from file
  rows_ = new std::vector<char *>();

  CSVFormat format;
  //设置分隔符号为","

  // no_header()
  // CSVFormat &no_header() {
  // this->header_row(-1);
  // return *this;
  // }  
  // header_row中的函数
  //if (row < 0)
  //this->variable_column_policy = VariableColumnPolicy::KEEP;
  //this->header = row;
  //this->col_names = {};
  //return *this;
  format.delimiter(',').no_header();

  CSVReader reader(file_path_, format);

  //读取每一行
  for (CSVRow &row : reader) {
    char *line = new char[FIXED_FIELD_LEN * num_cols_];
    auto offset = 0;
    //读取每一行中的每一列
    for (CSVField &field : row) {
      auto t = field.get<int32_t>();
      *(int32_t *)(line + offset) = t;
      offset += FIXED_FIELD_LEN;
    }
    rows_->emplace_back(line);
  }
  return *rows_;
}

} // namespace bytedance_db_project
