#pragma once

/*!
 * @file Object.h
 * An "Object" represents a scheme object.
 * There are different types of objects, as represented by ObjectType.
 * An "Object" is an efficient wrapper around any of these types.
 * Some types are "heap allocated", and have reference semantics, and others are
 * "fixed" and have value semantics.  Heap allocated objects implement reference counting with
 * std::shared_ptr.
 *
 * To create a new Object for a heap allocated type, use the make_new static method of the type of
 * object you want to make. This will return a correctly setup Object. For fixed objects, use
 * Object::make_<type>
 *
 * To convert an Object into a more specific object, use the as_<type> method of Object.
 * It will throw an exception if you get the type wrong.
 *
 * These are all the types:
 *
 * EMPTY_LIST - a special heap allocated object. There is only one EMPTY_LIST allocated, and
 * EmptyListObject::make_new() will always return an Object which references that one.
 *
 * INTEGER - a fixed type. Use Object::make_integer() to create one. Internally uses int64_t
 * FLOAT - a fixed type. Use Object::make_float() to create one. Internally uses double
 * CHAR - a fixed type. Use Object::make_char() to create one. Internally uses char
 *
 * SYMBOL - a special heap allocated object. SymbolObject::make_new requires a SymbolTable to
 * store the newly allocated symbol in, and will return an existing symbol if there already is one.
 *
 * STRING - a heap allocated object. Create with StringObject::make_new. Uses std::string internally
 *
 * PAIR - a heap allocated object containing two Objects.
 *
 * ARRAY - a heap allocated object containing a std::vector<Object>
 *
 * LAMBDA - a heap allocated object representing a GOOS lambda
 * MACRO - a heap allocated object representing a GOOS macro
 * ENVIRONMENT - a heap allocated object representing a GOOS environment
 *
 */

#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <stdexcept>
#include <map>
#include "common/common_types.h"
#include "common/util/Assert.h"

namespace goos {

using FloatType = double;
using IntType = s64;

/*!
 * All objects have one of these kinds
 */
enum class ObjectType : u8 {
  // special
  EMPTY_LIST,

  // fixed
  INTEGER,
  FLOAT,
  CHAR,

  // allocated
  SYMBOL,
  STRING,
  PAIR,
  ARRAY,
  LAMBDA,
  MACRO,
  ENVIRONMENT,
  INVALID
};

std::string object_type_to_string(ObjectType kind);

// Some objects are "fixed", meaning they are stored inline in the Object, and not heap allocated.

/*!
 * By default, convert a fixed object data to string with std::to_string.
 * This will be used for integers, but float and char define their own.
 */
template <typename T>
std::string fixed_to_string(T x) {
  return std::to_string(x);
}

/*!
 * Special case to print floating point
 */
template <>
std::string fixed_to_string<FloatType>(FloatType);

/*!
 * Special case to print character
 */
template <>
std::string fixed_to_string<char>(char);

/*!
 * Special case to print integer
 */
template <>
std::string fixed_to_string<IntType>(IntType);

/*!
 * Common implementation for a fixed object
 */
template <typename T>
class FixedObject {
 public:
  T value;

  explicit FixedObject(T v) : value(v) {}
  FixedObject() = default;
  std::string print() const { return fixed_to_string(value); }
  std::string inspect() const { return type_as_string() + " " + fixed_to_string(value) + "\n"; }
  ~FixedObject() = default;

  bool operator==(const FixedObject<T>& other) const { return value == other.value; }

 private:
  std::string type_as_string() const {
    if (std::is_same<T, FloatType>())
      return object_type_to_string(ObjectType::FLOAT);
    if (std::is_same<T, IntType>())
      return object_type_to_string(ObjectType::INTEGER);
    if (std::is_same<T, char>())
      return object_type_to_string(ObjectType::CHAR);
    throw std::runtime_error("Unsupported FixedObject type");
  }
};

/*!
 * The fixed objects supported by GOOS
 */
using IntegerObject = FixedObject<int64_t>;
using FloatObject = FixedObject<double>;
using CharObject = FixedObject<char>;

// Other objects are separate allocated on the heap. These objects should be HeapObjects.

class HeapObject {
 public:
  virtual std::string print() const = 0;
  virtual std::string inspect() const = 0;
  virtual ~HeapObject() = default;
};

// forward declare all HeapObjects
class PairObject;
class EnvironmentObject;
class SymbolObject;
class StringObject;
class LambdaObject;
class MacroObject;
class ArrayObject;

// Wrapper Object class for all objects
class Object {
 public:
  std::shared_ptr<HeapObject> heap_obj = nullptr;
  friend Object build_list(const std::vector<Object>& objects);
  friend Object build_list(std::vector<Object>&& objects);

