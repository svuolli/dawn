// Copyright 2023 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_TRANSFORM_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_TRANSFORM_H_

#include <utility>

#include "src/tint/lang/core/builtin/address_space.h"
#include "src/tint/utils/rtti/castable.h"

// Forward declarations
namespace tint::ir {
class Module;
}  // namespace tint::ir

namespace tint::ir::transform {

/// Interface for IR Module transforms.
class Transform : public utils::Castable<Transform> {
  public:
    /// Constructor
    Transform();
    /// Destructor
    ~Transform() override;

    /// Run the transform on @p module
    /// @param module the source module to transform
    virtual void Run(ir::Module* module) const = 0;
};

}  // namespace tint::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_TRANSFORM_H_
