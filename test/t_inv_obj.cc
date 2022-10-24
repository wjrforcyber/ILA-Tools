/// \file
/// Unit test for invariant object/cnf/cex/...

#include <ilang/ila/instr_lvl_abs.h>
#include <ilang/ilang++.h>
#include <ilang/util/container_shortcut.h>
#include <ilang/util/fs.h>
#include <ilang/vtarget-out/vtarget_gen.h>

#include "unit-include/config.h"
#include "unit-include/memswap.h"
#include "unit-include/pipe_ila.h"
#include "unit-include/util.h"

namespace ilang {

#ifdef ILANG_BUILD_INVSYN

TEST(InvSynSupportAuxClass, CexObj) {
  auto dirName = os_portable_append_dir(
      ILANG_TEST_SRC_ROOT, std::vector<std::string>({"unit-data", "cex"}));
  auto vcdfile = os_portable_append_dir(dirName, "cex.vcd");
  auto cexfile = os_portable_append_dir(dirName, "cex.txt");

  CexExtractor cex(
      vcdfile, "m1", [](const std::string&) { return true; }, true);

  cex.DropStates({"m1.m1__DOT__out[3:0]"});
  cex.StoreCexToFile(cexfile);
  {
    CexExtractor cex(cexfile);
    EXPECT_TRUE(!IN("m1.m1__DOT__out[3:0]", cex.GetCex()));
  }
  os_portable_remove_file(cexfile);
}


#endif

}; // namespace ilang
