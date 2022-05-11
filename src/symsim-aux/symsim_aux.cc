/// \file Generating sym-sim related auxiliary information
// ---Hongce Zhang

#include <ilang/symsim-aux/symsim_aux.h>
#include <ilang/util/log.h>
#include <ilang/util/container_shortcut.h>
#include <ilang/target-smt/z3_expr_adapter.h>

namespace ilang {



InstrUpdateFunGenerator::vlg_name_t
InstrUpdateFunGenerator::getVlgFromExpr(const ExprPtr& e) {

  auto pos = nmap.find(e);
  ILA_ASSERT(pos != nmap.end())
      << "Expr:" << (e) << " has not been translated yet";
  return pos->second;
}
InstrUpdateFunGenerator::vlg_name_t InstrUpdateFunGenerator::getArg(const ExprPtr& e,
                                                      const size_t& i) {
  auto arg_i = e->arg(i);
  return getVlgFromExpr(arg_i);
}


void InstrUpdateFunGenerator::parseArg(const ExprPtr& e) {
  for (size_t i = 0; i != e->arg_num(); ++i) {
    ParseNonMemUpdateExpr(e->arg(i));
  }
}

// will be used by ParseNonMemUpdateExpr, will not be directly called by
// ParseMemUpdateNode the later will call the former first
InstrUpdateFunGenerator::vlg_name_t
InstrUpdateFunGenerator::translateBoolOp(const std::shared_ptr<ExprOp>& e) {

  vlg_stmt_t result_stmt;
  std::string op_name = e->op_name();
  size_t arg_num = e->arg_num();

  if (op_name == "APP") { // Function application
    // deal with the case with a function
    ILA_ASSERT(false) << "TODO: not implemented";
    result_stmt = "__ERROR__";
  } else if (arg_num == 1) {
    if (op_name == "NOT")
      result_stmt = "~ ( " + getArg(e, 0) + " ) ";
    else
      ILA_ASSERT(false) << op_name << " is not supported by VerilogGenerator";
  } else if (arg_num == 2) {
    auto arg1 = getArg(e, 0);
    auto arg2 = getArg(e, 1);
    if (op_name == "AND")
      result_stmt = " ( " + arg1 + " ) & (" + arg2 + " ) ";
    else if (op_name == "OR")
      result_stmt = " ( " + arg1 + " ) | ( " + arg2 + " ) ";
    else if (op_name == "XOR")
      result_stmt = " ( " + arg1 + " ) ^ ( " + arg2 + " ) ";
    else if (op_name == "EQ")
      result_stmt = " ( " + arg1 + " ) == ( " + arg2 + " ) ";
    else if (op_name == "IMPLY")
      result_stmt = " ( ~ ( " + arg1 + " ) | ( " + arg2 +
                    " ) )"; // do we need to support boolean comparison?
    else if (op_name == "LT")
      result_stmt =
          vlg_stmt_t(" $signed( ") + arg1 + " ) < $signed( " + arg2 + " ) ";
    else if (op_name == "GT")
      result_stmt =
          vlg_stmt_t(" $signed( ") + arg1 + " ) > $signed( " + arg2 + " ) ";
    else if (op_name == "ULT")
      result_stmt = vlg_stmt_t(" ( ") + arg1 + " ) < ( " + arg2 + " ) ";
    else if (op_name == "UGT")
      result_stmt = vlg_stmt_t(" ( ") + arg1 + " ) > ( " + arg2 + " ) ";
    else
      ILA_ASSERT(false) << op_name << " is not supported by VerilogGenerator";
  } else if (arg_num == 3) {
    auto arg1 = getArg(e, 0);
    auto arg2 = getArg(e, 1);
    auto arg3 = getArg(e, 2);
    if (op_name == "ITE")
      result_stmt = " ( " + arg1 + " ) ? ( " + arg2 + " ) : ( " + arg3 + " ) ";
    else
      ILA_ASSERT(false) << op_name << " is not supported by VerilogGenerator";
  }
  vlg_name_t result_var = new_id(e);
  add_wire(result_var, 1);
  add_assign_stmt(result_var, result_stmt);
  return result_var;
}


// will be used by ParseNonMemUpdateExpr, will not be directly called by
// ParseMemUpdateNode the later will call the former first
InstrUpdateFunGenerator::vlg_name_t
InstrUpdateFunGenerator::translateBvOp(const std::shared_ptr<ExprOp>& e) {

  vlg_stmt_t result_stmt;
  std::string op_name = e->op_name();
  size_t arg_num = e->arg_num();

  if (op_name == "APP") { // Function application
    // deal with the case with a function
    ILA_ASSERT(false) << "TODO: not implemented";
    result_stmt = "__ERROR__";
  } else if (arg_num == 1) {
    vlg_name_t arg0 = getArg(e, 0);
    if (op_name == "NEGATE") // negate : 2's complement
      result_stmt = vlg_stmt_t("( ~ ( ") + arg0 + " ) + 1'b1 )";
    else if (op_name == "COMPLEMENT") // 1's complement
      result_stmt = vlg_stmt_t("~ ( ") + arg0 + " )";
    else if (op_name == "EXTRACT") {
      int hi = e->param(0);
      int lo = e->param(1);
      result_stmt = arg0 + "[" + std::to_string(hi) + ":" + std::to_string(lo) + "]";
    } else if (op_name == "ZERO_EXTEND") {
      int outw = e->param(0);
      int inw = get_width(e->arg(0));
      if (outw == inw)
        result_stmt = arg0;
      else
        result_stmt =
            vlg_stmt_t(" {") + std::to_string(outw - inw) + "'d0 , " + arg0 + "} ";
    } else if (op_name == "SIGN_EXTEND") {
      int outw = e->param(0);
      int inw = get_width(e->arg(0));
      if (outw == inw)
        result_stmt = arg0;
      else if (e->arg(0)->is_const() && inw == 1) {
        result_stmt = vlg_stmt_t(" { {") + std::to_string(outw - inw) + "{" + arg0 +
                      "} }, " + arg0 + "} ";
      } else
        result_stmt = vlg_stmt_t(" { {") + std::to_string(outw - inw) + "{" + arg0 +
                      "[" + std::to_string(inw - 1) + "]} }, " + arg0 + "} ";
    } else if (op_name == "RIGHT_ROTATE") {
      // {x[i-1:0], x[w-1:i]}
      int rotw = e->param(0);
      int inw = get_width(e->arg(0));
      result_stmt = vlg_stmt_t(" { ( ") + arg0 + "[" + std::to_string(rotw - 1) +
                    ":0] ), ( " + arg0 + "[" + std::to_string(inw - 1) + ":" +
                    std::to_string(rotw) + "] ) } ";
    } else if (op_name == "LEFT_ROTATE") {
      // {x[w-1-i:0], x[w-1:w-i]}
      int rotw = e->param(0);
      int inw = get_width(e->arg(0));
      result_stmt = vlg_stmt_t(" { ( ") + arg0 + "[" + std::to_string(inw - 1 - rotw) +
                    ":0] ), ( " + arg0 + "[" + std::to_string(inw - 1) + ":" +
                    std::to_string(inw - rotw) + "] ) } ";
    } else
      ILA_ASSERT(false) << op_name << " is not supported by VerilogGenerator";
  } // else if(arg_num == 1)
  else if (arg_num == 2) {
    vlg_name_t arg1 = getArg(e, 0);
    vlg_name_t arg2 = getArg(e, 1);
    if (op_name == "AND")
      result_stmt = vlg_stmt_t(" ( ") + arg1 + " ) & ( " + arg2 + " ) ";
    else if (op_name == "OR")
      result_stmt = vlg_stmt_t(" ( ") + arg1 + " ) | ( " + arg2 + " ) ";
    else if (op_name == "XOR")
      result_stmt = vlg_stmt_t(" ( ") + arg1 + " ) ^ ( " + arg2 + " ) ";
    else if (op_name == "SHL") // only shift, use 0 on the right
      result_stmt = vlg_stmt_t(" ( ") + arg1 + " ) << ( " + arg2 + " ) ";
    else if (op_name == "ASHR") // arithmetic shift right
      result_stmt =
          vlg_stmt_t(" ( $signed( ") + arg1 + " ) >>> ( " + arg2 + " )) ";
    else if (op_name == "LSHR")
      result_stmt = vlg_stmt_t(" ( ( ") + arg1 + " ) >> ( " + arg2 + " )) ";
    else if (op_name == "ADD")
      result_stmt = vlg_stmt_t(" ( ") + arg1 + " ) + ( " + arg2 + " ) ";
    else if (op_name == "SUB")
      result_stmt = vlg_stmt_t(" ( ") + arg1 + " ) - ( " + arg2 + " ) ";
    else if (op_name == "MUL")
      result_stmt = vlg_stmt_t(" ( ") + arg1 + " ) * ( " + arg2 + " ) ";
    else if (op_name == "DIV")
      result_stmt = vlg_stmt_t(" ( ") + arg1 + " ) / ( " + arg2 + " ) ";
    else if (op_name == "UREM")
      result_stmt = vlg_stmt_t(" ( ") + arg1 + " ) % ( " + arg2 + " ) ";
    else if (op_name == "CONCAT")
      result_stmt = vlg_stmt_t(" { ( ") + arg1 + " ) , ( " + arg2 + " ) } ";
    else if (op_name == "LOAD") {
      // arg1 should be the memvar // arg2 should be the address
      // in the future, we may need to avoid the leaves first traverse to
      // account for the LOAD(STORE) LOAD(ITE)
      ILA_ASSERT(false) << "TODO: not implemented.";
      result_stmt = "__ERROR__";
    } // end of else if(op_name == "LOAD")
    else
      ILA_ASSERT(false) << op_name << " is not supported by VerilogGenerator";
  } // end of else if(arg_num == 2)
  else if (arg_num == 3) {
    vlg_name_t arg1 = getArg(e, 0);
    vlg_name_t arg2 = getArg(e, 1);
    vlg_name_t arg3 = getArg(e, 2);
    if (op_name == "ITE")
      result_stmt =
          vlg_stmt_t(" ( ") + arg1 + " ) ? ( " + arg2 + " ) : ( " + arg3 + " )";
    else
      ILA_ASSERT(false) << op_name << " is not supported by VerilogGenerator";
  } // else if(arg_num == 3)
  else
    ILA_ASSERT(false) << op_name << " is not supported by VerilogGenerator";

  vlg_name_t result_var = new_id(e);
  add_wire(result_var, get_width(e));
  add_assign_stmt(result_var, result_stmt);
  return result_var;
} // end of translateBvOp


// used in general for an expression (not the update of a memvar)
void InstrUpdateFunGenerator::ParseNonMemUpdateExpr(
    const ExprPtr& e) { // will be used in parsing state update of non mem and
                        // decode function
  // memorize
  ILA_DLOG("VerilogGen.ParseNonMemUpdateExpr") << "Parsing:" << e;
  if (nmap.find(e) != nmap.end()) {
    ILA_DLOG("VerilogGen.ParseNonMemUpdateExpr") << "Cached.";
    return;
  }

  if (e->is_bool()) {
    if (e->is_var()) {
      auto signame = sanitizeName(e);
      ILA_CHECK(IN(signame, input_signals));
      nmap[e] = signame; // just use its name
      ILA_DLOG("VerilogGen.ParseNonMemUpdateExpr")
          << "BoolVar: " << e->name().str();
    } else if (e->is_op()) { // bool op
      // leaves first,
      ILA_DLOG("VerilogGen.ParseNonMemUpdateExpr") << "BoolOp, leaves-first ";
      parseArg(e);
      std::shared_ptr<ExprOp> expr_op_ptr =
          std::dynamic_pointer_cast<ExprOp>(e);
      ILA_NOT_NULL(expr_op_ptr);
      nmap[e] = translateBoolOp(expr_op_ptr);
    } else if (e->is_const()) { // bool const
      vlg_name_t bcnst =
          vlg_name_t("1'b") +
          (std::dynamic_pointer_cast<ExprConst>(e)->val_bool()->val() ? "1"
                                                                      : "0");
      ILA_DLOG("VerilogGen.ParseNonMemUpdateExpr") << "BoolConst: " << bcnst;
      nmap[e] = bcnst;
    } else
      ILA_ASSERT(false) << "Expr sort: " << (e->sort()) << " is not supported.";
  } else if (e->is_bv()) {
    if (e->is_var()) {
      auto signame = sanitizeName(e);
      ILA_CHECK(IN(signame, input_signals));
      nmap[e] = signame; // just use its name
      ILA_DLOG("VerilogGen.ParseNonMemUpdateExpr") << "BV: " << e->name().str();
    } else if (e->is_op()) {
      // leaves first
      std::shared_ptr<ExprOp> expr_op_ptr =
          std::dynamic_pointer_cast<ExprOp>(e);
      ILA_NOT_NULL(expr_op_ptr);

      ILA_DLOG("VerilogGen.ParseNonMemUpdateExpr") << "BVop, leaves-first ";
      parseArg(e); // if not LOAD, leaf-first
      // BTW, you cannot cache the LOAD(STORE/ITE/MEMCONST) pattern
      nmap[e] = translateBvOp(expr_op_ptr);
    } else if (e->is_const()) {
      int width = get_width(e);
      ILA_ASSERT(width > 0);
      IlaBvValType value =
          (std::dynamic_pointer_cast<ExprConst>(e)->val_bv()->val());
      vlg_name_t result_var;

      auto pos = cmap.find(std::make_pair(value, (unsigned)width));
      if (pos == cmap.end()) { // not found
        vlg_const_t bvcnst = ToVlgNum(value, (unsigned)width);
        result_var = "bv_" + std::to_string(width) + "_" +
                     std::to_string(IlaBvValUnsignedType(value)) + "_" + new_id(e);
        add_wire(result_var, get_width(e));
        add_assign_stmt(result_var, bvcnst);

        ILA_DLOG("VerilogGen.ParseNonMemUpdateExpr")
            << "BVconst: " << bvcnst << " as " << result_var;
        cmap.insert(
            std::make_pair(std::make_pair(value, (unsigned)width), result_var));
      } else { // found
        result_var = pos->second;
      }
      nmap[e] = result_var;
    } else
      ILA_ASSERT(false) << "Expr sort: " << (e->sort()) << " is not supported.";
  } else if (e->is_mem()) {
    // TODO: ?
      ILA_ASSERT(false)
          << "Implementation bug, do not support mem_op ( "
             "LOAD(STORE/ITE/MEMCONST) pattern ) in non-mem-update expression";
  } else
    ILA_ASSERT(false) << "Expr sort: " << (e->sort()) << " is not supported.";
} // end of ParseNonMemUpdateExpr

//--------------------------------------------------------------------------------

void InstrUpdateFunGenerator::InsertInputVar(const ExprPtr& e) {
  ILA_CHECK(e->is_var());
  auto signame = sanitizeName(e);
  auto width = get_width(e);
  input_signals[signame] = width;
  add_input(signame, width);
  add_wire(signame, width);
}

std::string InstrUpdateFunGenerator::InsertOutputVar(const ExprPtr& e) {
  ILA_CHECK(e->is_var());
  auto signame = sanitizeName(e)+"_next";
  auto width = get_width(e);
  input_signals[signame] = width;
  add_output(signame, width);
  add_wire(signame, width);
  return signame;
}

void InstrUpdateFunGenerator::ExportInstrUpdate(const InstrPtr& instr_ptr_) {
  ILA_NOT_NULL(instr_ptr_);
  auto host_ = instr_ptr_->host();
  ILA_NOT_NULL(host_);
  for (size_t input_idx = 0; input_idx < host_->input_num(); ++ input_idx)
    InsertInputVar(host_->input(input_idx));
  for (size_t sv_idx = 0; sv_idx < host_->state_num(); ++ sv_idx)
    InsertInputVar(host_->state(sv_idx));
  
  // insert update for inst_update
  for (size_t sv_idx = 0; sv_idx < host_->state_num(); ++ sv_idx) {
    auto update_expr = instr_ptr_->update(host_->state(sv_idx));
    auto output_var_name = InsertOutputVar(host_->state(sv_idx));
    ILA_WARN_IF(update_expr == nullptr) << "state var `" << host_->state(sv_idx)->name().str() 
      << "` is not updated by instruction `" << instr_ptr_->name().str() <<"`";
    if (update_expr != nullptr) {
      ParseNonMemUpdateExpr(update_expr);
      auto result_var = getVlgFromExpr(update_expr);
      add_assign_stmt(output_var_name, result_var);
    }
  }  
} // END of ExportInstrUpdate

void InstrUpdateFunGenerator::ExportInstrUpdateSmt2(const InstrPtr& instr_ptr_, std::map<std::string,std::string> & output ) {
  ILA_NOT_NULL(instr_ptr_);
  auto host_ = instr_ptr_->host();
  ILA_NOT_NULL(host_);

  // insert update for inst_update
  for (size_t sv_idx = 0; sv_idx < host_->state_num(); ++ sv_idx) {
    auto sv = host_->state(sv_idx);
    auto update_expr = instr_ptr_->update(sv);
    z3::context c;
    Z3ExprAdapter adapter(c);
    z3::solver s(c);

    auto update_z3expr = adapter.GetExpr(update_expr);
    auto sv_z3expr = adapter.GetExpr(sv);
    s.add(update_z3expr == sv_z3expr);
    auto strexpr = s.to_smt2();
    output.emplace(sv->name().str(), strexpr);
  }
}

} // namespace ilang

