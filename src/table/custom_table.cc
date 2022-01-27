#include "custom_table.h"
#include <cstring>

namespace bytedance_db_project {
CustomTable::CustomTable() {}

CustomTable::~CustomTable() {}

void CustomTable::Load(BaseDataLoader *loader) {
  // TODO: Implement this!
  num_cols_ = loader->GetNumCols();
  std::vector<char *> rows = loader->GetRows();
  num_rows_ = rows.size();

  //设置每个数据的存储位数为2个字节；
  rows_ = new char[2 * num_rows_ * num_cols_];

  for(auto row_id = 0; row_id < num_rows_; row_id++){
    auto cur_row = rows.at(row_id);
    for(auto col_id = 0; col_id < num_cols_; col_id++){
      std::memcpy(rows_ + 2 * (row_id * num_cols_ + col_id), cur_row + col_id*FIXED_FIELD_LEN, 2);
      //构建索引0;
      if(col_id == 0){
        int16_t key = *(int16_t *)(cur_row);
        //从索引中获取该索引的迭代器。
        auto iter = index_0.find(key);
        if(iter != index_0.end()){
          iter->second.push_back(row_id);
        }
        else{
          std::vector<int32_t> vec(1,row_id);
          index_0.emplace(key,vec);
        }
      }
      //构建索引1;
      if(col_id == 1){
        int16_t key = *(int16_t *)(cur_row + FIXED_FIELD_LEN*1);
        //从索引中获取该索引的迭代器。
        auto iter = index_1.find(key);
        if(iter != index_1.end()){
          iter->second.push_back(row_id);
        }else{
          std::vector<int32_t> vec(1,row_id);
          index_1.emplace(key,vec);
        }
      }
      //构建索引2;
      if(col_id == 2){
        int16_t key = *(int16_t *)(cur_row + FIXED_FIELD_LEN*2);
        //从索引中获取该索引的迭代器。
        auto iter = index_2.find(key);
        if(iter != index_2.end()){
          iter->second.push_back(row_id);
        }else{
          std::vector<int32_t> vec(1,row_id);
          index_2.emplace(key,vec);
        }
      }
    }
  }
}

int32_t CustomTable::GetIntField(int32_t row_id, int32_t col_id) {
  // TODO: Implement this!
  return *(int16_t *)(rows_ + 2*(row_id * num_cols_ + col_id));
}

void CustomTable::PutIntField(int32_t row_id, int32_t col_id, int32_t field) {
  // TODO: Implement this!
  //如果不是索引列直接加入内存中
  if(col_id != 0 || col_id != 1 || col_id != 2){
    int32_t offset = 2*(row_id * num_cols_ + col_id);
    *(int16_t *)(rows_ + offset) = field;
  }else{
    //是索引列，需要更新索引表
    //首先存储原来的值，并且存储新的field，更新索引的值。
    int32_t offset = 2*(row_id * num_cols_ + col_id);
    int16_t key = *(int16_t *)(rows_ + offset);
    *(int16_t *)(rows_ + offset) = field;
    //列0
    if(col_id == 0){
      std::vector<int32_t> *temp = &index_0.find(key)->second;
      //如果只有一个就移除该索引
      if(temp->size() == 1){  
        index_0.erase(key);
      }else{
        //否则，在索引的vector向量中移除该行
        for(std::vector<int32_t>::iterator iter = temp->begin(); iter < temp->end(); iter++){
          if(*iter == row_id) {
            temp->erase(iter, iter+1);
          }
        }
      }
      //添加新的索引值。
      auto it = index_0.find(field);
      //如果原来的index_中没有该值，在索引中添加该值
      if(it == index_0.end()){
        std::vector<int32_t> aa(1,row_id);
        index_0.emplace(field,aa);
      }else{
        //如果原来的索引中已经有该值,将row_id压入到容器中。
        it->second.push_back(row_id);
      }
    }
    //列1
    if(col_id == 1){
      std::vector<int32_t> *temp = &index_1.find(key)->second;
      //如果只有一个就移除该索引
      if(temp->size() == 1){  
        index_1.erase(key);
      }else{
        //否则，在索引的vector向量中移除该行
        for(std::vector<int32_t>::iterator iter = temp->begin(); iter < temp->end(); iter++){
          if(*iter == row_id) {
            temp->erase(iter, iter+1);
          }
        }
      }
      //添加新的索引值。
      auto it = index_1.find(field);
      //如果原来的index_中没有该值，在索引中添加该值
      if(it == index_1.end()){
        std::vector<int32_t> aa(1,row_id);
        index_1.emplace(field,aa);
      }else{
        //如果原来的索引中已经有该值,将row_id压入到容器中。
        it->second.push_back(row_id);
      }
    }
    //列2
    if(col_id == 2){
      std::vector<int32_t> *temp = &index_2.find(key)->second;
      //如果只有一个就移除该索引
      if(temp->size() == 1){  
        index_2.erase(key);
      }else{
        //否则，在索引的vector向量中移除该行
        for(std::vector<int32_t>::iterator iter = temp->begin(); iter < temp->end(); iter++){
          if(*iter == row_id) {
            temp->erase(iter, iter+1);
          }
        }
      }
      //添加新的索引值。
      auto it = index_2.find(field);
      //如果原来的index_中没有该值，在索引中添加该值
      if(it == index_2.end()){
        std::vector<int32_t> aa(1,row_id);
        index_2.emplace(field,aa);
      }else{
        //如果原来的索引中已经有该值,将row_id压入到容器中。
        it->second.push_back(row_id);
      }
    }
  }
}

int64_t CustomTable::ColumnSum() {
  // TODO: Implement this!
  int64_t res=0;
  for(auto iter = index_0.begin(); iter != index_0.end(); iter++){
    res += iter->first * iter->second.size();
  }
  return res;
}

int64_t CustomTable::PredicatedColumnSum(int32_t threshold1,
                                         int32_t threshold2) {  
  std::unordered_map<int32_t,int8_t> hash_map; //存储列1满足条件的行
  std::vector<int32_t> rows; //存储列1和列2满足条件的行
  //分别将col1中满足条件的行以（row,1）的形式放入哈希表中
  for(auto iter = index_1.begin(); iter != index_1.end(); iter++){
    if(iter->first > threshold1){
      for(long unsigned int i = 0; i < iter->second.size(); i++ ){
        hash_map.emplace(std::pair<int32_t,int8_t>(iter->second.at(i),1));
      }
    }
  }
  //遍历列2的索引，如果行满足条件，并且也在列1满足条件的索引中，就压入rows中
  for(auto iter = index_2.begin(); iter != index_2.end(); iter++){
    if(iter->first < threshold2){
      for(long unsigned int i = 0; i < iter->second.size(); i++ ){
        if(hash_map.find(iter->second.at(i)) != hash_map.end()){
          rows.push_back(iter->second.at(i));
        }
      }
    }
  }
  
  int64_t res = 0;
  for(auto i=rows.begin(); i<rows.end(); i++){
    res += GetIntField(*i,0);
  }
  return res;
}

int64_t CustomTable::PredicatedAllColumnsSum(int32_t threshold) {
  // TODO: Implement this!
  int64_t res = 0;
  for(auto iter = index_0.begin(); iter != index_0.end(); iter++){
    if(iter->first > threshold){
      for(auto vec = iter->second.begin(); vec < iter->second.end(); vec++){
        for(int16_t col = 0; col < num_cols_; col++){
          res+=GetIntField(*vec,col);
        }
      }
    }
  }
  return res;
}

int64_t CustomTable::PredicatedUpdate(int32_t threshold) {
  // TODO: Implement this!
  int64_t res = 0;
  for(auto iter = index_0.begin(); iter != index_0.end(); iter++){
    if(iter->first < threshold){
      for(auto vec = iter->second.begin(); vec < iter->second.end(); vec++){
        PutIntField(*vec,3,GetIntField(*vec,2)+GetIntField(*vec,3));
      }
      res += iter->second.size();
    }
  }
  return res;
}
} // namespace bytedance_db_project