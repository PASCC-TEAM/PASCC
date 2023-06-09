#include <vector>

#include "parser.h"
#include "type.h"

using namespace pascals;
TypeTemplate *IDVarpartsAttr::AccessCheck(TypeTemplate *base_type) {
  //   std::cout << "variable type:" << base_type << std::endl;
  int in_array = 0;
  std::vector<TypeTemplate *> vistor;
  TypeTemplate *cur_type = base_type;

  for (int i = 0; i < var_parts->size(); i++) {
    // check each dim's type
    if ((*var_parts)[i].flag == 0) {  //数组
      if (!in_array) in_array = 1;
      std::copy((*var_parts)[i].subscript->begin(),
                (*var_parts)[i].subscript->end(), std::back_inserter(vistor));
    } else {  //结构体
      // check array
      if (in_array) {
        if (cur_type->template_type() != TypeTemplate::TYPE::ARRAY) {
          return TYPE_ERROR;
        }
        auto cur_array_type =
            new ArrayType(cur_type->DynamicCast<ArrayType>()->Visit(vistor));
        PtrCollect(cur_array_type);
        if (!cur_array_type->Valid()) {
          return TYPE_ERROR;
        }
        if (cur_array_type->dims() != 0) {
          return TYPE_ERROR;
        }
        cur_type = cur_array_type->base_type();
        in_array = 0;
      }
      vistor.clear();
      // check record
      if (cur_type->template_type() != TypeTemplate::TYPE::RECORD) {
        return TYPE_ERROR;
      }
      cur_type =
          cur_type->DynamicCast<RecordType>()->Find((*var_parts)[i].name);
      if (cur_type == TYPE_ERROR) {
        return TYPE_ERROR;
      }
    }
  }
  if (in_array) {
    if (cur_type->template_type() != TypeTemplate::TYPE::ARRAY) {
      return TYPE_ERROR;
    }
    auto cur_array_type =
        new ArrayType(cur_type->DynamicCast<ArrayType>()->Visit(vistor));
    PtrCollect(cur_array_type);
    if (!cur_array_type->Valid()) {
      return TYPE_ERROR;
    }
    if (cur_array_type->dims() == 0) {
      cur_type = cur_array_type->base_type();
    } else
      cur_type = cur_array_type;
  }
  return cur_type;
}