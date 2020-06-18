// Copyright 2020 The Tint Authors.
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

#include "src/type_determiner.h"

#include <memory>
#include <vector>

#include "spirv/unified1/GLSL.std.450.h"
#include "src/ast/array_accessor_expression.h"
#include "src/ast/as_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/break_statement.h"
#include "src/ast/call_expression.h"
#include "src/ast/case_statement.h"
#include "src/ast/cast_expression.h"
#include "src/ast/continue_statement.h"
#include "src/ast/else_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/intrinsic.h"
#include "src/ast/loop_statement.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"

namespace tint {
namespace {

// Most of these are floating-point general except the below which are only
// FP16 and FP32. We only have FP32 at this point so the below works, if we
// get FP64 support or otherwise we'll need to differentiate.
//   * radians
//   * degrees
//   * sin, cos, tan
//   * asin, acos, atan
//   * sinh, cosh, tanh
//   * asinh, acosh, atanh
//   * exp, exp2
//   * log, log2
enum class GlslDataType {
  kFloatScalarOrVector,
  kIntScalarOrVector,
  kFloatVector,
  kMatrix
};
struct GlslData {
  const char* name;
  uint8_t param_count;
  uint32_t op_id;
  GlslDataType type;
  uint8_t vector_count;
};

constexpr const GlslData kGlslData[] = {
    {"acos", 1, GLSLstd450Acos, GlslDataType::kFloatScalarOrVector, 0},
    {"acosh", 1, GLSLstd450Acosh, GlslDataType::kFloatScalarOrVector, 0},
    {"asin", 1, GLSLstd450Asin, GlslDataType::kFloatScalarOrVector, 0},
    {"asinh", 1, GLSLstd450Asinh, GlslDataType::kFloatScalarOrVector, 0},
    {"atan", 1, GLSLstd450Atan, GlslDataType::kFloatScalarOrVector, 0},
    {"atan2", 2, GLSLstd450Atan2, GlslDataType::kFloatScalarOrVector, 0},
    {"atanh", 1, GLSLstd450Atanh, GlslDataType::kFloatScalarOrVector, 0},
    {"ceil", 1, GLSLstd450Ceil, GlslDataType::kFloatScalarOrVector, 0},
    {"cos", 1, GLSLstd450Cos, GlslDataType::kFloatScalarOrVector, 0},
    {"cosh", 1, GLSLstd450Cosh, GlslDataType::kFloatScalarOrVector, 0},
    {"cross", 2, GLSLstd450Cross, GlslDataType::kFloatVector, 3},
    {"degrees", 1, GLSLstd450Degrees, GlslDataType::kFloatScalarOrVector, 0},
    {"determinant", 1, GLSLstd450Determinant, GlslDataType::kMatrix, 0},
    {"distance", 2, GLSLstd450Distance, GlslDataType::kFloatScalarOrVector, 0},
    {"exp", 1, GLSLstd450Exp, GlslDataType::kFloatScalarOrVector, 0},
    {"exp2", 1, GLSLstd450Exp2, GlslDataType::kFloatScalarOrVector, 0},
    {"fabs", 1, GLSLstd450FAbs, GlslDataType::kFloatScalarOrVector, 0},
    {"faceforward", 3, GLSLstd450FaceForward,
     GlslDataType::kFloatScalarOrVector, 0},
    {"fclamp", 3, GLSLstd450FClamp, GlslDataType::kFloatScalarOrVector, 0},
    {"findilsb", 1, GLSLstd450FindILsb, GlslDataType::kIntScalarOrVector, 0},
    {"findumsb", 1, GLSLstd450FindUMsb, GlslDataType::kIntScalarOrVector, 0},
    {"findsmsb", 1, GLSLstd450FindSMsb, GlslDataType::kIntScalarOrVector, 0},
    {"floor", 1, GLSLstd450Floor, GlslDataType::kFloatScalarOrVector, 0},
    {"fma", 3, GLSLstd450Fma, GlslDataType::kFloatScalarOrVector, 0},
    {"fmax", 2, GLSLstd450FMax, GlslDataType::kFloatScalarOrVector, 0},
    {"fmin", 2, GLSLstd450FMin, GlslDataType::kFloatScalarOrVector, 0},
    {"fmix", 3, GLSLstd450FMix, GlslDataType::kFloatScalarOrVector, 0},
    {"fract", 1, GLSLstd450Fract, GlslDataType::kFloatScalarOrVector, 0},
    {"fsign", 1, GLSLstd450FSign, GlslDataType::kFloatScalarOrVector, 0},
    {"interpolateatcentroid", 1, GLSLstd450InterpolateAtCentroid,
     GlslDataType::kFloatScalarOrVector, 0},
    {"inversesqrt", 1, GLSLstd450InverseSqrt,
     GlslDataType::kFloatScalarOrVector, 0},
    {"length", 1, GLSLstd450Length, GlslDataType::kFloatScalarOrVector, 0},
    {"log", 1, GLSLstd450Log, GlslDataType::kFloatScalarOrVector, 0},
    {"log2", 1, GLSLstd450Log2, GlslDataType::kFloatScalarOrVector, 0},
    {"matrixinverse", 1, GLSLstd450MatrixInverse, GlslDataType::kMatrix, 0},
    {"nclamp", 3, GLSLstd450NClamp, GlslDataType::kFloatScalarOrVector, 0},
    {"nmax", 2, GLSLstd450NMax, GlslDataType::kFloatScalarOrVector, 0},
    {"nmin", 2, GLSLstd450NMin, GlslDataType::kFloatScalarOrVector, 0},
    {"normalize", 1, GLSLstd450Normalize, GlslDataType::kFloatScalarOrVector,
     0},
    {"pow", 2, GLSLstd450Pow, GlslDataType::kFloatScalarOrVector, 0},
    {"radians", 1, GLSLstd450Radians, GlslDataType::kFloatScalarOrVector, 0},
    {"reflect", 2, GLSLstd450Reflect, GlslDataType::kFloatScalarOrVector, 0},
    {"round", 1, GLSLstd450Round, GlslDataType::kFloatScalarOrVector, 0},
    {"roundeven", 1, GLSLstd450RoundEven, GlslDataType::kFloatScalarOrVector,
     0},
    {"sabs", 1, GLSLstd450SAbs, GlslDataType::kIntScalarOrVector, 0},
    {"sin", 1, GLSLstd450Sin, GlslDataType::kFloatScalarOrVector, 0},
    {"sinh", 1, GLSLstd450Sinh, GlslDataType::kFloatScalarOrVector, 0},
    {"smax", 2, GLSLstd450SMax, GlslDataType::kIntScalarOrVector, 0},
    {"smin", 2, GLSLstd450SMin, GlslDataType::kIntScalarOrVector, 0},
    {"smoothstep", 3, GLSLstd450SmoothStep, GlslDataType::kFloatScalarOrVector,
     0},
    {"sqrt", 1, GLSLstd450Sqrt, GlslDataType::kFloatScalarOrVector, 0},
    {"ssign", 1, GLSLstd450SSign, GlslDataType::kIntScalarOrVector, 0},
    {"step", 2, GLSLstd450Step, GlslDataType::kFloatScalarOrVector, 0},
    {"tan", 1, GLSLstd450Tan, GlslDataType::kFloatScalarOrVector, 0},
    {"tanh", 1, GLSLstd450Tanh, GlslDataType::kFloatScalarOrVector, 0},
    {"trunc", 1, GLSLstd450Trunc, GlslDataType::kFloatScalarOrVector, 0},
    {"umax", 2, GLSLstd450UMax, GlslDataType::kIntScalarOrVector, 0},
    {"umin", 2, GLSLstd450UMin, GlslDataType::kIntScalarOrVector, 0},
};
constexpr const uint32_t kGlslDataCount = sizeof(kGlslData) / sizeof(GlslData);

}  // namespace

TypeDeterminer::TypeDeterminer(Context* ctx, ast::Module* mod)
    : ctx_(*ctx), mod_(mod) {}

TypeDeterminer::~TypeDeterminer() = default;

void TypeDeterminer::set_error(const Source& src, const std::string& msg) {
  error_ = "";
  if (src.line > 0) {
    error_ +=
        std::to_string(src.line) + ":" + std::to_string(src.column) + ": ";
  }
  error_ += msg;
}

bool TypeDeterminer::Determine() {
  for (const auto& var : mod_->global_variables()) {
    variable_stack_.set_global(var->name(), var.get());

    if (var->has_constructor()) {
      if (!DetermineResultType(var->constructor())) {
        return false;
      }
    }
  }

  for (const auto& func : mod_->functions()) {
    name_to_function_[func->name()] = func.get();
  }

  if (!DetermineFunctions(mod_->functions())) {
    return false;
  }
  return true;
}

bool TypeDeterminer::DetermineFunctions(const ast::FunctionList& funcs) {
  for (const auto& func : funcs) {
    if (!DetermineFunction(func.get())) {
      return false;
    }
  }
  return true;
}

bool TypeDeterminer::DetermineFunction(ast::Function* func) {
  variable_stack_.push_scope();
  if (!DetermineStatements(func->body())) {
    return false;
  }
  variable_stack_.pop_scope();

  return true;
}

bool TypeDeterminer::DetermineStatements(const ast::StatementList& stmts) {
  for (const auto& stmt : stmts) {
    if (!DetermineVariableStorageClass(stmt.get())) {
      return false;
    }

    if (!DetermineResultType(stmt.get())) {
      return false;
    }
  }
  return true;
}

bool TypeDeterminer::DetermineVariableStorageClass(ast::Statement* stmt) {
  if (!stmt->IsVariableDecl()) {
    return true;
  }

  auto* var = stmt->AsVariableDecl()->variable();
  // Nothing to do for const
  if (var->is_const()) {
    return true;
  }

  if (var->storage_class() == ast::StorageClass::kFunction) {
    return true;
  }

  if (var->storage_class() != ast::StorageClass::kNone) {
    set_error(stmt->source(),
              "function variable has a non-function storage class");
    return false;
  }

  var->set_storage_class(ast::StorageClass::kFunction);
  return true;
}

bool TypeDeterminer::DetermineResultType(ast::Statement* stmt) {
  if (stmt->IsAssign()) {
    auto* a = stmt->AsAssign();
    return DetermineResultType(a->lhs()) && DetermineResultType(a->rhs());
  }
  if (stmt->IsBreak()) {
    return true;
  }
  if (stmt->IsCase()) {
    auto* c = stmt->AsCase();
    return DetermineStatements(c->body());
  }
  if (stmt->IsContinue()) {
    return true;
  }
  if (stmt->IsElse()) {
    auto* e = stmt->AsElse();
    return DetermineResultType(e->condition()) &&
           DetermineStatements(e->body());
  }
  if (stmt->IsFallthrough()) {
    return true;
  }
  if (stmt->IsIf()) {
    auto* i = stmt->AsIf();
    if (!DetermineResultType(i->condition()) ||
        !DetermineStatements(i->body())) {
      return false;
    }

    for (const auto& else_stmt : i->else_statements()) {
      if (!DetermineResultType(else_stmt.get())) {
        return false;
      }
    }
    return true;
  }
  if (stmt->IsKill()) {
    return true;
  }
  if (stmt->IsLoop()) {
    auto* l = stmt->AsLoop();
    return DetermineStatements(l->body()) &&
           DetermineStatements(l->continuing());
  }
  if (stmt->IsReturn()) {
    auto* r = stmt->AsReturn();
    return DetermineResultType(r->value());
  }
  if (stmt->IsSwitch()) {
    auto* s = stmt->AsSwitch();
    if (!DetermineResultType(s->condition())) {
      return false;
    }
    for (const auto& case_stmt : s->body()) {
      if (!DetermineResultType(case_stmt.get())) {
        return false;
      }
    }
    return true;
  }
  if (stmt->IsVariableDecl()) {
    auto* v = stmt->AsVariableDecl();
    variable_stack_.set(v->variable()->name(), v->variable());
    return DetermineResultType(v->variable()->constructor());
  }

  set_error(stmt->source(), "unknown statement type for type determination");
  return false;
}

bool TypeDeterminer::DetermineResultType(const ast::ExpressionList& list) {
  for (const auto& expr : list) {
    if (!DetermineResultType(expr.get())) {
      return false;
    }
  }
  return true;
}

bool TypeDeterminer::DetermineResultType(ast::Expression* expr) {
  // This is blindly called above, so in some cases the expression won't exist.
  if (!expr) {
    return true;
  }

  if (expr->IsArrayAccessor()) {
    return DetermineArrayAccessor(expr->AsArrayAccessor());
  }
  if (expr->IsAs()) {
    return DetermineAs(expr->AsAs());
  }
  if (expr->IsBinary()) {
    return DetermineBinary(expr->AsBinary());
  }
  if (expr->IsCall()) {
    return DetermineCall(expr->AsCall());
  }
  if (expr->IsCast()) {
    return DetermineCast(expr->AsCast());
  }
  if (expr->IsConstructor()) {
    return DetermineConstructor(expr->AsConstructor());
  }
  if (expr->IsIdentifier()) {
    return DetermineIdentifier(expr->AsIdentifier());
  }
  if (expr->IsMemberAccessor()) {
    return DetermineMemberAccessor(expr->AsMemberAccessor());
  }
  if (expr->IsUnaryOp()) {
    return DetermineUnaryOp(expr->AsUnaryOp());
  }

  set_error(expr->source(), "unknown expression for type determination");
  return false;
}

bool TypeDeterminer::DetermineArrayAccessor(
    ast::ArrayAccessorExpression* expr) {
  if (!DetermineResultType(expr->array())) {
    return false;
  }
  if (!DetermineResultType(expr->idx_expr())) {
    return false;
  }

  auto* res = expr->array()->result_type();
  auto* parent_type = res->UnwrapPtrIfNeeded();
  ast::type::Type* ret = nullptr;
  if (parent_type->IsArray()) {
    ret = parent_type->AsArray()->type();
  } else if (parent_type->IsVector()) {
    ret = parent_type->AsVector()->type();
  } else if (parent_type->IsMatrix()) {
    auto* m = parent_type->AsMatrix();
    ret = ctx_.type_mgr().Get(
        std::make_unique<ast::type::VectorType>(m->type(), m->rows()));
  } else {
    set_error(expr->source(), "invalid parent type in array accessor");
    return false;
  }

  // If we're extracting from a pointer, we return a pointer.
  if (res->IsPointer()) {
    ret = ctx_.type_mgr().Get(std::make_unique<ast::type::PointerType>(
        ret, res->AsPointer()->storage_class()));
  }
  expr->set_result_type(ret);

  return true;
}

bool TypeDeterminer::DetermineAs(ast::AsExpression* expr) {
  if (!DetermineResultType(expr->expr())) {
    return false;
  }

  expr->set_result_type(expr->type());
  return true;
}

bool TypeDeterminer::DetermineCall(ast::CallExpression* expr) {
  if (!DetermineResultType(expr->params())) {
    return false;
  }

  // The expression has to be an identifier as you can't store function pointers
  // but, if it isn't we'll just use the normal result determination to be on
  // the safe side.
  if (expr->func()->IsIdentifier()) {
    auto* ident = expr->func()->AsIdentifier();

    if (ast::intrinsic::IsIntrinsic(ident->name())) {
      if (!DetermineIntrinsic(ident->name(), expr))
        return false;

    } else if (ident->has_path()) {
      auto* imp = mod_->FindImportByName(ident->path());
      if (imp == nullptr) {
        set_error(expr->source(), "Unable to find import for " + ident->name());
        return false;
      }

      uint32_t ext_id = 0;
      auto* result_type = GetImportData(expr->source(), imp->path(),
                                        ident->name(), expr->params(), &ext_id);
      if (result_type == nullptr) {
        if (error_.empty()) {
          set_error(expr->source(),
                    "Unable to determine result type for GLSL expression " +
                        ident->name());
        }
        return false;
      }

      imp->AddMethodId(ident->name(), ext_id);
      expr->func()->set_result_type(result_type);
    } else {
      // An identifier with a single name is a function call, not an import
      // lookup which we can handle with the regular identifier lookup.
      if (!DetermineResultType(ident)) {
        return false;
      }
    }
  } else {
    if (!DetermineResultType(expr->func())) {
      return false;
    }
  }
  expr->set_result_type(expr->func()->result_type());
  return true;
}

bool TypeDeterminer::DetermineIntrinsic(const std::string& name,
                                        ast::CallExpression* expr) {
  if (ast::intrinsic::IsDerivative(name)) {
    if (expr->params().size() != 1) {
      set_error(expr->source(), "incorrect number of parameters for " + name);
      return false;
    }

    // The result type must be the same as the type of the parameter.
    auto& param = expr->params()[0];
    if (!DetermineResultType(param.get())) {
      return false;
    }
    expr->func()->set_result_type(param->result_type()->UnwrapPtrIfNeeded());
    return true;
  }
  if (name == "any" || name == "all") {
    expr->func()->set_result_type(
        ctx_.type_mgr().Get(std::make_unique<ast::type::BoolType>()));
    return true;
  }
  if (ast::intrinsic::IsFloatClassificationIntrinsic(name)) {
    if (expr->params().size() != 1) {
      set_error(expr->source(), "incorrect number of parameters for " + name);
      return false;
    }

    auto* bool_type =
        ctx_.type_mgr().Get(std::make_unique<ast::type::BoolType>());

    auto* param_type = expr->params()[0]->result_type()->UnwrapPtrIfNeeded();
    if (param_type->IsVector()) {
      expr->func()->set_result_type(
          ctx_.type_mgr().Get(std::make_unique<ast::type::VectorType>(
              bool_type, param_type->AsVector()->size())));
    } else {
      expr->func()->set_result_type(bool_type);
    }
    return true;
  }
  if (name == "dot") {
    expr->func()->set_result_type(
        ctx_.type_mgr().Get(std::make_unique<ast::type::F32Type>()));
    return true;
  }
  if (name == "outer_product") {
    if (expr->params().size() != 2) {
      set_error(expr->source(),
                "incorrect number of parameters for outer_product");
      return false;
    }

    auto* param0_type = expr->params()[0]->result_type()->UnwrapPtrIfNeeded();
    auto* param1_type = expr->params()[1]->result_type()->UnwrapPtrIfNeeded();
    if (!param0_type->IsVector() || !param1_type->IsVector()) {
      set_error(expr->source(), "invalid parameter type for outer_product");
      return false;
    }

    expr->func()->set_result_type(
        ctx_.type_mgr().Get(std::make_unique<ast::type::MatrixType>(
            ctx_.type_mgr().Get(std::make_unique<ast::type::F32Type>()),
            param0_type->AsVector()->size(), param1_type->AsVector()->size())));
    return true;
  }

  return false;
}

bool TypeDeterminer::DetermineCast(ast::CastExpression* expr) {
  if (!DetermineResultType(expr->expr())) {
    return false;
  }

  expr->set_result_type(expr->type());
  return true;
}

bool TypeDeterminer::DetermineConstructor(ast::ConstructorExpression* expr) {
  if (expr->IsTypeConstructor()) {
    auto* ty = expr->AsTypeConstructor();
    for (const auto& value : ty->values()) {
      if (!DetermineResultType(value.get())) {
        return false;
      }
    }
    expr->set_result_type(ty->type());
  } else {
    expr->set_result_type(expr->AsScalarConstructor()->literal()->type());
  }
  return true;
}

bool TypeDeterminer::DetermineIdentifier(ast::IdentifierExpression* expr) {
  if (expr->has_path()) {
    set_error(expr->source(),
              "determine identifier should not be called with imports");
    return false;
  }

  auto name = expr->name();
  ast::Variable* var;
  if (variable_stack_.get(name, &var)) {
    // A constant is the type, but a variable is always a pointer so synthesize
    // the pointer around the variable type.
    if (var->is_const()) {
      expr->set_result_type(var->type());
    } else {
      expr->set_result_type(
          ctx_.type_mgr().Get(std::make_unique<ast::type::PointerType>(
              var->type(), var->storage_class())));
    }
    return true;
  }

  auto iter = name_to_function_.find(name);
  if (iter != name_to_function_.end()) {
    expr->set_result_type(iter->second->return_type());
    return true;
  }

  return true;
}

bool TypeDeterminer::DetermineMemberAccessor(
    ast::MemberAccessorExpression* expr) {
  if (!DetermineResultType(expr->structure())) {
    return false;
  }

  auto* res = expr->structure()->result_type();
  auto* data_type = res->UnwrapPtrIfNeeded();

  while (data_type->IsAlias()) {
    data_type = data_type->AsAlias()->type();
  }

  ast::type::Type* ret = nullptr;
  if (data_type->IsStruct()) {
    auto* strct = data_type->AsStruct()->impl();
    auto name = expr->member()->name();

    for (const auto& member : strct->members()) {
      if (member->name() == name) {
        ret = member->type();
        break;
      }
    }

    if (ret == nullptr) {
      set_error(expr->source(), "struct member " + name + " not found");
      return false;
    }

    // If we're extracting from a pointer, we return a pointer.
    if (res->IsPointer()) {
      ret = ctx_.type_mgr().Get(std::make_unique<ast::type::PointerType>(
          ret, res->AsPointer()->storage_class()));
    }
  } else if (data_type->IsVector()) {
    auto* vec = data_type->AsVector();

    auto size = expr->member()->name().size();
    if (size == 1) {
      // A single element swizzle is just the type of the vector.
      ret = vec->type();
      // If we're extracting from a pointer, we return a pointer.
      if (res->IsPointer()) {
        ret = ctx_.type_mgr().Get(std::make_unique<ast::type::PointerType>(
            ret, res->AsPointer()->storage_class()));
      }
    } else {
      // The vector will have a number of components equal to the length of the
      // swizzle. This assumes the validator will check that the swizzle
      // is correct.
      ret = ctx_.type_mgr().Get(
          std::make_unique<ast::type::VectorType>(vec->type(), size));
    }
  } else {
    set_error(expr->source(),
              "invalid type " + data_type->type_name() + " in member accessor");
    return false;
  }

  expr->set_result_type(ret);

  return true;
}

bool TypeDeterminer::DetermineBinary(ast::BinaryExpression* expr) {
  if (!DetermineResultType(expr->lhs()) || !DetermineResultType(expr->rhs())) {
    return false;
  }

  // Result type matches first parameter type
  if (expr->IsAnd() || expr->IsOr() || expr->IsXor() || expr->IsShiftLeft() ||
      expr->IsShiftRight() || expr->IsAdd() || expr->IsSubtract() ||
      expr->IsDivide() || expr->IsModulo()) {
    expr->set_result_type(expr->lhs()->result_type()->UnwrapPtrIfNeeded());
    return true;
  }
  // Result type is a scalar or vector of boolean type
  if (expr->IsLogicalAnd() || expr->IsLogicalOr() || expr->IsEqual() ||
      expr->IsNotEqual() || expr->IsLessThan() || expr->IsGreaterThan() ||
      expr->IsLessThanEqual() || expr->IsGreaterThanEqual()) {
    auto* bool_type =
        ctx_.type_mgr().Get(std::make_unique<ast::type::BoolType>());
    auto* param_type = expr->lhs()->result_type()->UnwrapPtrIfNeeded();
    if (param_type->IsVector()) {
      expr->set_result_type(
          ctx_.type_mgr().Get(std::make_unique<ast::type::VectorType>(
              bool_type, param_type->AsVector()->size())));
    } else {
      expr->set_result_type(bool_type);
    }
    return true;
  }
  if (expr->IsMultiply()) {
    auto* lhs_type = expr->lhs()->result_type()->UnwrapPtrIfNeeded();
    auto* rhs_type = expr->rhs()->result_type()->UnwrapPtrIfNeeded();

    // Note, the ordering here matters. The later checks depend on the prior
    // checks having been done.
    if (lhs_type->IsMatrix() && rhs_type->IsMatrix()) {
      expr->set_result_type(
          ctx_.type_mgr().Get(std::make_unique<ast::type::MatrixType>(
              lhs_type->AsMatrix()->type(), lhs_type->AsMatrix()->rows(),
              rhs_type->AsMatrix()->columns())));

    } else if (lhs_type->IsMatrix() && rhs_type->IsVector()) {
      auto* mat = lhs_type->AsMatrix();
      expr->set_result_type(ctx_.type_mgr().Get(
          std::make_unique<ast::type::VectorType>(mat->type(), mat->rows())));
    } else if (lhs_type->IsVector() && rhs_type->IsMatrix()) {
      auto* mat = rhs_type->AsMatrix();
      expr->set_result_type(
          ctx_.type_mgr().Get(std::make_unique<ast::type::VectorType>(
              mat->type(), mat->columns())));
    } else if (lhs_type->IsMatrix()) {
      // matrix * scalar
      expr->set_result_type(lhs_type);
    } else if (rhs_type->IsMatrix()) {
      // scalar * matrix
      expr->set_result_type(rhs_type);
    } else if (lhs_type->IsVector() && rhs_type->IsVector()) {
      expr->set_result_type(lhs_type);
    } else if (lhs_type->IsVector()) {
      // Vector * scalar
      expr->set_result_type(lhs_type);
    } else if (rhs_type->IsVector()) {
      // Scalar * vector
      expr->set_result_type(rhs_type);
    } else {
      // Scalar * Scalar
      expr->set_result_type(lhs_type);
    }

    return true;
  }

  set_error(expr->source(), "Unknown binary expression");
  return false;
}

bool TypeDeterminer::DetermineUnaryOp(ast::UnaryOpExpression* expr) {
  // Result type matches the parameter type.
  if (!DetermineResultType(expr->expr())) {
    return false;
  }
  expr->set_result_type(expr->expr()->result_type()->UnwrapPtrIfNeeded());
  return true;
}

ast::type::Type* TypeDeterminer::GetImportData(
    const Source& source,
    const std::string& path,
    const std::string& name,
    const ast::ExpressionList& params,
    uint32_t* id) {
  if (path != "GLSL.std.450") {
    return nullptr;
  }

  const GlslData* data = nullptr;
  for (uint32_t i = 0; i < kGlslDataCount; ++i) {
    if (name == kGlslData[i].name) {
      data = &kGlslData[i];
      break;
    }
  }
  if (data == nullptr) {
    return nullptr;
  }

  if (params.size() != data->param_count) {
    set_error(source, "incorrect number of parameters for " + name +
                          ". Expected " + std::to_string(data->param_count) +
                          " got " + std::to_string(params.size()));
    return nullptr;
  }

  std::vector<ast::type::Type*> result_types;
  for (uint32_t i = 0; i < data->param_count; ++i) {
    result_types.push_back(params[i]->result_type()->UnwrapPtrIfNeeded());

    switch (data->type) {
      case GlslDataType::kFloatScalarOrVector:
        if (!result_types.back()->is_float_scalar_or_vector()) {
          set_error(source, "incorrect type for " + name + ". " +
                                "Requires float scalar or float vector values");
          return nullptr;
        }

        break;
      case GlslDataType::kIntScalarOrVector:
        if (!result_types.back()->is_integer_scalar_or_vector()) {
          set_error(source,
                    "incorrect type for " + name + ". " +
                        "Requires integer scalar or integer vector values");
          return nullptr;
        }
        break;
      case GlslDataType::kFloatVector:
        if (!result_types.back()->is_float_vector()) {
          set_error(source, "incorrect type for " + name + ". " +
                                "Requires float vector values");
          return nullptr;
        }
        if (data->vector_count > 0 &&
            result_types.back()->AsVector()->size() != data->vector_count) {
          set_error(source,
                    "incorrect vector size for " + name + ". " + "Requires " +
                        std::to_string(data->vector_count) + " elements");
          return nullptr;
        }
        break;
      case GlslDataType::kMatrix:
        if (!result_types.back()->IsMatrix()) {
          set_error(source,
                    "incorrect type for " + name + ". Requires matrix value");
          return nullptr;
        }
        break;
    }
  }

  // Verify all the parameter types match
  for (size_t i = 1; i < data->param_count; ++i) {
    if (result_types[0] != result_types[i]) {
      error_ = "mismatched parameter types for " + name;
      return nullptr;
    }
  }

  *id = data->op_id;

  // Handle functions which aways return the type, even if a vector is provided.
  if (name == "length" || name == "distance") {
    return result_types[0]->is_float_scalar()
               ? result_types[0]
               : result_types[0]->AsVector()->type();
  }
  // The determinant returns the component type of the columns
  if (name == "determinant") {
    return result_types[0]->AsMatrix()->type();
  }
  return result_types[0];
}

}  // namespace tint
