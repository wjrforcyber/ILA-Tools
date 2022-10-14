/// \file Get the environment invariants
// -- 
// remember to translate assumptions to the design
//  (by removing RTL.)
//  (you can translate through SMT-LIB interface, so you
//   don't need to worry about connection)
//  (be sure to add check in Pono such that it will report
//   the problem of var not found)


// This file will coordinate cex extraction &
// property construction & inv_extraction
// cex --(filter)--> smt_properties --> (pono) --> inv
// inv --> rfExpr
//  


#ifndef ILANG_VTARGET_DESIGN_ENV_INV_H__
#define ILANG_VTARGET_DESIGN_ENV_INV_H__

#include <ilang/vtarget-out/inv-syn/design_to_btor.h>
#include <ilang/rfmap-in/verilog_rfmap.h>

namespace ilang {

class EnvironmentInvariantSynthesizer {

public:
    EnvironmentInvariantSynthesizer(const DesignToBtor & design2btor);
    void CexToSmtProperties(const std::string & cex_vcd_fname, const std::string & property_smt2) const;
    rfmap::RfExpr ParseInvariant(const std::string & smt2_fname) const;

protected:
    const DesignToBtor & design_info;

}; // class EnvironmentInvariantSynthesizer

} // namespace ilang

#endif // ILANG_VTARGET_DESIGN_ENV_INV_H__

