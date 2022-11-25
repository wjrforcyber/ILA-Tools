
#include <ilang/env-inv-in/pono_inv_in_wrapper.h>

#include <iostream>
#include <fstream>
#include <string>
#include <map>

using namespace std;
using namespace ilang;

int main(int argc, char **argv) {
  if (argc < 4) {
    cout << "usage : " << argv[0] << " btor-of-design inv.smt2 out.json" << endl;
    return 1;
  }

  DesignToBtor converter;
  converter.LoadDesignFromBtor(argv[1]);
  const auto & info = converter.GetBtorInfo();

  ifstream fin(argv[2]);
  ofstream fout(argv[3]);
  if (!fin.is_open()) {
    cout << argv[2] << " is not acccessible.";
    return 1;
  }
  if (!fout.is_open()) {
    cout << argv[3] << " is not acccessible.";
    return 1;
  }

  map<string, string> var_def;
  vector<string> invariants;

  string s;
  while( getline(fin, s) ) {
    smt::SmtlibInvariantParserInstance env_inv_in(info);
    env_inv_in.ParseSmtResultFromString("(assert " + s + ")");

    const auto & localvar_def = env_inv_in.GetLocalVarDefStr();
    for (const auto & var_expr : localvar_def) {
      var_def.emplace(var_expr.first, var_expr.second);
    }
    invariants.push_back(env_inv_in.GetFinalTranslateResult());
  }

  fout << "\"monitor\" :{ " << endl;
  auto l = var_def.size();
  size_t idx = 0;
  for (const auto & var_expr : var_def) {
    fout<< "  \""<< var_expr.first<<"\" : \" " << var_expr.second;
    if (idx == l - 1)
      fout << "\"\n";
    else
      fout <<"\",\n";
    idx ++;
  }
  fout << "}" << endl;


  fout << "\"global-invariants\" :{ " << endl;
  l = invariants.size();
  idx = 0;
  for (const auto & inv : invariants) {
      fout<< "  \""<< inv;
    if (idx == l - 1)
      fout << "\"\n";
    else
      fout <<"\",\n";
    idx ++;
  }
  fout << "}" << endl;

  return 0;
}
