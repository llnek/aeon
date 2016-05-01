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
 * Copyright (c) 2013-2016, Kenneth Leung. All rights reserved. */

#pragma once

//////////////////////////////////////////////////////////////////////////////
//
#include "aeon/fusilli.h"
NS_BEGIN(ecs)

#define __decl_comp_tpid(x) \
    virtual const ecs::COMType typeId() { return x; }

//////////////////////////////////////////////////////////////////////////
//
typedef s::string SystemType;
typedef s::string COMType;
typedef s::string NodeType;
typedef int NodeId;

//////////////////////////////////////////////////////////////////////////
//
class Node;
class MS_DLL Component {
friend class Node;
  void setNode(not_null<Node*> n) { _compParentNode=n.get(); }
  __decl_ptr(Node, _compParentNode)
protected:
  Component() {}
public:
  virtual Node* getNode() { return _compParentNode; }
  //auto delete, reference counted?
  virtual bool isAuto() { return false; }
  virtual const COMType typeId() = 0;
  virtual void dispose() {}
  virtual ~Component() {}
};

typedef s::map<NodeId,Component*> CompoCache ;
typedef s::map<NodeId,Node*> NodeCache ;

//////////////////////////////////////////////////////////////////////////
//
enum SPV {
  PreUpdate =  100,
  NetPlay=    200,
  AI=     300,
  Motion=     400,
  Move=       500,
  Logic=   600,
  Collide=  700,
  Resolve=    800,
  Render=     900,

  Error = -1
};



NS_END



