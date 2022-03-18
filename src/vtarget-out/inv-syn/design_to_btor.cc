
#include <ilang/util/container_shortcut.h>
#include <ilang/util/log.h>
#include <ilang/util/str_util.h>
#include <ilang/util/fs.h>
#include <ilang/vtarget-out/inv-syn/design_to_btor.h>

#include <fstream>

namespace ilang {

static std::string yosysGenerateBtor = R"***(
hierarchy -check
proc
chformal -assume -early;
%propselect%
memory %-nomap%;
flatten
setundef -undriven -expose;
sim -clock %clk% -reset rst -rstlen %rstlen% -n %cycle% -w %module%
)***";
// %propselect% is for

DesignToBtor::DesignToBtor() {}

void DesignToBtor::LoadDesignFromBtor(const std::string& btor_fname) {
  std::ifstream fin(btor_fname);

  ILA_ERROR_IF(!fin.is_open()) << "unable to write to " << btor_fname;
  
  std::string line;
  std::map<int, BtorVarType> sorts;
  
  btor_state_vars.state_vars.clear();
  btor_state_vars.state_var_without_names.clear();

  while(std::getline(fin, line)) {
    auto line_split = SplitSpaceTabEnter(line);
    if(line_split.size() >= 2 && line_split[1] == "sort") {
      ILA_CHECK(line_split.size() >= 4);
      int id = StrToInt(line_split[0]);
      if (line_split[2] == "bitvec") {
        int w = StrToInt(line_split[3]);
        BtorVarType tp(w);
        sorts.emplace(id, tp);
      } else if (line_split[2] == "array") {
        ILA_CHECK(line_split.size() >= 5);
        int sortid_addr = StrToInt(line_split[3]);
        int sortid_data = StrToInt(line_split[4]);
        ILA_CHECK(IN(sortid_addr, sorts));
        ILA_CHECK(IN(sortid_data, sorts));
        ILA_CHECK(sorts[sortid_addr].is_bv());
        ILA_CHECK(sorts[sortid_data].is_bv());
        int addrw = sorts[sortid_addr].unified_width();
        int dataw = sorts[sortid_data].unified_width();
        BtorVarType tp(addrw,dataw);
        sorts.emplace(id, tp);     
      } // others: unknown sort
    } else if (line_split.size() >= 2 && line_split[1] == "state") {
      ILA_CHECK(line_split.size() >= 3);
      int sortid = StrToInt(line_split[2]);
      ILA_CHECK(IN(sortid, sorts));
      auto sort = sorts[sortid];
      if(line_split.size() == 3) {
        int id = StrToInt(line_split[0]);
        btor_state_vars.state_var_without_names.emplace(id, sort);
      } else { // line_split.size() > 3
        btor_state_vars.state_vars.emplace(line_split[3], sort);
      }
    } else if (line_split.size() >= 4 && line_split[1] == "output") {
      int sid = StrToInt(line_split[2]);
      auto pos = btor_state_vars.state_var_without_names.find(sid);
      if (pos != btor_state_vars.state_var_without_names.end()) {
        btor_state_vars.state_vars.emplace(line_split[3], pos->second);
        btor_state_vars.state_var_without_names.erase(pos);
      }
    }
  }
} // DesignToBtor::LoadDesignFromBtor

bool DesignToBtor::YosysParseDesignToBtor(
    const std::string& output_path,  // will be the fname for the generate btor
    const std::vector<std::string>& implementation_srcs,
    const std::vector<std::string>& include_dirs,
    const std::string& module_name,
    const rfmap::ClockSpecification & clock_specification,
    const rfmap::ResetSpecification & reset_specification,
    const _vtg_config & config = _vtg_config()) const {

// 1. generate Yosys Script
  auto ys_script_name_path = os_portable_append_dir(output_path,"genBtor.ys");
  auto generated_btor_name = os_portable_append_dir(output_path,"design.btor");
// 2. generate run.sh
  auto script_fname = os_portable_append_dir(output_path, "genBtor.sh");
  auto yosys_output_file = os_portable_append_dir(output_path, "__yosys_exec_result.txt");
// 3. run without run.sh

  {
    // export to ys_script_name
    std::ofstream ys_script_fout(ys_script_name_path);

    std::string write_btor_options;
    write_btor_options += config.BtorAddCommentsInOutputs ? " -v" : "";
    write_btor_options += config.BtorSingleProperty ? " -s" : "";

    std::string all_files = Join(implementation_srcs, " ");
    ys_script_fout << "read_verilog -sv "
                    << all_files // os_portable_append_dir(_output_path, top_file_name)
                    << std::endl;
    ys_script_fout << "prep -top " << module_name << std::endl;

    ILA_CHECK(
      clock_specification.custom_clock_factor.empty() &&
      clock_specification.custom_clock_sequence.empty()
      ) << "TODO: custom clock sequence not implemented yet";
    
    ILA_CHECK(
      reset_specification.custom_reset_sequence.empty() &&
      reset_specification.initial_state.empty()
      ) << "TODO: custom reset sequence not implemented yet";

    ys_script_fout << ReplaceAll(
      ReplaceAll(
      ReplaceAll(
        ReplaceAll(
            ReplaceAll(
                ReplaceAll(yosysGenerateBtor, "%rstlen%",
                            std::to_string(
                                reset_specification.reset_cycle)),
                "%cycle%",
                std::to_string(reset_specification.reset_cycle)),
            "%module%", module_name),
        "%propselect%", ""),
      "%-nomap%", config.YosysSmtArrayForRegFile ? "-nomap" : "" ),
      "",);

    // this is for pono, I don't know why it is unhappy, but we need fix this
    // in the long run
    if (config.YosysSetUndrivenZero)
      ys_script_fout << "setundef -undriven -zero\n";

    ys_script_fout << "write_btor " << write_btor_options 
                  << " " << generated_btor_name
                  << std::endl;
  } // end of step 1

  { // step 2

    std::ofstream fout(script_fname);
    if (!fout.is_open()) {
      ILA_ERROR << "Error writing to file:" << script_fname;
      return;
    }
    fout << "#!/bin/bash" << std::endl;
    fout << "echo \"* Remove prior results...\"" << std::endl;
    fout << "rm -f *.btor2 *.vcd __yosys*.txt" << std::endl;

    fout << "echo \"* Parsing input...\"" << std::endl;

    std::string yosys = "yosys";

    if (!config.YosysPath.empty())
      yosys = os_portable_append_dir(config.YosysPath, yosys);

    // execute it
    fout << yosys << " -s " << ys_script_name_path << " > " << yosys_output_file << "\n";
  } // end of step 2

  {
    auto exec_res = os_portable_execute_shell({"env","bash",script_fname});
    return exec_res.ret == 0;
  }

} // DesignToBtor::YosysParseDesignToBtor

}; // namespace ilang
