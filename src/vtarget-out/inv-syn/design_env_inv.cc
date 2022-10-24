// should derive from design_to_btor
// remember to translate assumptions to the design
//  (by removing RTL.)
//  (you can translate through SMT-LIB interface, so you
//   don't need to worry about connection)
//  (be sure to add check in Pono such that it will report
//   the problem of var not found)

#include <ilang/vtarget-out/inv-syn/design_env_inv.h>
#include <ilang/vtarget-out/inv-syn/cex_extract.h>

namespace ilang {

EnvironmentInvariantSynthesizer::EnvironmentInvariantSynthesizer(const DesignToBtor & design2btor) : design_info(design2btor) { }


void EnvironmentInvariantSynthesizer::CexToSmtProperties(const std::string & cex_vcd_fname, const std::string & property_smt2) const {
  auto is_reg = [this](const std::string & name) -> bool {
    const auto & sv = this->design_info.GetBtorInfo().state_vars;
    if ( sv.find(name) == sv.end() )
      return false;
    return true;
  };

  CexExtractor cex2smt2 (cex_vcd_fname, "RTL", is_reg , true);
  // cex2smt2.toSmt2(property_smt2);
}

rfmap::RfExpr EnvironmentInvariantSynthesizer::ParseInvariant(const std::string & smt2_fname) const {
  return NULL;
}

} // namespace ilang

