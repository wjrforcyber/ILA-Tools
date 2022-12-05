/// \file Pono Invariant Input Parsing (a wrapper)
// --- Hongce Zhang (hongcez@princeton.edu)

#ifndef PONO_INV_IN_WRAPPER_H__
#define PONO_INV_IN_WRAPPER_H__

#include <ilang/vtarget-out/inv-syn/design_to_btor.h>

namespace ilang {
namespace smt {

/// \brief this a base class, should not be instantiated
class SmtlibInvariantParserBase {

protected:
  // the raw result (not including the sat/unsat)
  std::string raw_string;
  /// all local variable definitions
  std::map<std::string,std::string> local_var_defs;


public:
  // -------------- CONSTRUCTOR ------------------- //
  // -------------- CONSTRUCTOR ------------------- //
  SmtlibInvariantParserBase(); // do nothing
  /// no copy constructor
  SmtlibInvariantParserBase(const SmtlibInvariantParserBase&) = delete;
  /// no assignment
  SmtlibInvariantParserBase&
  operator=(const SmtlibInvariantParserBase&) = delete;
  // -------------- DESTRUCTOR ------------------- //
  virtual ~SmtlibInvariantParserBase(); // do nothing

  // -------------- METHODS ------------------- //
  // parse from a file, we will add something there to make
  // if sat --> failed (return false)
  // if unsat --> add the (assert ...)
  virtual bool ParseInvResultFromFile(const std::string& fname) = 0;
  // parse from a string: assume we have the (assert ...) there
  virtual void ParseSmtResultFromString(const std::string& text) = 0;
  /// get the translate result
  virtual std::string GetFinalTranslateResult() const = 0;
  /// get the local variable definitions
  virtual const std::map<std::string,std::string> & GetLocalVarDefStr() const { 
    return local_var_defs; }
  

  /// return raw_string
  std::string GetRawSmtString() const;

  /// set the counter to a new val when loading
  static void set_new_local_ctr(unsigned cnt);
  /// parse the names to set the new counters
  static void parse_local_var_name_to_set_counter(const std::string & name);
  /// to check the value of this counter
  static unsigned get_local_ctr();

protected:
  /// a counter to get local variable name
  static std::string get_a_new_local_var_name();
  /// the idx to it
  static unsigned local_var_idx;

}; // class SmtlibInvariantParserBase

/// \brief this a base class, should not be instantiated
class SmtlibInvariantParserInstance {
protected:
  // the pointer that will be used to cast to internal datatype
  SmtlibInvariantParserBase* _ptr;

public:
  // -------------- CONSTRUCTOR ------------------- //
  SmtlibInvariantParserInstance(const BtorStateVars &, const std::string &);
  /// no copy constructor
  SmtlibInvariantParserInstance(const SmtlibInvariantParserInstance&) = delete;
  /// no assignment
  SmtlibInvariantParserInstance&
  operator=(const SmtlibInvariantParserInstance&) = delete;

  // -------------- DESTRUCTOR ------------------- //
  virtual ~SmtlibInvariantParserInstance();

  // -------------- METHODS ------------------- //
  // parse from a file, we will add something there to make
  // if sat --> failed (return false)
  // if unsat --> add the (assert ...)
  bool ParseInvResultFromFile(const std::string& fname);
  // parse from a string: assume we have the (assert ...) there
  void ParseSmtResultFromString(const std::string& text);
  /// get the translate result
  std::string GetFinalTranslateResult() const;
  /// get the local variable definitions
  const std::map<std::string,std::string> & GetLocalVarDefStr() const { 
    return _ptr->GetLocalVarDefStr(); }
}; // class SmtlibInvariantParserInstance

}; // namespace smt
}; // namespace ilang

#endif // PONO_INV_IN_WRAPPER_H__