  union {
    IntegerObject integer_obj;
    FloatObject float_obj;
    CharObject char_obj;
  };

  ObjectType type = ObjectType::INVALID;

  std::string print() const {
    switch (type) {
      case ObjectType::INTEGER:
        return integer_obj.print();
      case ObjectType::FLOAT:
        return float_obj.print();
      case ObjectType::CHAR:
        return char_obj.print();
      case ObjectType::EMPTY_LIST:
        return "()";
      default:
        return heap_obj->print();
    }
  }

  std::string inspect() const {
    switch (type) {
      case ObjectType::INTEGER:
        return integer_obj.inspect();
      case ObjectType::FLOAT:
        return float_obj.inspect();
      case ObjectType::CHAR:
        return char_obj.inspect();
      case ObjectType::EMPTY_LIST:
        return "[empty list] ()\n";
      default:
        return heap_obj->inspect();
    }
  }

  template <typename T>
  static Object make_number(T value);

  static Object make_integer(IntType value) {
    Object o;
    o.type = ObjectType::INTEGER;
    o.integer_obj.value = value;
    return o;
  }

  static Object make_float(FloatType value) {
    Object o;
    o.type = ObjectType::FLOAT;
    o.float_obj.value = value;
    return o;
  }

  static Object make_char(char value) {
    Object o;
    o.type = ObjectType::CHAR;
    o.char_obj.value = value;
    return o;
  }

  static Object make_empty_list() {
    Object o;
    o.type = ObjectType::EMPTY_LIST;
    return o;
  }

  PairObject* as_pair() const;
  EnvironmentObject* as_env() const;
  std::shared_ptr<EnvironmentObject> as_env_ptr() const;
  SymbolObject* as_symbol() const;
  StringObject* as_string() const;
  LambdaObject* as_lambda() const;
  MacroObject* as_macro() const;
  ArrayObject* as_array() const;

  IntType& as_int() {
    if (type != ObjectType::INTEGER) {
      throw std::runtime_error("as_int called on a " + object_type_to_string(type) + " " + print());
    }
    return integer_obj.value;
  }

  const IntType& as_int() const {
    if (type != ObjectType::INTEGER) {
      throw std::runtime_error("as_int called on a " + object_type_to_string(type) + " " + print());
    }
    return integer_obj.value;
  }

  FloatType& as_float() {
    if (type != ObjectType::FLOAT) {
      throw std::runtime_error("as_float called on a " + object_type_to_string(type) + " " +
                               print());
    }
    return float_obj.value;
  }

  const FloatType& as_float() const {
    if (type != ObjectType::FLOAT) {
      throw std::runtime_error("as_float called on a " + object_type_to_string(type) + " " +
                               print());
    }
    return float_obj.value;
  }

  char& as_char() {
    if (type != ObjectType::CHAR) {
      throw std::runtime_error("as_char called on a " + object_type_to_string(type) + " " +
                               print());
    }
    return char_obj.value;
  }

  bool is_empty_list() const { return type == ObjectType::EMPTY_LIST; }
  bool is_list() const { return type == ObjectType::EMPTY_LIST || type == ObjectType::PAIR; }
  bool is_int() const { return type == ObjectType::INTEGER; }
  bool is_float() const { return type == ObjectType::FLOAT; }
  bool is_char() const { return type == ObjectType::CHAR; }
  bool is_symbol() const { return type == ObjectType::SYMBOL; }
  bool is_symbol(const std::string& name) const;
  bool is_string() const { return type == ObjectType::STRING; }
  bool is_pair() const { return type == ObjectType::PAIR; }
  bool is_array() const { return type == ObjectType::ARRAY; }
  bool is_env() const { return type == ObjectType::ENVIRONMENT; }
  bool is_macro() const { return type == ObjectType::MACRO; }

  bool operator==(const Object& other) const;
  bool operator!=(const Object& other) const { return !((*this) == other); }
};

class SymbolTable;

/*!
 * A Symbol Object, which is just a wrapper around a string.
 * The make_new function will correctly
 */
class SymbolObject : public HeapObject {
 public:
  std::string name;
  explicit SymbolObject(std::string _name) : name(std::move(_name)) {}
  static Object make_new(SymbolTable& st, const std::string& name);

  std::string print() const override { return name; }

  std::string inspect() const override { return "[symbol] " + name + "\n"; }

  ~SymbolObject() = default;
};

/*!
 * A Symbol Table, which holds all symbols.
 */
class SymbolTable {
 public:
  std::shared_ptr<HeapObject> intern(const std::string& name) {
    const auto& kv = table.find(name);
    if (kv == table.end()) {
      auto iter = table.insert({name, std::make_shared<SymbolObject>(name)});
      return (*iter.first).second;
    } else {
      return kv->second;
    }
  }

