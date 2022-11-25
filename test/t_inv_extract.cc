/// \file
/// Unit test for invariant extract

#include <ilang/ila/instr_lvl_abs.h>
#include <ilang/ilang++.h>
#include <ilang/env-inv-in/pono_inv_in_wrapper.h>
#include <ilang/util/fs.h>
#include <ilang/util/log.h>
#include <ilang/vtarget-out/vtarget_gen.h>

#include "unit-include/config.h"
#include "unit-include/memswap.h"
#include "unit-include/pipe_ila.h"
#include "unit-include/util.h"

namespace ilang {

class TestInvExtract : public ::testing::Test {
public:
  TestInvExtract() {}
  ~TestInvExtract() {}

  void SetUp() {
    // EnableDebug("InvExtract");
  }
  void TearDown() { DisableDebug("InvExtract"); }

}; // class TestInvExtract

TEST_F(TestInvExtract, PonoInv) {
  
  auto outpath = os_portable_append_dir(std::string(ILANG_TEST_SRC_ROOT),
                                        std::vector<std::string>({"unit-data", "inv_syn", "design2btor", "output"}));

  DesignToBtor converter;
  converter.LoadDesignFromBtor(os_portable_append_dir(outpath, "design.btor"));
  const auto & info = converter.GetBtorInfo();

  {
    smt::SmtlibInvariantParserInstance env_inv_in(info);
    env_inv_in.ParseSmtResultFromString("(assert (= ex_wb_rd id_ex_rd))");
    const auto & localvar_def = env_inv_in.GetLocalVarDefStr();
    for (const auto & var_def : localvar_def) {
      std::cout << var_def.first << " := " << var_def.second << std::endl;
    }
    std::cout << env_inv_in.GetFinalTranslateResult() << std::endl;
  }
  
}

}; // namespace ilang
