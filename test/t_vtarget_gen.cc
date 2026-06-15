/// \file
/// Unit test for generating Verilog verification target

#include <ilang/ila/instr_lvl_abs.h>
#include <ilang/ilang++.h>
#include <ilang/util/fs.h>
#include <ilang/vtarget-out/vtarget_gen.h>

#include "unit-include/config.h"
#include "unit-include/memswap.h"
#include "unit-include/pipe_ila.h"
#include "unit-include/util.h"

namespace ilang {

typedef std::vector<std::string> P;

// #warning "Continue your test from here"

TEST(TestVlgTargetGen, Memory) {
  auto ila_model = MemorySwap::BuildModel();

  auto dirName =
      os_portable_join_dir({ILANG_TEST_SRC_ROOT, "unit-data", "vpipe", "vmem"});
  VerilogVerificationTargetGenerator vg(
      {},                                                // no include
      {os_portable_append_dir(dirName, "swap.v")},       // vlog files
      "swap",                                            // top_module_name
      os_portable_append_dir(dirName, P({"vmap.json"})), // variable mapping
      os_portable_append_dir(dirName, P({"cond.json"})),
      dirName, // output path
      ila_model.get(),
      ModelCheckerSelection::PONO);

  EXPECT_FALSE(vg.in_bad_state());

  vg.GenerateTargets();
}

TEST(TestVlgTargetGen, MemoryInternal) { // test the expansion of memory

  auto ila_model = MemorySwap::BuildSimpleSwapModel();

  RtlVerifyConfig vtg_cfg; // default configuration
  VerilogGeneratorBase::VlgGenConfig vlg_cfg;
  vlg_cfg.extMem = false;
  auto dirName =
      os_portable_join_dir({ILANG_TEST_SRC_ROOT, "unit-data", "vpipe", "vmem"});
  VerilogVerificationTargetGenerator vg(
      {},                                                  // no include
      {os_portable_append_dir(dirName, "swap_im.v")},      // vlog files
      "swap",                                              // top_module_name
      os_portable_append_dir(dirName, "vmap-expand.json"), // variable mapping
      os_portable_append_dir(dirName, "cond-expand.json"),
      dirName, // output path
      ila_model.get(),
      ModelCheckerSelection::PONO, vtg_cfg,
      vlg_cfg);

  EXPECT_FALSE(vg.in_bad_state());

  vg.GenerateTargets();
}

TEST(TestVlgTargetGen, MemoryInternalExternal) {
  auto ila_model = MemorySwap::BuildRfAsMemModel();

  auto dirName =
      os_portable_join_dir({ILANG_TEST_SRC_ROOT, "unit-data", "vpipe", "vmem"});

  VerilogVerificationTargetGenerator vg(
      {},                                                   // no include
      {os_portable_append_dir(dirName, "rf_as_mem.v")},     // vlog files
      "proc",                                               // top_module_name
      os_portable_append_dir(dirName, "vmap-rfarray.json"), // variable mapping
      os_portable_append_dir(dirName, "cond-rfarray.json"),
      dirName, // output path
      ila_model.get(),
      ModelCheckerSelection::PONO);

  EXPECT_FALSE(vg.in_bad_state());

  vg.GenerateTargets();
}

TEST(TestVlgTargetGen, MemoryInternalExternalEntry6) {
  auto ila_model = MemorySwap::BuildRfAsMemModelRegEntry6();

  // DebugLog::Enable("VTG.ReplWireEq");
  // DebugLog::Enable("VTG.ReplAssert");
  // DebugLog::Enable("VTG.ReplAssume");

  // DebugLog::Enable("VTG.AddWireEq");
  // DebugLog::Enable("VTG.AddAssert");
  // DebugLog::Enable("VTG.AddAssume");

  RtlVerifyConfig vtg_cfg;
  vtg_cfg.PonoAddKeep = false;

  auto dirName = std::string(ILANG_TEST_SRC_ROOT) + "/unit-data/vpipe/vmem/";
  VerilogVerificationTargetGenerator vg(
      {},                             // no include
      {dirName + "rf_as_mem_6rf.v"},  // vlog files
      "proc",                         // top_module_name
      dirName + "vmap-rfarray6.json", // variable mapping
      dirName + "cond-rfarray.json",  // cond path
      dirName + "rfarray_rf6/",       // output path
      ila_model.get(),
      ModelCheckerSelection::PONO, vtg_cfg);

  EXPECT_FALSE(vg.in_bad_state());

  vg.GenerateTargets();
}

TEST(TestVlgTargetGen, MemoryRead) {
  auto ila_model = MemorySwap::BuildRdModel();

  auto dirName = std::string(ILANG_TEST_SRC_ROOT) + "/unit-data/vpipe/vmem/";
  VerilogVerificationTargetGenerator vg(
      {},                       // no include
      {dirName + "read.v"},     // vlog files
      "rdtop",                  // top_module_name
      dirName + "vmap-rd.json", // variable mapping
      dirName + "cond-rd.json", // cond path
      dirName,                  // output path
      ila_model.get(),
      ModelCheckerSelection::PONO);

  EXPECT_FALSE(vg.in_bad_state());

  vg.GenerateTargets();
}

TEST(TestVlgTargetGen, MemoryReadAbsReadJasperGold) {
  RtlVerifyConfig vtg_cfg;

  auto ila_model = MemorySwap::BuildRdModel();

  auto dirName = std::string(ILANG_TEST_SRC_ROOT) + "/unit-data/vpipe/vmem/";
  VerilogVerificationTargetGenerator vg(
      {},                       // no include
      {dirName + "read.v"},     // vlog files
      "rdtop",                  // top_module_name
      dirName + "vmap-rd.json", // variable mapping
      dirName + "cond-rd.json", // cond path
      dirName + "rdabs_jg/",    // output path
      ila_model.get(),
      ModelCheckerSelection::JASPERGOLD,
      vtg_cfg);

  EXPECT_FALSE(vg.in_bad_state());

  vg.GenerateTargets();
}


TEST(TestVlgTargetGen, MemoryForallEqualPono) {
  RtlVerifyConfig vtg_cfg;

  auto ila_model = MemorySwap::BuildSimpleLargeArray();

  auto dirName = std::string(ILANG_TEST_SRC_ROOT) + "/unit-data/vpipe/vmem/";
  VerilogVerificationTargetGenerator vg(
      {},                       // no include
      {dirName + "smallarray.v"},     // vlog files
      "top",                  // top_module_name
      dirName + "vmap-forall.json", // variable mapping
      dirName + "cond-forall.json", // cond path
      dirName + "large_small_forall/",    // output path
      ila_model.get(),
      ModelCheckerSelection::PONO,
      vtg_cfg);

  EXPECT_FALSE(vg.in_bad_state());

  vg.GenerateTargets();
}


TEST(TestVlgTargetGen, MemoryForallEqualJg) {
  RtlVerifyConfig vtg_cfg;

  auto ila_model = MemorySwap::BuildSimpleLargeArray();

  auto dirName = std::string(ILANG_TEST_SRC_ROOT) + "/unit-data/vpipe/vmem/";
  VerilogVerificationTargetGenerator vg(
      {},                       // no include
      {dirName + "smallarray.v"},     // vlog files
      "top",                  // top_module_name
      dirName + "vmap-forall.json", // variable mapping
      dirName + "cond-forall.json", // cond path
      dirName + "large_small_forall_jg/",    // output path
      ila_model.get(),
      ModelCheckerSelection::JASPERGOLD,
      vtg_cfg);

  EXPECT_FALSE(vg.in_bad_state());

  vg.GenerateTargets();
}


TEST(TestVlgTargetGen, UndetValue) {
  auto ila_model = UndetVal::BuildModel();

  auto dirName = std::string(ILANG_TEST_SRC_ROOT) + "/unit-data/vpipe/undetf/";
  VerilogVerificationTargetGenerator vg(
      {},                        // no include
      {dirName + "val.v"},       // vlog files
      "undetval",                // top_module_name
      dirName + "vmap-val.json", // variable mapping
      dirName + "cond-val.json", // cond path
      dirName,                   // output path
      ila_model.get(),
      ModelCheckerSelection::PONO);

  EXPECT_FALSE(vg.in_bad_state());

  vg.GenerateTargets();
}

TEST(TestVlgTargetGen, UndetFunc) {
  auto ila_model = UndetFunc::BuildModel();

  auto dirName = std::string(ILANG_TEST_SRC_ROOT) + "/unit-data/vpipe/undetf/";
  VerilogVerificationTargetGenerator vg(
      {},                         // no include
      {dirName + "func.v"},       // vlog files
      "undetfunc",                // top_module_name
      dirName + "vmap-func.json", // variable mapping
      dirName + "cond-func.json", // cond path
      dirName,                    // output path
      ila_model.get(),
      ModelCheckerSelection::PONO);

  EXPECT_FALSE(vg.in_bad_state());

  vg.GenerateTargets();
}

TEST(TestVlgTargetGen, ResetAnnotation) {
  {
    auto ila_model = MemorySwap::BuildResetterTest();
    auto dirName = os_portable_join_dir(
        {ILANG_TEST_SRC_ROOT, "unit-data", "vpipe", "reset"});

    VerilogVerificationTargetGenerator vg(
        {}, // no include
        {os_portable_join_dir(
            {dirName, "verilog", "resetter.v"})}, // vlog files
        "resetter",                               // top_module_name
        os_portable_join_dir(
            {dirName, "rfmap", "vmap-e1.json"}), // variable mapping
        os_portable_join_dir({dirName, "rfmap", "cond.json"}), // cond path
        os_portable_append_dir(dirName, "out"),                // output path
        ila_model.get(),
        ModelCheckerSelection::PONO);

    EXPECT_FALSE(vg.in_bad_state());

    vg.GenerateTargets();
  }
  {
    auto ila_model = MemorySwap::BuildResetterTest();
    auto dirName = os_portable_join_dir(
        {ILANG_TEST_SRC_ROOT, "unit-data", "vpipe", "reset"});

    VerilogVerificationTargetGenerator vg(
        {}, // no include
        {os_portable_join_dir(
            {dirName, "verilog", "resetter.v"})}, // vlog files
        "resetter",                               // top_module_name
        os_portable_join_dir(
            {dirName, "rfmap", "vmap.json"}), // variable mapping
        os_portable_join_dir({dirName, "rfmap", "cond.json"}), // cond path
        os_portable_append_dir(dirName, "out2"),               // output path
        ila_model.get(),
        ModelCheckerSelection::PONO);

    EXPECT_FALSE(vg.in_bad_state());

    vg.GenerateTargets();
  }
}

}; // namespace ilang