  HeapObject* intern_ptr(const std::string& name) {
    const auto& kv = table.find(name);
    if (kv == table.end()) {
      auto iter = table.insert({name, std::make_shared<SymbolObject>(name)});
      return (*iter.first).second.get();
    } else {
      return kv->second.get();
    }
  }

  ~SymbolTable() = default;

 private:
  std::unordered_map<std::string, std::shared_ptr<HeapObject>> table;
};

class StringObject : public HeapObject {
 public:
  std::string data;
  explicit StringObject(std::string text) : data(std::move(text)) {}

  static Object make_new(const std::string& text) {
    Object obj;
    obj.type = ObjectType::STRING;
    obj.heap_obj = std::make_shared<StringObject>(text);
    return obj;
  }

  std::string print() const override;
  std::string inspect() const override;

  ~StringObject() override = default;
};

class PairObject : public HeapObject {
 public:
  Object car, cdr;

  PairObject(const Object& car_, const Object& cdr_) : car(car_), cdr(cdr_) {}
  PairObject() = default;

  static Object make_new(const Object& a, const Object& b) {
    Object obj;
    obj.type = ObjectType::PAIR;
    obj.heap_obj = std::make_shared<PairObject>(a, b);
    return obj;
  }

  std::string print() const override {
    std::pair<Object, Object> to_print_pair = std::make_pair(car, cdr);

    std::string result = "(";

    // print first thing:
    result += car.print();

    // print second thing
    Object to_print = cdr;
    if (to_print.type == ObjectType::EMPTY_LIST) {
      result += ")";
      return result;
    } else {
      result += " ";
    }

    for (;;) {
      if (to_print.type == ObjectType::PAIR) {
        Object to_print_car = std::dynamic_pointer_cast<PairObject>(to_print.heap_obj)->car;
        result += to_print_car.print();
        to_print = std::dynamic_pointer_cast<PairObject>(to_print.heap_obj)->cdr;
        if (to_print.type == ObjectType::EMPTY_LIST) {
          result += ")";
          return result;
        } else {
          result += " ";
        }
      } else {
        result += ". ";
        result += to_print.print();
        result += ")";
        return result;
      }
    }
  }

  std::string inspect() const override { return "[pair] " + print() + "\n"; }

  ~PairObject() = default;
};

class EnvironmentObject : public HeapObject {
 public:
  std::string name;
  std::shared_ptr<EnvironmentObject> parent_env;

  // the symbols will be stored in the symbol table and never removed, so we don't need shared
  // pointers here.
  std::unordered_map<HeapObject*, Object> vars;

  EnvironmentObject() = default;

  static Object make_new() {
    Object obj;
    obj.type = ObjectType::ENVIRONMENT;
    obj.heap_obj = std::make_shared<EnvironmentObject>();
    return obj;
  }

  static Object make_new(std::string name,
                         std::shared_ptr<EnvironmentObject> parent_env = nullptr) {
    Object obj;
    obj.type = ObjectType::ENVIRONMENT;
    auto env = std::make_shared<EnvironmentObject>();
    env->name = std::move(name);
    env->parent_env = std::move(parent_env);
    obj.heap_obj = std::move(env);
    return obj;
  }

  std::string print() const override {
    if (name.empty()) {
      return "<unnamed environment>";
    } else {
      return "<environment \"" + name + "\">";
    }
  }

  std::string inspect() const override {
    std::string result = "[environment]\n  name: " + name +
                         "\n  parent: " + (parent_env ? parent_env->print() : "NONE") +
                         "\n  vars:\n";
    for (const auto& kv : vars) {
      result += "    " + kv.first->print() + ": " + kv.second.print() + "\n";
    }
    return result;
  }
};

struct NamedArg {
  bool has_default = false;
  Object default_value;
};

struct ArgumentSpec {
  bool varargs = false;
  std::vector<std::string> unnamed;
  std::unordered_map<std::string, NamedArg> named;
  std::string rest;
  std::string print() const;
};

ArgumentSpec make_varargs();

struct Arguments {
  std::vector<Object> unnamed;
  // note that this is _not_ an unordered map so that named arguments have a consistent (but
  // arbitrary) alphabetical evaluation order.
  std::map<std::string, Object> named;
  std::vector<Object> rest;
  bool has_rest = false;
  std::string print() const;

