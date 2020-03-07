#include "types.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::lisp {
namespace d=czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LValue::LValue(d::ExprValue m) : metaObj(m) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LValue::LValue() {}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LValue::withMeta(d::ExprValue m) const {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LValue::meta() const {
  return metaObj.isNull() ? nil() : metaObj;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LValue::equals(const d::SValue* rhs) const {
  return compare(rhs) == 0;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LInt::LInt(const LInt& rhs, d::ExprValue m) : LValue(m) {
  value= rhs.value;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LInt::LInt(llong n) : value(n) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string LInt::toString() const {
  return std::to_string(value);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
llong LInt::impl() const { return value; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LInt::compare(const d::SValue* rhs) const {
  return static_cast<const LInt*>(rhs)->value == value;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue withMeta(d::ExprValue m) const {
  return new LInt(*this, m);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LStr(const std::string& s) : value (s) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LStr(const LStr& rhs, d::ExprValue m) : LValue(m) {
  value = rhs.impl();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string LStrk::impl() const { return value; }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LString(const LString& rhs, d::ExprValue m) : LStrk(rhs,m) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LString::LString(const std::string& s) : LStrk(s) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string LString::toString() const {
  //return readably ? escapedValue() : value();
  return impl();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string LString::encoded() const {
  return encode(impl());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LString::compare(const d::SValue* rhs) const {
  return ::strcmp(static_cast<const LString*>(rhs)->impl().c_str, impl().c_str);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LString::withMeta(d::ExprValue m) const {
  return new LString(*this, m);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LKeyword::LKeyword(const std::string& s) : LStrk(s) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LKeyword::LKeyword(const LKeyword& rhs, d::ExprValue m) : LStrk(rhs,m) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LKeyword::compare(const d::SValue* rhs) const {
  return ::strcmp(static_cast<const LKeyword*>(rhs)->impl().c_str(), impl().c_str);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string LKeyword::toString() const {
  return impl();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LKeyword::withMeta(d::ExprValue m) const {
  return new LKeyword(*this, m);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSymbol(const std::string& s) : value(s) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSymbol::LSymbol(const LSymbol& rhs, d::ExprValue m) : LStrk(rhs, m) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LSymbol::eval(d::IEvaluator* e) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LSymbol::compare(const d::SValue* rhs) const {
  return ::strcmp(static_cast<const LSymbol*>(rhs)->impl().c_str(), impl().c_str());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LSymbol::withMeta(d::ExprValue m) {
  return new LSymbol(*this, m);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSeq(ValueVec* v) : values(v) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSeq::LSeq(VVecIter begin, VVecIter end) {
  values= new ValueVec(begin,end);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSeq::LSeq(const LSeq& rhs, d::ExprValue m) : LValue(m) {
  values = new ValueVec(*(rhs.values));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSeq::~LSeq() {
  del_ptr(values);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string LSeq::toString() const {
  //deal with constness
  std::string out;
  for (auto& x = values->cbegin(); x != values->cend(); ++x) {
    if (!out.empty()) {
      out += " ";
    }
    out += (*x)->print(readably);
  }
  return out;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ValueVec* LSeq::evalItems(d::IEvaluator* e) const {
  auto out = new ValueVec();
  //v->reserve(count());
  for (auto& i : values) {
    out->push_back(EVAL(i, e));
  }
  return out;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LSeq::isEmpty() const { return values->empty(); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LSeq::count() const { return values->size(); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LSeq::item(int pos) const {
  return values->operator[](pos);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
VVecIter LSeq::begin() const { return values->begin(); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
VVecIter LSeq::end() const { return values->end(); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LSeq::compare(const d::SValue* v) const {
  const LSeq* rhs= static_cast<const LSeq*>(v);
  int sz = count();

  if (sz != rhs->count()) { return false; }

  for (auto i = 0; i < sz; ++i) {
    if (!(item(i).get()->equals(rhs->item(i).get())))
    return false;
  }

  return true;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LSeq::first() const {
  return count() == 0 ? mal::nilValue() : item(0);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LSeq::rest() const {
  VVecIter head = (count() > 0) ? begin() + 1 : end();
  return mal::list(head, end());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(ValueVec* v) : LSeq(v) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(VVecIter begin, VVecIter end) : LSeq(begin, end) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(const LList& rhs, d::ExprValue m) : LSeq(rhs, m) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string LList::toString() const {
  return "(" + LSeq::toString(readably) + ")";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LList::eval(d::IEvaluator* e) {
  /*
    // Note, this isn't actually called since the TCO updates, but
    // is required for the earlier steps, so don't get rid of it.
    if (count() == 0) {
        return malValuePtr(this);
    }

    std::unique_ptr<malValueVec> items(evalItems(env));
    auto it = items->begin();
    malValuePtr op = *it;
    return APPLY(op, ++it, items->end());
    */

}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LList::conj(VVecIter from, VVecIter to) const {
  auto c= std::distance(begin(), end());
  auto t= std::distance(from, to);
  auto out = new ValueVec(c+t);

  std::reverse_copy(from, to, out->begin());
  std::copy(begin(), end(), out->begin() + t);
  return mal::list(out);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LList::withMeta(d::ExprValue m) const {
  return new LList(*this,m);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec::LVec(ValueVec* v) : LSeq(v) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec::LVec(VVecIter s, VVecIter e) : LSeq(s,e) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec::LVec(const LVec& rhs, d::ExprValue m) : LSeq(rhs, m) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LVec::eval(d::IEvaluator* e) {
  return mal::vector(evalItems(e));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string LVec::toString() const {
  return "[" + LSeq::toString(readably) + "]";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LVec::conj(VVecIter froms, VVecIter to) const {
  auto c = std::distance(begin(), end());
  auto t = std::distance(from,to);
  auto out = new ValueVec(c+t);

  std::copy(begin(), end(), out->begin());
  std::copy(from,to, out->begin() + c);

  return mal::vector(out);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LVec::withMeta(d::ExprValue m) const {
  return new LVec(*this, m);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LDict::LDict(VVecIter from, VVecIter to bool isEvaluated) {
  auto c = (int) std::distance(from,to);
  ASSERT(a::isEven(c), "Expected even n# of args, got %d", c);
  for (auto i = from; i != to; ++i) {
    values[hash_key(*i++)] = *i;
  }
  m_isEvaluated=isEvaluated;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LDict::LDict(const std::map<std::string,d::ExprValue>& m) {
  values = new std::map<std::string,d::ExprValue>(m);
  m_isEvaluated=true;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LDict::LDict(const LDict& rhs, d::ExprValue m) : LValue(m) {
  values = rhs.values;
  m_isEvaluated= rhs.m_isEvaluated;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LDict::assoc(VVecIter from, VVecIter to) const {
  auto c = (int) std::distance(from,to);
  ASSERT(a::isEven(c), "Expected even n# of args, got %d", c);
  std::map<std::string,d::ExprValue> m(values);
  for (auto i = from; i != to; ++i) {
    m[hash_key(*i++)] = *i;
  }
  return d::ExprValue(new LDict(m));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LDict::dissoc(VVecIter from, VVecIter to) const {
  std::map<std::string,d::ExprValue> m(values);
  for (auto& i = from; i != to; ++i) {
    m.erase(hash_key(*i));
  }
  return d::ExprValue(new LDict(m));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LDict::contains(d::ExprValue key) const {
  return values.find(hash_key(key)) != values.end();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LDict::eval(d::IEvaluator* e) {
  if (m_isEvaluated) {
    return d::ExprValue(this);
  }
  std::map<std::string,d::ExprValue> out;
  for (auto& i= values.begin(); i != values.end(); ++i) {
    out[i->first] = EVAL(it->second, e);
  }
  return d::ExprValue(new LDict(out));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LDict::get(d::ExprValue key) const {
  auto i = values.find(hash_key(key));
  return (i != values.end()) ? i->second : nil();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LDict::keys() const {
  auto keys = new ValueSet();
  for (auto& x : values) {
    if (x->first[0] == '"') {
      keys->insert(mal::string(unescape(it->first)));
    } else {
      keys->insert(mal::keyword(it->first));
    }
  }
  return d::ExprValue(new LSet(keys));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LDict::values() const {
  auto out = new ValueVec();
  for (auto& x : values) {
    out->push_back(x->second);
  }
  return d::ExprValue(new LList(out));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string LDict::toString() const {
  std::string out;

  for (auto& x : values) {
    if (!out.empty()) {
      out += ",";
    }
    out += x->first + " " + x->second->toString();//print(readably);
  }

  return "{" + out + "}";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LDict::compare(const d::SValue* rhs) const {

  auto const &rvs = static_cast<const LDict*>(rhs)->values;

  if (values.size() != rvs.size()) { return -1; }

  for (auto i = values.begin(), j = rvs.begin(); i != values.end(); ++i,++j) {
    if (i->first != j->first) { return -1; }
    if (!i->second->equals(j->second.get())) { return -1; }
  }

  return 0;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LDict::withMeta(d::ExprValue m) const {
  return new LDict(*this, m);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSet::LSet(VVecIter from, VVecIter to, bool isEvaluated) {
  for (auto i = from; i != to; ++i) {
    values.insert(*i);
  }
  m_isEvaluated=isEvaluated;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSet::LSet(const std::set<d::ExprValue>& m) {
  values = new std::set<d::ExprValue>(m);
  m_isEvaluated=true;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSet::LSet(const LSet& rhs, d::ExprValue m) : LValue(m) {
  values = rhs.values;
  m_isEvaluated= rhs.m_isEvaluated;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LSet::conj(VVecIter from, VVecIter to) const {

  return d::ExprValue(new LSet(m));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LSet::disj(VVecIter from, VVecIter to) const {

  return d::ExprValue(new LSet(m));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LSet::contains(d::ExprValue key) const {
  return values.find(key) != values.end();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LSet::eval(d::IEvaluator* e) {
  if (m_isEvaluated) {
    return d::ExprValue(this);
  }
  std::set<d::ExprValue> out;
  for (auto& i : values) {
    out.insert( EVAL(i, e));
  }
  return d::ExprValue(new LSet(out));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
std::string LSet::toString() const {
  std::string out;

  for (auto& x : values) {
    if (!out.empty()) {
      out += ",";
    }
    out += x->toString();//print(readably);
  }

  return "#{" + out + "}";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LSet::compare(const d::SValue* rhs) const {

  auto const &rvs = static_cast<const LSet*>(rhs)->values;

  if (values.size() != rvs.size()) { return -1; }

  for (auto i = values.begin(), j = rvs.begin(); i != values.end(); ++i,++j) {
    if (!i->equals(j->get())) { return -1; }
  }

  return 0;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::ExprValue LSet::withMeta(d::ExprValue m) const {
  return new LSet(*this, m);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;








//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

