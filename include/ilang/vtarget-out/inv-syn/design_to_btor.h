/// \file Use Yosys to parser Verilog to Btor
// Hongce Zhang

#ifndef ILANG_VTARGET_DESIGN_TO_BTOR_H__
#define ILANG_VTARGET_DESIGN_TO_BTOR_H__

#include <ilang/vtarget-out/vlg_mod.h>
#include <ilang/vtarget-out/vtarget_gen_impl.h>
#include <ilang/rfmap-in/rfvar_type.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>

namespace ilang {

typedef rfmap::RfMapVarType BtorVarType;

struct BtorStateVars {
  std::map<int, BtorVarType> state_var_without_names;
  std::map<std::string, BtorVarType> state_vars;
};


class DesignToBtor {

public:
  // --------------------- CONSTRUCTOR ---------------------------- //  
  DesignToBtor(); // default constructor
  
  ///
  /// \param[in] output path (ila-verilog, wrapper-verilog, problem.txt,
  /// run-verify-by-???, modify-impl, it there is )
  /// \param[in] all implementation sources
  /// \param[in] all include paths
  /// \param[in] verilog top module name  
  bool YosysParseDesignToBtor(
      const std::string& output_path,  // will be the fname for the generate btor
      const std::vector<std::string>& implementation_srcs,
      const std::vector<std::string>& include_dirs,
      const std::string& module_name,
      const rfmap::ClockSpecification & clock_specification,
      const rfmap::ResetSpecification & reset_specification,
      const _vtg_config & config = _vtg_config()) const;

  void LoadDesignFromBtor(const std::string& btor_fname);
  
  const BtorStateVars & GetBtorInfo() const { return btor_state_vars; }
  
protected:
  
  BtorStateVars btor_state_vars;
};

}; // namespace ilang


#endif // ILANG_VTARGET_DESIGN_TO_BTOR_H__
