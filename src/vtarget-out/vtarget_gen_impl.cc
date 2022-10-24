/// \file Source for Verilog Verification Targets Generation
///
// --- Hongce Zhang

#include <ilang/vtarget-out/vtarget_gen_impl.h>

#include <cmath>
#include <iostream>

#include <ilang/ila/ast_hub.h>
#include <ilang/util/container_shortcut.h>
#include <ilang/util/fs.h>
#include <ilang/util/log.h>
#include <ilang/util/str_util.h>
#include <ilang/vtarget-out/vtarget_gen_jasper.h>
#include <ilang/vtarget-out/vtarget_gen_pono.h>
// #include <ilang/vtarget-out/vtarget_gen_relchc.h>
// #include <ilang/vtarget-out/vtarget_gen_yosys.h>
// for invariant synthesis
// #include <ilang/vtarget-out/inv-syn/vtarget_gen_inv_abc.h>
// #include <ilang/vtarget-out/inv-syn/vtarget_gen_inv_chc.h>
// #include <ilang/vtarget-out/inv-syn/vtarget_gen_inv_enhance.h>

namespace ilang {

// ------------------------------ VlgVerifTgtGen ---------------------------- //

VlgVerifTgtGen::VlgVerifTgtGen(
    const std::vector<std::string>& implementation_include_path,
    const std::vector<std::string>& implementation_srcs,
    const std::string& implementation_top_module,
    const rfmap::VerilogRefinementMap& refinement,
    const std::string& output_path, const InstrLvlAbsPtr& ila_ptr,
    ModelCheckerSelection backend, const RtlVerifyConfig& vtg_config)
    : _vlg_impl_include_path(implementation_include_path),
      _vlg_impl_srcs(implementation_srcs),
      _vlg_impl_top_name(implementation_top_module), _refinement(refinement),
      _output_path(output_path), _ila_ptr(ila_ptr),
      // configure is only for ila, generate the start signal
      vlg_info_ptr(
          NULL), // not creating it now, because we don't have the info to do so
      _backend(backend), _vtg_config(vtg_config),
      _bad_state(false) {

  if (_ila_ptr == nullptr) {
    ILA_ERROR << "ILA should not be none";
    _bad_state = true;
  }

  // TODO: check more
}

VlgVerifTgtGen::~VlgVerifTgtGen() {
  if (vlg_info_ptr)
    delete vlg_info_ptr;
}

const std::vector<std::string>& VlgVerifTgtGen::GetRunnableScriptName() const {
  return runnable_script_name;
}

void VlgVerifTgtGen::GenerateTargets(void) {
  if (bad_state_return())
    return;

  runnable_script_name.clear();

  vlg_info_ptr = new VerilogInfo(_vlg_impl_include_path, _vlg_impl_srcs, "RTL",
                                 _vlg_impl_top_name);

  if (vlg_info_ptr == NULL || vlg_info_ptr->in_bad_state()) {
    ILA_ERROR << "Unable to generate targets. Verilog parser failed.";
    return; //
  }

  if (!isValidVerifBackend(_backend)) {
    ILA_ERROR << "Unknown backend specification:" << int(_backend) << ", quit.";
    return;
  }

  if (_vtg_config.target_select == RtlVerifyConfig::BOTH ||
      _vtg_config.target_select == RtlVerifyConfig::INV) {
    // check if there are really invariants:
    bool invariantExists = false;
    if (!_refinement.global_invariants.empty())
      invariantExists = true;


    auto sub_output_path = os_portable_append_dir(_output_path, "invariants");
    if (_backend == ModelCheckerSelection::PONO && invariantExists) {
      auto target = VlgSglTgtGen_Pono(
          sub_output_path,
          NULL, // invariant
          _ila_ptr, _refinement, vlg_info_ptr, "wrapper", _vlg_impl_srcs,
          _vlg_impl_include_path, _vtg_config, _backend,
          target_type_t::INVARIANTS);
      target.ConstructWrapper();
      target.ExportAll("wrapper.v", "ila.v", "run.sh", "gen_btor.ys");
      target.do_not_instantiate(); // no use, just for coverage
    } else if (_backend == ModelCheckerSelection::JASPERGOLD && invariantExists) {
      auto target = VlgSglTgtGen_Jasper(
          sub_output_path,
          NULL, // invariant
          _ila_ptr, _refinement, vlg_info_ptr, "wrapper", _vlg_impl_srcs,
          _vlg_impl_include_path, _vtg_config, _backend,
          target_type_t::INVARIANTS);
      target.ConstructWrapper();
      target.ExportAll("wrapper.v", "ila.v", "run.sh", "do.tcl");
      target.do_not_instantiate(); // no use, just for coverage
    }

    if (invariantExists)
      runnable_script_name.push_back(
          os_portable_append_dir(sub_output_path, "run.sh"));
    // end if backend...
  } // end if if(_vtg_config.target_select == BOTH || _vtg_config.target_select
    // == INV)

  // now let's deal w. instructions in rf_cond
  if (_vtg_config.target_select == RtlVerifyConfig::BOTH ||
      _vtg_config.target_select == RtlVerifyConfig::INST) {
    bool generate_forall_inst = _refinement.global_inst_complete_set;

    for (auto&& instr : _refinement.inst_complete_cond) {
      std::string iname = instr.first;
      auto instr_ptr = _ila_ptr->instr(iname);
      ILA_ERROR_IF(instr_ptr == nullptr) 
                << "ila:" << _ila_ptr->name().str()
                << " has no instruction:" << iname;
    }
    for (unsigned inst_idx = 0; inst_idx < _ila_ptr->instr_num() ; ++ inst_idx) {
      auto instr_ptr = _ila_ptr->instr(inst_idx);
      std::string iname = instr_ptr->name().str();
      if (_vtg_config.CheckThisInstructionOnly != "" &&
          _vtg_config.CheckThisInstructionOnly != iname)
        continue; // skip, not checking this instruction
      if (!generate_forall_inst && 
        _refinement.inst_complete_cond.find(iname) == _refinement.inst_complete_cond.end())
        continue;
      
      auto sub_output_path = os_portable_append_dir(_output_path, iname);

      if (_backend == ModelCheckerSelection::PONO) {
        auto target = VlgSglTgtGen_Pono(
            sub_output_path,
            instr_ptr, // instruction
            _ila_ptr, _refinement, vlg_info_ptr, "wrapper", _vlg_impl_srcs,
            _vlg_impl_include_path, _vtg_config, _backend,
            target_type_t::INSTRUCTIONS);
        target.ConstructWrapper();
        target.ExportAll("wrapper.v", "ila.v", "run.sh", "gen_btor.ys");
        target.do_not_instantiate();
      } else if (_backend == ModelCheckerSelection::JASPERGOLD) {
        auto target = VlgSglTgtGen_Jasper(
            sub_output_path,
            instr_ptr, // instruction
            _ila_ptr, _refinement, vlg_info_ptr, "wrapper", _vlg_impl_srcs,
            _vlg_impl_include_path, _vtg_config, _backend,
            target_type_t::INSTRUCTIONS);
        target.ConstructWrapper();
        target.ExportAll("wrapper.v", "ila.v", "run.sh", "do.tcl");
        target.do_not_instantiate();
      }
      runnable_script_name.push_back(
          os_portable_append_dir(sub_output_path, "run.sh"));
    } // end for instrs
  }   // end if target select == ...

  if (vlg_info_ptr) {
    delete vlg_info_ptr;
    vlg_info_ptr = NULL;
  }
} // end of function GenerateTargets

bool VlgVerifTgtGen::bad_state_return(void) {
  ILA_ERROR_IF(_bad_state)
      << "VlgVerifTgtGen is in a bad state, cannot proceed.";
  return _bad_state;
} // bad_state_return


}; // namespace ilang