  Object get_named(const std::string& name, const Object& default_value);
  Object get_named(const std::string& name);
  bool has_named(const std::string& name);
  bool only_contains_named(const std::unordered_set<std::string>& names);
};

class LambdaObject : public HeapObject {
 public:
  std::string name;
  std::shared_ptr<EnvironmentObject> parent_env;
  Object body;
  ArgumentSpec args;

  LambdaObject() = default;

  static Object make_new() {
    Object obj;
    obj.type = ObjectType::LAMBDA;
    obj.heap_obj = std::make_shared<LambdaObject>();
    return obj;
  }

  std::string print() const override {
    if (name.empty()) {
      return "<unnamed lambda>";
    } else {
      return "<lambda \"" + name + "\">";
    }
  }

  std::string inspect() const override { return "[lambda]\n  name: " + name + "\n" + args.print(); }
};

class MacroObject : public HeapObject {
 public:
  std::string name;
  std::shared_ptr<EnvironmentObject> parent_env;
  Object body;
  ArgumentSpec args;

  MacroObject() = default;

  static Object make_new() {
    Object obj;
    obj.type = ObjectType::MACRO;
    obj.heap_obj = std::make_shared<MacroObject>();
    return obj;
  }

  std::string print() const override {
    if (name.empty()) {
      return "<unnamed macro>";
    } else {
      return "<macro \"" + name + "\">";
    }
  }

  std::string inspect() const override { return "[macro]\n  name: " + name + "\n" + args.print(); }
};

class ArrayObject : public HeapObject {
 public:
  std::vector<Object> data;
  ArrayObject(std::vector<Object> objects) : data(std::move(objects)) {}
  static Object make_new(std::vector<Object> objects) {
    Object obj;
    obj.type = ObjectType::ARRAY;
    obj.heap_obj = std::make_shared<ArrayObject>(std::move(objects));
    return obj;
  }

  std::string print() const override {
    std::string result = "#(";
    if (data.empty()) {
      return result + ")";
    }
    for (const auto& obj : data) {
      result += obj.print() + " ";
    }
    result.pop_back();  // remove last space
    return result + ")";
  }

  std::string inspect() const override {
    return "[array] size: " + std::to_string(data.size()) + " data: " + print() + "\n";
  }

  std::size_t size() const { return data.size(); }

  const Object& operator[](size_t idx) const { return data.at(idx); }

  Object& operator[](size_t idx) { return data.at(idx); }
};

Object build_list(const std::vector<Object>& objects);
Object build_list(std::vector<Object>&& objects);

inline PairObject* Object::as_pair() const {
  if (type != ObjectType::PAIR) {
    throw std::runtime_error("as_pair called on a " + object_type_to_string(type) + " " + print());
  }
  return static_cast<PairObject*>(heap_obj.get());
}

inline EnvironmentObject* Object::as_env() const {
  if (type != ObjectType::ENVIRONMENT) {
    throw std::runtime_error("as_env called on a " + object_type_to_string(type) + " " + print());
  }
  return static_cast<EnvironmentObject*>(heap_obj.get());
}

inline std::shared_ptr<EnvironmentObject> Object::as_env_ptr() const {
  if (type != ObjectType::ENVIRONMENT) {
    throw std::runtime_error("as_env called on a " + object_type_to_string(type) + " " + print());
  }
  return std::dynamic_pointer_cast<EnvironmentObject>(heap_obj);
}

inline SymbolObject* Object::as_symbol() const {
  if (type != ObjectType::SYMBOL) {
    throw std::runtime_error("as_symbol called on a " + object_type_to_string(type) + " " +
                             print());
  }
  return static_cast<SymbolObject*>(heap_obj.get());
}

inline StringObject* Object::as_string() const {
  if (type != ObjectType::STRING) {
    throw std::runtime_error("as_string called on a " + object_type_to_string(type) + " " +
                             print());
  }
  return static_cast<StringObject*>(heap_obj.get());
}

inline LambdaObject* Object::as_lambda() const {
  if (type != ObjectType::LAMBDA) {
    throw std::runtime_error("as_lambda called on a " + object_type_to_string(type) + " " +
                             print());
  }
  return static_cast<LambdaObject*>(heap_obj.get());
}

inline MacroObject* Object::as_macro() const {
  if (type != ObjectType::MACRO) {
    throw std::runtime_error("as_macro called on a " + object_type_to_string(type) + " " + print());
  }
  return static_cast<MacroObject*>(heap_obj.get());
}

inline ArrayObject* Object::as_array() const {
  if (type != ObjectType::ARRAY) {
    throw std::runtime_error("as_array called on a " + object_type_to_string(type) + " " + print());
  }
  return static_cast<ArrayObject*>(heap_obj.get());
}
}  // namespace goos
