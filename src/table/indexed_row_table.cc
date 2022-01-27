#include "indexed_row_table.h"
#include <cstring>
namespace bytedance_db_project {
IndexedRowTable::IndexedRowTable(int32_t index_column) {
  index_column_ = index_column;
}

void IndexedRowTable::Load(BaseDataLoader *loader) {
  num_cols_ = loader->GetNumCols();
  std::vector<char*> rows = loader->GetRows();
  num_rows_ = rows.size();
  rows_ =new char[FIXED_FIELD_LEN * num_rows_ * num_cols_];
  for(int32_t row = 0; row < num_rows_; row++){
    //读取行的数据
    auto cur_row = rows.at(row);
    //将内容赋值到内存中     
    std::memcpy(rows_ + row * (FIXED_FIELD_LEN * num_cols_), cur_row, FIXED_FIELD_LEN * num_cols_);
    //获取该行第0列的值;
    int32_t key = *(int32_t *)(cur_row + FIXED_FIELD_LEN * index_column_);
    
    auto iter = index_.find(key);
    if(iter != index_.end()){
      //如果可以找到，就将行的值压入到value中去
      iter->second.push_back(row);
    }else{
      //如果找不到，就将索引和索引值新添加到索引中去
      std::vector<int> vec(1,row);
      index_.emplace(key,vec);
    }
  }
  // TODO: Implement this!
}

int32_t IndexedRowTable::GetIntField(int32_t row_id, int32_t col_id) {
  // TODO: Implement this!
  int32_t res;
  res = *(int32_t *)(rows_ + FIXED_FIELD_LEN *(row_id * num_cols_ + col_id));
  return res;
}

void IndexedRowTable::PutIntField(int32_t row_id, int32_t col_id,
                                  int32_t field) {
  // TODO: Implement this!
  //如果不是索引列直接加入内存中
  if(col_id != index_column_){
    int32_t offset = FIXED_FIELD_LEN*(row_id * num_cols_ + col_id);
    *(int32_t *)(rows_ + offset) = field;
  }else{
    //是索引列，需要更新索引表
    //首先存储原来的值，并且存储新的field，更新索引的值。
    int32_t offset = FIXED_FIELD_LEN*(row_id * num_cols_ + col_id);
    int32_t key = *(int32_t *)(rows_ + offset);
    *(int32_t *)(rows_ + offset) = field;
    std::vector<int32_t> *temp = &index_.find(key)->second;
    //如果只有一个就移除该索引
    if(temp->size() == 1){  
      index_.erase(key);
    }else{
      //否则，在索引的vector中移除该行
      for(std::vector<int32_t>::iterator iter = temp->begin(); iter < temp->end(); iter++){
        if(*iter == row_id) {
          temp->erase(iter, iter+1);
        }
      }
    }
    //添加新的索引值。
    auto it = index_.find(field);
    //如果原来的index_中没有该值，在索引中添加该值
    if(it == index_.end()){
      std::vector<int32_t> aa(1,row_id);
      index_.emplace(field,aa);
    }else{
      //如果原来的索引中已经有该值,将row_id压入到容器中。
      it->second.push_back(row_id);
    }
  }
}

int64_t IndexedRowTable::ColumnSum() {
  int64_t res=0;
  for(auto iter = index_.begin(); iter != index_.end(); iter++){
    res += iter->first * iter->second.size();
  }
  return res;
}

int64_t IndexedRowTable::PredicatedColumnSum(int32_t threshold1,
                                             int32_t threshold2) {
  // TODO: Implement this!
  int64_t res=0;
  for(int32_t row=0; row<num_rows_; row++ ){
    //判断col1是否大于threshold1,并且col2是否小于threshold2
    if(GetIntField(row,1) > threshold1 && GetIntField(row,2) < threshold2){
      res += *(int32_t*)(rows_ + FIXED_FIELD_LEN*(row*num_cols_)); 
    }
  }
  return res;  
}

int64_t IndexedRowTable::PredicatedAllColumnsSum(int32_t threshold) {
  // TODO: Implement this!
  int64_t res=0;
  std::vector<int32_t> rows;
  for(auto it = index_.begin(); it != index_.end(); it++){
    if(it->first > threshold){
      for(long unsigned int i=0; i < it->second.size(); i++){
        for(int col_id = 0; col_id < num_cols_; col_id++){
          res += GetIntField(it->second.at(i),col_id);
        }
      }
    }
  }
  return res;
}

int64_t IndexedRowTable::PredicatedUpdate(int32_t threshold) {
  // TODO: Implement this!
int64_t res=0;
  for(auto it = index_.begin(); it != index_.end(); it++){
    if(it->first < threshold){
      int32_t board = it->second.size();
      for(std::int32_t i = 0; i < board; i++){
       PutIntField(it->second.at(i),3,GetIntField(it->second.at(i),2)+GetIntField(it->second.at(i),3));
      }
      res+=board;
    }
  }
return res;
}
} // namespace bytedance_db_project