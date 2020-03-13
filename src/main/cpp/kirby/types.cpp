/* Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Copyright © 2013-2020, Kenneth Leung. All rights reserved. */

#include "types.h"

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace czlab::kirby {
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
namespace a= czlab::aeon;
namespace d= czlab::dsl;
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr unescape(const stdstr& src) {
  auto len = src.length();
  auto ch= '\n';
  stdstr res;
  if (len == 0 ||
      !(src[0]=='"' && src[len-1]=='"')) { return src; }
  // skip 1st and last => no dqoutes
  --len;
  for (auto i = 1; i < len; ++i) {
    ch = src[i];
    if (ch == '\\') {
      i += 1;
      res += a::unescape_char(src[i]);
    } else {
      res += ch;
    }
  }
  return res;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr escape(const stdstr& src) {
  auto len = src.length();
  stdstr res;
  if (len == 0 ||
      (src[0]=='"' && src[len-1]=='"')) { return src; }
  for (auto i = 0; i < len; ++i) {
    res += a::escape_char(src[i]);
  }
  return "\"" + res + "\"";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue false_value() {
  return d::DslValue(new LFalse());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue true_value() {
  return d::DslValue(new LTrue());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue nil_value() {
  return d::DslValue(new LNil());
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LValue::withMeta(d::DslValue& m) const {
  throw d::SemanticError("withMeta: not supported here.");
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LValue::meta() const {
  return metaObj.isNull() ? nil_value() : metaObj;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LValue::eval(d::IEvaluator* e) {
  return d::DslValue(this);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LValue::equals(const d::Data* rhs) const {
  return X_NIL(rhs) ? compare(rhs) == 0 : false;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LFalse::toString(bool p) const  {
  return "false";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LFalse::compare(const d::Data* rhs) const {
  return typeid(*this) == typeid(*rhs) ? 0 : 1;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LTrue::toString(bool p) const  {
  return "true";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LTrue::compare(const d::Data* rhs) const {
  return typeid(*this) == typeid(*rhs) ? 0 : 1;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LNil::toString(bool p) const  {
  return "nil";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LNil::compare(const d::Data* rhs) const {
  return typeid(*this) == typeid(*rhs) ? 0 : 1;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNumber::LNumber(d::DslValue& m) : LValue(m) {}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LInt::LInt(const LInt& rhs, d::DslValue& m) : LNumber(m) {
  value= rhs.value;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LInt::toString(bool p) const {
  return std::to_string(value);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LInt::compare(const Data* rhs) const {
  return value == s__cast(const LInt, rhs)->value ? 0 : 1;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LInt::withMeta(d::DslValue& m) const {
  return d::DslValue(new LInt(*this, m));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LFloat::LFloat(const LFloat& rhs, d::DslValue& m) : LNumber(m) {
  value= rhs.value;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LFloat::toString(bool p) const {
  return std::to_string(value);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LFloat::compare(const d::Data* rhs) const {
  return value == s__cast(const LFloat,rhs)->value ? 0 : 1;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LFloat::withMeta(d::DslValue& m) const {
  return d::DslValue(new LFloat(*this, m));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LStrk::LStrk(const LStrk& rhs, d::DslValue& m) : LValue(m) {
  value = rhs.value;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LStrk::compare(const d::Data* rhs) const {
  return value == s__cast(const LString,rhs)->value ? 0 : 1;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LString::LString(const LString& rhs, d::DslValue& m) : LStrk(rhs,m) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LString::LString(stdstr& s) : LStrk(s) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LString::toString(bool p) const {
  return p ? encoded() : value;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LString::encoded() const {
  return escape(value);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr hash_key(d::DslValue& s) {
  auto x = s__cast(LValue, s.ptr());
  if (!x->hashable()) {
    RAISE(d::SemanticError,
        "Can't act as hash-key %s.\n", x->toString().c_str());
  }
  return x->toString();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LString::withMeta(d::DslValue& m) const {
  return d::DslValue(new LString(*this, m));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LKeyword::LKeyword(stdstr& s) : LStrk(s) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LKeyword::LKeyword(const LKeyword& rhs, d::DslValue& m) : LStrk(rhs,m) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LKeyword::toString(bool p) const {
  return ":" +  value;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LKeyword::compare(const d::Data* rhs) const {
  return value == s__cast(const LKeyword,rhs)->value ? 0 : 1;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LKeyword::withMeta(d::DslValue& m) const {
  return d::DslValue(new LKeyword(*this, m));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSymbol::LSymbol(stdstr& s) : LStrk(s) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSymbol::LSymbol(const LSymbol& rhs, d::DslValue& m) : LStrk(rhs, m) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LSymbol::toString(bool p) const {
  return value;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSymbol::eval(d::IEvaluator* e) {
  if (! e->containsSymbol(value)) {
    RAISE(NoSuchVarError,
        "No such symbol %s.\n", value.c_str());
  }
  return e->getValue(value);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LSymbol::compare(const d::Data* rhs) const {
  return value == s__cast(const LSymbol,rhs)->value ? 0 : 1;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSymbol::withMeta(d::DslValue& m) const {
  return d::DslValue(new LSymbol(*this, m));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSeq::LSeq(const LSeq& rhs, d::DslValue& m) : LValue(m) {
  values.clear();
  s__ccat(values,rhs.values);
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSeq::LSeq(ValueVec& chunk) {
  values.clear();
  s__ccat(values, chunk);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LSeq::~LSeq() {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LSeq::toString(bool pretty) const {
  stdstr out;
  for (auto b = values.cbegin(), e=values.cend(); b != e; ++b) {
    if (!out.empty()) { out += " "; }
    out += (*b)->toString(pretty);
  }
  return out;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ValueVec LSeq::evalEach(d::IEvaluator* e) const {
  auto _e = s__cast(Lisper, e);
  ValueVec out;
  for (auto& i : values) {
    auto r= _e->eval(i);
    if (r.isSome()) {
      s__conj(out,r);
    } else {
      RAISE(d::SemanticError,
          "Bad eval %s.\n", this->toString(true).c_str());
    }
  }
  return out;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LSeq::isEmpty() const { return values.empty(); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LSeq::count() const { return values.size(); }
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSeq::get(int pos) const {
  return (pos >= 0 && pos < values.size()) ? values[pos] : nil_value();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LSeq::compare(const d::Data* v) const {
  auto rhs= s__cast(const LSeq,v);
  int sz = count();

  if (sz != rhs->count()) { return false; }

  for (auto i = 0; i < sz; ++i) {
    if (!(get(i)->equals(rhs->get(i).ptr())))
    return false;
  }

  return true;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSeq::first() const {
  return count() == 0 ? nil_value() : get(0);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LSeq::rest() const {
  ValueVec out;
  if (count() > 0)
    for (auto b=values.cbegin()+1,e=values.cend(); b != e; ++b) {
      s__conj(out, *b);
    }
  return d::DslValue(new LList(out));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(ValueVec& v) : LSeq(v) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LList::LList(const LList& rhs, d::DslValue& m) : LSeq(rhs, m) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LList::toString(bool pretty) const {
  return "(" + LSeq::toString(pretty) + ")";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LList::eval(d::IEvaluator* e) {
  if (values.size() == 0) { return d::DslValue(this); }
  auto res= evalEach(e);
  auto op= s__cast(LNativeFn,res[0].ptr());
  ValueVec args;
  args.insert(args.end(), res.begin()+1,res.end());
  return op->apply(args);
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LList::conj(ValueVec& chunk) const {
  ValueVec out;
  std::reverse_copy(chunk.begin(), chunk.end(), out.begin());
  std::copy(values.begin(), values.end(), out.end());
  return d::DslValue(new LList(out));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LList::withMeta(d::DslValue& m) const {
  return d::DslValue(new LList(*this,m));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec::LVec(const LVec& rhs, d::DslValue& m) : LSeq(rhs, m) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LVec::LVec(ValueVec& v) : LSeq(v) {
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LVec::eval(d::IEvaluator* e) {
  auto res= evalEach(e);
  return d::DslValue(new LVec(res));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LVec::toString(bool pretty) const {
  return "[" + LSeq::toString(pretty) + "]";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LVec::conj(ValueVec& more) const {
  ValueVec out;

  std::copy(values.begin(), values.end(), out.end());
  std::copy(more.begin(),more.end(), out.end());

  return d::DslValue(new LVec(out));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LVec::withMeta(d::DslValue& m) const {
  return d::DslValue(new LVec(*this, m));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash::LHash(ValueVec& more) {
  int c = more.size();
  ASSERT(a::is_even(more.size()),
      "Expected even n# of args, got %d", c);
  for (auto i = 0; i < c; ++i) {
    auto k= hash_key(more[i]);
    i += 1;
    values[k] = more[i];
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash::LHash(std::map<stdstr,d::DslValue>& m) {
  values.clear();
  for (auto& x : m) {
    values[x.first] = x.second;
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LHash::LHash(const LHash& rhs, d::DslValue& m) : LValue(m) {
  values.clear();
  for (auto& x : rhs.values) {
    values[x.first]= x.second;
  }
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::assoc(ValueVec& more) const {
  int c = more.size();
  ASSERT(a::is_even(c),
      "Expected even n# of args, got %d", c);
  std::map<stdstr,d::DslValue> m(values);
  for (auto i = 0; i < c; ++i) {
    m[hash_key(more[i])] = more[i+1];
    i += 1;
  }
  return d::DslValue(new LHash(m));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::dissoc(ValueVec& more) const {
  std::map<stdstr,d::DslValue> m(values);
  for (auto& k : more) {
    m.erase(hash_key(k));
  }
  return d::DslValue(new LHash(m));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
bool LHash::contains(d::DslValue& key) const {
  return values.find(hash_key(key)) != values.end();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::eval(d::IEvaluator* e) {
  std::map<stdstr,d::DslValue> out;
  for (auto& it : values) {
    out[it.first] = 0;//EVAL(it.second, e);
  }
  return d::DslValue(new LHash(out));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::get(d::DslValue& key) const {
  auto i = values.find(hash_key(key));
  return (i != values.end()) ? i->second : nil_value();
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::keys() const {
  ValueVec keys;
  for (auto& x : values) {
    auto k = x.first;
    if (k[0] == ':') {
      s__conj(keys, d::DslValue(new LKeyword(k)));
    } else {
      s__conj(keys, d::DslValue(new LString(k)));
    }
  }
  return d::DslValue(new LList(keys));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::vals() const {
  ValueVec out;
  for (auto& x : values) {
    s__conj(out,x.second);
  }
  return d::DslValue(new LList(out));
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LHash::toString(bool pretty) const {
  stdstr out;

  for (auto& x : values) {
    if (!out.empty()) {
      out += ",";
    }
    out += x.first + " " + x.second->toString(pretty);
  }

  return "{" + out + "}";
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LHash::compare(const d::Data* rhs) const {

  auto const &rvs = s__cast(const LHash,rhs)->values;

  if (values.size() != rvs.size()) { return -1; }

  for (auto i = values.begin(),
            e = values.end(),
            j = rvs.begin(); i != e; ++i,++j) {
    if ( (*i).first != (*j).first) { return -1; }
    if (! (*i).second->equals((*j).second.ptr())) { return -1; }
  }

  return 0;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LHash::withMeta(d::DslValue& m) const {
  return d::DslValue(new LHash(*this, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNativeFn::LNativeFn(const stdstr& name, Func* p) : _name(name), fn(p) {
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
LNativeFn::LNativeFn(const LNativeFn& rhs, d::DslValue& m) : LValue(m) {
  _name=rhs._name;
  fn=rhs.fn;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
stdstr LNativeFn::toString(bool pretty) const {
  return "(native)@" + _name;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
int LNativeFn::compare(const d::Data* rhs) const {
  return this == rhs;
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LNativeFn::withMeta(d::DslValue& m) const {
  return d::DslValue(new LNativeFn(*this, m));
}

//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
d::DslValue LNativeFn::apply(ValueVec& args) {
  return fn(args);
}



//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
//;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
//EOF

