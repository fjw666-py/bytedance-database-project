#include "column_table.h"
#include <cstring>
#include <iostream>

namespace bytedance_db_project {
ColumnTable::ColumnTable() {}

ColumnTable::~ColumnTable() {
  if (columns_ != nullptr) {
    delete columns_;
    columns_ = nullptr;
  }
}

//
// columnTable, which stores data in column-major format.
// That is, data is laid out like
//   col 1 | col 2 | ... | col m.
//
void ColumnTable::Load(BaseDataLoader *loader) {
  num_cols_ = loader->GetNumCols();
  std::vector<char *> rows = loader->GetRows();
  num_rows_ = rows.size();
  columns_ = new char[FIXED_FIELD_LEN * num_rows_ * num_cols_];
  for (int32_t row_id = 0; row_id < num_rows_; row_id++) {
    auto cur_row = rows.at(row_id);
    for (int32_t col_id = 0; col_id < num_cols_; col_id++) {
      int32_t offset = FIXED_FIELD_LEN * ((col_id * num_rows_) + row_id);
      std::memcpy(columns_ + offset, cur_row + FIXED_FIELD_LEN * col_id,
                  FIXED_FIELD_LEN);
    }
  }
}

int32_t ColumnTable::GetIntField(int32_t row_id, int32_t col_id) {
  int32_t res = *(int32_t *)(columns_+FIXED_FIELD_LEN*(col_id * num_rows_ + row_id));
  return res;
}

void ColumnTable::PutIntField(int32_t row_id, int32_t col_id, int32_t field) {
  // TODO: Implement this!
  *(int32_t *)(columns_+FIXED_FIELD_LEN*(col_id * num_rows_ + row_id)) = field;
}

int64_t ColumnTable::ColumnSum() {
  int64_t res=0;
  for(int32_t row_id = 0; row_id < num_rows_; row_id++){
    res += GetIntField(row_id,0); 
  }
  return res;
}

int64_t ColumnTable::PredicatedColumnSum(int32_t threshold1,
                                         int32_t threshold2) {
  int64_t res=0;
  char *col1 = columns_+FIXED_FIELD_LEN*num_rows_;
  char *col2 = columns_+FIXED_FIELD_LEN*num_rows_*2;
  for(int32_t row = 0; row<num_rows_; row++){
    if(*(int32_t *)(col1 + row*FIXED_FIELD_LEN) > threshold1 && *(int32_t *)(col2 + row*FIXED_FIELD_LEN)){
      res+=GetIntField(row,0);
    }
  }
  return res;
}

int64_t ColumnTable::PredicatedAllColumnsSum(int32_t threshold) {
  // TODO: Implement this!
  int64_t res = 0;
  std::vector<int32_t> row_id; 
  for(int32_t row = 0; row < num_rows_; row++){
    if(GetIntField(row,0) > threshold){
      row_id.push_back(row);
    }
  }
  for(int32_t col_id = 0; col_id < num_cols_; col_id++){
    for(long unsigned int i = 0; i < row_id.size(); i++){
      res += GetIntField(row_id.at(i),col_id);
    }  
  }
  return res;
}

int64_t ColumnTable::PredicatedUpdate(int32_t threshold) {
  // TODO: Implement this!
  int64_t res=0;
  for(int32_t row = 0; row<num_rows_; row++){
    if(*(int32_t *)(columns_ + FIXED_FIELD_LEN * row) <  threshold){
      PutIntField(row,3,GetIntField(row,3)+GetIntField(row,2));
      res++;
    }
  }
  return res;
}
} // namespace bytedance_db_project