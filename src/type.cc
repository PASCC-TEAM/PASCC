#include "type.h"

#include "log.h"

using std::string;
using std::vector;

namespace pascals {
void ConstValue::set_unimus() {
  if (m_Type == TYPE_INT)
    m_INT = -m_INT;
  else if (m_Type == TYPE_REAL)
    m_REAL = -m_REAL;
  else if (m_Type == TYPE_CHAR)
    m_CHAR = -m_CHAR;
  else
    throw std::runtime_error("ConstValue : set_unimus : type not supported");
}
ConstValue::ConstValue(const ConstValue& other) {
  m_Type = other.m_Type;
  m_INT = other.m_INT;
  m_REAL = other.m_REAL;
  m_CHAR = other.m_CHAR;
  m_BOOLEAN = other.m_BOOLEAN;
  m_STRING = other.m_STRING;
}

ConstValue& ConstValue::operator=(const ConstValue& other) {
  m_Type = other.m_Type;
  m_INT = other.m_INT;
  m_REAL = other.m_REAL;
  m_CHAR = other.m_CHAR;
  m_BOOLEAN = other.m_BOOLEAN;
  m_STRING = other.m_STRING;
  return *this;
}

// operation +
ConstValue ConstValue::operator+(const ConstValue& other) {
  if (m_Type != other.m_Type)
    throw std::runtime_error("ConstValue : operator+ : type not match");
  ConstValue ret;
  if (m_Type == TYPE_INT) {
    ret.set((int)(m_INT + other.m_INT));
    return ret;
  } else if (m_Type == TYPE_REAL) {
    ret.set((float)(m_REAL + other.m_REAL));
    return ret;
  } else if (m_Type == TYPE_STRINGLIKE) {
    ret.set(m_STRING + other.m_STRING);
    return ret;
  } else if (m_Type == TYPE_CHAR) {
    ret.set((char)((int)m_CHAR + (int)other.m_CHAR));
    return ret;
  } else {
    throw std::runtime_error("ConstValue : operator+ : type not supported");
  }
}

// operation -
ConstValue ConstValue::operator-(const ConstValue& other) {
  if (m_Type != other.m_Type)
    throw std::runtime_error("ConstValue : operator- : type not match");
  ConstValue ret;
  if (m_Type == TYPE_INT) {
    ret.set((int)(m_INT - other.m_INT));
    return ret;
  } else if (m_Type == TYPE_REAL) {
    ret.set((float)(m_REAL - other.m_REAL));
    return ret;
  } else if (m_Type == TYPE_CHAR) {
    ret.set((char)((int)m_CHAR - (int)other.m_CHAR));
    return ret;
  } else {
    throw std::runtime_error("ConstValue : operator- : type not supported");
  }
}

// operation *
ConstValue ConstValue::operator*(const ConstValue& other) {
  if (m_Type != other.m_Type)
    throw std::runtime_error("ConstValue : operator* : type not match");
  ConstValue ret;
  if (m_Type == TYPE_INT) {
    ret.set((int)(m_INT * other.m_INT));
    return ret;
  } else if (m_Type == TYPE_REAL) {
    ret.set((float)(m_REAL * other.m_REAL));
    return ret;
  } else {
    throw std::runtime_error("ConstValue : operator* : type not supported");
  }
}

// operation /
ConstValue ConstValue::operator/(const ConstValue& other) {
  if (m_Type != other.m_Type)
    throw std::runtime_error("ConstValue : operator* : type not match");
  ConstValue ret;
  if (m_Type == TYPE_INT) {
    ret.set((int)(m_INT / other.m_INT));
    return ret;
  } else if (m_Type == TYPE_REAL) {
    ret.set((float)(m_REAL / other.m_REAL));
    return ret;
  } else {
    throw std::runtime_error("ConstValue : operator* : type not supported");
  }
}

std::string BasicType::name() {
  switch (basic_type_) {
    case BASIC_TYPE::INT:
      return "integer";
    case BASIC_TYPE::REAL:
      return "real";
    case BASIC_TYPE::BOOL:
      return "boolean";
    case BASIC_TYPE::CHAR:
      return "char";
    default:
      return "none";
  }
}

std::string ArrayType::ArrayBound::name() {
  if(type_ != TYPE_CHAR && type_ != TYPE_INT) {
    return "[error]";
  }
  if(type_ == TYPE_CHAR){
    return "[" + string(1, (char)lb_) + ".." + string(1, (char)ub_) + "]";
  }
  return "[" + std::to_string(lb_) + ".." + std::to_string(ub_) + "]";
}

std::string ArrayType::name() {
  std::string ret = "array";
  for (auto& b : bounds_) {
    ret += b.name();
  }
  string base_name = base_type_ == TYPE_ERROR ? "error" : base_type_->name();
  return ret + " of " + base_name;
}

ArrayType::ArrayBound& ArrayType::ArrayBound::operator=(const ArrayBound& b2) {
  type_ = b2.type_;
  lb_ = b2.lb_;
  ub_ = b2.ub_;
  return *this;
}

ArrayType::ArrayType(const ArrayType& other) {
  base_type_ = other.base_type_;
  for (auto& b : other.bounds_) {
    bounds_.emplace_back(b);
  }
}

ArrayType& ArrayType::operator=(const ArrayType& other) {
  base_type_ = other.base_type_;
  for (auto& b : other.bounds_) {
    bounds_.emplace_back(b);
  }
  return *this;
}

ArrayType ArrayType::Visit(std::vector<TypeTemplate*> v_types) {
  if (v_types.size() == 0) return *this;
  if (v_types.size() > dims()) return ArrayType(TYPE_ERROR);
  for (int i = 0; i < v_types.size(); i++) {
    if (bounds_[i].type_ != v_types[i]) return ArrayType(TYPE_ERROR);
  }
  return Visit(v_types.size());
}

ArrayType ArrayType::Visit(unsigned int v_layer) {
  if (v_layer == 0) return *this;
  if (v_layer > dims()) return ArrayType(TYPE_ERROR);
  // temp array
  vector<ArrayBound> bs;
  for (int i = v_layer; i < bounds_.size(); i++) {
    bs.emplace_back(bounds_[i]);
  }
  return ArrayType(base_type_, bs);
}

bool ArrayType::operator==(const ArrayType& a2) const {
  if (base_type_ != a2.base_type_) return false;
  int dims = bounds_.size();
  if (dims != a2.bounds_.size()) return false;
  for (int i = 0; i < dims; i++) {
    if (!(bounds_[i] == a2.bounds_[i])) return false;
  }
  return true;
}

std::string RecordType::name() {
  std::string ret = "record ";
  for (auto& e : types_map_) {
    string type_name = e.second == TYPE_ERROR ? "error" : e.second->name();
    ret += " " + e.first + ":" + type_name + ";";
  }
  return ret + " end";
}

TypeTemplate* RecordType::Visit(std::vector<std::string> names) {
  TypeTemplate* vtype = this;
  int loop = 0, len = names.size();
  while (loop < len) {
    // cast assert
    if (vtype->template_type() != TYPE::RECORD) return nullptr;
    // find element
    auto t = vtype->DynamicCast<RecordType>()->Find(names[loop]);
    if (t == nullptr) return nullptr;
    // next
    vtype = t;
    loop++;
  }
  return vtype;
}

bool TypeTemplate::StringLike() {
  if (template_type_ == TYPE::ARRAY) {
    auto array = DynamicCast<ArrayType>();
    return array->StringLike();
  }
  return false;
}

bool ArrayType::StringLike(int access_layer) {
  if (base_type_ != TYPE_CHAR) return false;
  if (bounds_.size() != access_layer + 1) return false;
  return true;
}

bool RecordType::add(std::string name, TypeTemplate* type) {
  if (types_map_.find(name) != types_map_.end()) return false;
  types_map_[name] = type;
  return true;
}

TypeTemplate* RecordType::Find(std::string name) {
  auto ptr = types_map_.find(name);
  if (ptr != types_map_.end())
    return (*ptr).second;
  else
    return nullptr;
}

/* **************** global initialize **************** */
BasicType* TYPE_INT;
BasicType* TYPE_REAL;
BasicType* TYPE_BOOL;
BasicType* TYPE_CHAR;
BasicType* TYPE_NONE;
BasicType* TYPE_ERROR;
BasicType* TYPE_STRINGLIKE;

std::vector<TypeTemplate*>* PTR_COLLECTOR = nullptr;

OperationMap operation_map;

void TypeInit() {
  log_set_level(LOG_INFO);
  TYPE_ERROR = nullptr;

  TYPE_BOOL = new BasicType(BasicType::BASIC_TYPE::BOOL);
  TYPE_CHAR = new BasicType(BasicType::BASIC_TYPE::CHAR);
  TYPE_INT = new BasicType(BasicType::BASIC_TYPE::INT);
  TYPE_REAL = new BasicType(BasicType::BASIC_TYPE::REAL);
  TYPE_NONE = new BasicType(BasicType::BASIC_TYPE::NONE);
  TYPE_STRINGLIKE = new BasicType(BasicType::BASIC_TYPE::CHAR);

  PTR_COLLECTOR = new std::vector<TypeTemplate*>();

  // bool
  operation_map[Operation(TYPE_BOOL, TYPE_BOOL, "and")] = TYPE_BOOL;
  operation_map[Operation(TYPE_BOOL, TYPE_BOOL, "or")] = TYPE_BOOL;
  operation_map[Operation(TYPE_BOOL, NULL, "not")] = TYPE_BOOL;
  operation_map[Operation(TYPE_BOOL, TYPE_BOOL, "=")] = TYPE_BOOL;
  operation_map[Operation(TYPE_BOOL, TYPE_BOOL, "<>")] = TYPE_BOOL;
  operation_map[Operation(TYPE_BOOL, TYPE_BOOL, "<")] = TYPE_BOOL;
  operation_map[Operation(TYPE_BOOL, TYPE_BOOL, ">")] = TYPE_BOOL;
  operation_map[Operation(TYPE_BOOL, TYPE_BOOL, "<=")] = TYPE_BOOL;
  operation_map[Operation(TYPE_BOOL, TYPE_BOOL, ">=")] = TYPE_BOOL;
  // int
  operation_map[Operation(TYPE_INT, NULL, "-")] = TYPE_INT;
  operation_map[Operation(TYPE_INT, NULL, "+")] = TYPE_INT;
  operation_map[Operation(TYPE_INT, TYPE_INT, "+")] = TYPE_INT;
  operation_map[Operation(TYPE_INT, TYPE_INT, "-")] = TYPE_INT;
  operation_map[Operation(TYPE_INT, TYPE_INT, "*")] = TYPE_INT;
  operation_map[Operation(TYPE_INT, TYPE_INT, "/")] = TYPE_REAL;
  operation_map[Operation(TYPE_INT, TYPE_INT, "mod")] = TYPE_INT;
  operation_map[Operation(TYPE_INT, TYPE_INT, "div")] = TYPE_INT;
  operation_map[Operation(TYPE_INT, TYPE_INT, "=")] = TYPE_BOOL;
  operation_map[Operation(TYPE_INT, TYPE_INT, "<>")] = TYPE_BOOL;
  operation_map[Operation(TYPE_INT, TYPE_INT, "<")] = TYPE_BOOL;
  operation_map[Operation(TYPE_INT, TYPE_INT, ">")] = TYPE_BOOL;
  operation_map[Operation(TYPE_INT, TYPE_INT, "<=")] = TYPE_BOOL;
  operation_map[Operation(TYPE_INT, TYPE_INT, ">=")] = TYPE_BOOL;
  // real
  operation_map[Operation(TYPE_REAL, NULL, "-")] = TYPE_REAL;
  operation_map[Operation(TYPE_REAL, NULL, "+")] = TYPE_REAL;
  operation_map[Operation(TYPE_REAL, TYPE_REAL, "+")] = TYPE_REAL;
  operation_map[Operation(TYPE_REAL, TYPE_REAL, "-")] = TYPE_REAL;
  operation_map[Operation(TYPE_REAL, TYPE_REAL, "*")] = TYPE_REAL;
  operation_map[Operation(TYPE_REAL, TYPE_REAL, "/")] = TYPE_REAL;
  operation_map[Operation(TYPE_REAL, TYPE_REAL, "=")] = TYPE_BOOL;
  operation_map[Operation(TYPE_REAL, TYPE_REAL, "<>")] = TYPE_BOOL;
  operation_map[Operation(TYPE_REAL, TYPE_REAL, ">")] = TYPE_BOOL;
  operation_map[Operation(TYPE_REAL, TYPE_REAL, "<")] = TYPE_BOOL;
  operation_map[Operation(TYPE_REAL, TYPE_REAL, ">=")] = TYPE_BOOL;
  operation_map[Operation(TYPE_REAL, TYPE_REAL, "<=")] = TYPE_BOOL;
  // char
  operation_map[Operation(TYPE_CHAR, TYPE_CHAR, "=")] = TYPE_BOOL;
  operation_map[Operation(TYPE_CHAR, TYPE_CHAR, "<>")] = TYPE_BOOL;
  operation_map[Operation(TYPE_CHAR, TYPE_CHAR, "<")] = TYPE_BOOL;
  operation_map[Operation(TYPE_CHAR, TYPE_CHAR, "<=")] = TYPE_BOOL;
  operation_map[Operation(TYPE_CHAR, TYPE_CHAR, ">")] = TYPE_BOOL;
  operation_map[Operation(TYPE_CHAR, TYPE_CHAR, ">=")] = TYPE_BOOL;
  // int real
  operation_map[Operation(TYPE_REAL, TYPE_INT, "+")] = TYPE_REAL;
  operation_map[Operation(TYPE_REAL, TYPE_INT, "-")] = TYPE_REAL;
  operation_map[Operation(TYPE_REAL, TYPE_INT, "*")] = TYPE_REAL;
  operation_map[Operation(TYPE_REAL, TYPE_INT, "/")] = TYPE_REAL;
  operation_map[Operation(TYPE_REAL, TYPE_INT, "=")] = TYPE_BOOL;
  operation_map[Operation(TYPE_REAL, TYPE_INT, "<>")] = TYPE_BOOL;
  operation_map[Operation(TYPE_REAL, TYPE_INT, ">")] = TYPE_BOOL;
  operation_map[Operation(TYPE_REAL, TYPE_INT, ">=")] = TYPE_BOOL;
  operation_map[Operation(TYPE_REAL, TYPE_INT, "<=")] = TYPE_BOOL;
  operation_map[Operation(TYPE_REAL, TYPE_INT, "<")] = TYPE_BOOL;
  operation_map[Operation(TYPE_INT, TYPE_REAL, "+")] = TYPE_REAL;
  operation_map[Operation(TYPE_INT, TYPE_REAL, "-")] = TYPE_REAL;
  operation_map[Operation(TYPE_INT, TYPE_REAL, "*")] = TYPE_REAL;
  operation_map[Operation(TYPE_INT, TYPE_REAL, "/")] = TYPE_REAL;
  operation_map[Operation(TYPE_INT, TYPE_REAL, "=")] = TYPE_BOOL;
  operation_map[Operation(TYPE_INT, TYPE_REAL, "<>")] = TYPE_BOOL;
  operation_map[Operation(TYPE_INT, TYPE_REAL, ">")] = TYPE_BOOL;
  operation_map[Operation(TYPE_INT, TYPE_REAL, ">=")] = TYPE_BOOL;
  operation_map[Operation(TYPE_INT, TYPE_REAL, "<=")] = TYPE_BOOL;
  operation_map[Operation(TYPE_INT, TYPE_REAL, "<")] = TYPE_BOOL;
}
int _ = (TypeInit(), 0);

void TypeRelease() {
  ReleaseTemp();
  delete TYPE_CHAR;
  delete TYPE_INT;
  delete TYPE_REAL;
  delete TYPE_NONE;
  delete TYPE_STRINGLIKE;
  delete PTR_COLLECTOR;
  log_debug("delete global basic types");
}

int __ = (atexit(TypeRelease), 0);
}  // namespace pascals