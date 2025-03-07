// genml.cc            see license.txt for copyright and terms of use
// code for genml.h
// first half based on 'emitActionCode' and friends from gramanl.cc
// second half based on 'emitConstructionCode' from parsetables.cc

#include "genml.h"       // this module
#include "gramanl.h"     // GrammarAnalysis
#include "emitcode.h"    // EmitCode
#include "parsetables.h" // ParseTables
#include "exc.h"         // XOpen
#include "strutil.h"     // replace

#include <fmt/core.h>    // fmt::format
#include <vector>        // std::vector


// NOTE: The as following code is largely copied from elsewhere,
// including comments, the comments may be in some places not
// perfectly in correspondence with the code.



// prototypes for this section; some of them accept Grammar simply
// because that's all they need; there's no problem upgrading them
// to GrammarAnalysis
void emitMLDescriptions(GrammarAnalysis const &g, EmitCode &out);
void emitMLActionCode(GrammarAnalysis const &g, rostring mliFname,
                      rostring mlFname, rostring srcFname);
void emitMLUserCode(EmitCode &out, LocString const &code, bool braces = true);
void emitMLActions(Grammar const &g, EmitCode &out, EmitCode &dcl);
void emitMLDupDelMerge(GrammarAnalysis const &g, EmitCode &out, EmitCode &dcl);
void emitMLFuncDecl(Grammar const &g, EmitCode &out, EmitCode &dcl,
                    char const *rettype, rostring params);
void emitMLDDMInlines(Grammar const &g, EmitCode &out, EmitCode &dcl,
                      Symbol const &sym);
void emitMLSwitchCode(Grammar const &g, EmitCode &out,
                      rostring signature, char const *switchVar,
                      std::list<Symbol> const &syms, int whichFunc,
                      char const *templateCode, char const *actUpon);


// ------------- first half: action emission ----------------
#if 0   // not needed
// yield the name of the inline function for this production; naming
// design motivated by desire to make debugging easier
string actionFuncName(Production const &prod)
{
  return stringc << "action" << prod.prodIndex
                 << "_" << prod.left->name;
}
#endif // 0


// emit the user's action code to a file
void emitMLActionCode(GrammarAnalysis const &g, rostring mliFname,
                      rostring mlFname, rostring srcFname)
{
  EmitCode dcl(mliFname);

  // prologue
  dcl << "(* " << mliFname << " *)\n"
      << "(* *** DO NOT EDIT BY HAND *** *)\n"
      << "(* automatically generated by elkhound, from " << srcFname << " *)\n"
      << "\n"
      ;

  // insert the stand-alone verbatim sections
  for (auto const& locstr : g.verbatim) {
    emitMLUserCode(dcl, locstr, false /*braces*/);
  }

  #if 0    // not implemented
  // insert each of the context class definitions; the last one
  // is the one whose name is 'g.actionClassName' and into which
  // the action functions are inserted as methods
  {
    int ct=0;
    FOREACH_OBJLIST(LocString, g.actionClasses, iter) {
      if (ct++ > 0) {
        // end the previous class; the following body will open
        // another one, and the brace following the action list
        // will close the last one
        dcl << "};\n";
      }

      dcl << "\n"
          << "// parser context class\n"
          << "class ";
      emitUserCode(dcl, *(iter.data()), false /*braces*/);
  }}

  // we end the context class with declarations of the action functions
  dcl << "\n"
      << "private:\n"
      << "  USER_ACTION_FUNCTIONS      // see useract.h\n"
      << "\n"
      << "  // declare the actual action function\n"
      << "  static SemanticValue doReductionAction(\n"
      << "    " << g.actionClassName << " *ths,\n"
      << "    int productionId, SemanticValue const *semanticValues"
         SOURCELOC( << ",\n  SourceLoc loc" )
      << ");\n"
      << "\n"
      << "  // declare the classifier function\n"
      << "  static int reclassifyToken(\n"
      << "    " << g.actionClassName << " *ths,\n"
      << "    int oldTokenType, SemanticValue sval);\n"
      << "\n"
      ;
  #endif // 0

  // all that goes into the interface is the name of the
  // tUserActions and tParseTables objects
  dcl << "val " << g.actionClassName << "ParseTables: Parsetables.tParseTables\n";
  dcl << "val " << g.actionClassName << "UserActions: Useract.tUserActions\n";

  EmitCode out(mlFname);

  out << "(* " << mlFname << " *)\n";
  out << "(* *** DO NOT EDIT BY HAND *** *)\n";
  out << "(* automatically generated by gramanl, from " << srcFname << " *)\n";
  out << "\n"
      << "open Useract      (* tSemanticValue *)\n"
      << "open Parsetables  (* tParseTables *)\n"
      << "\n"
      << "\n"
      ;

  // stand-alone verbatim sections go into .ml file *also*
  for (auto const& locstr : g.verbatim) {
    emitMLUserCode(out, locstr, false /*braces*/);
  }

  #if 0   // not implemented and/or not needed
    #ifdef NO_GLR_SOURCELOC
      // we need to make sure the USER_ACTION_FUNCTIONS use
      // the declarations consistent with how we're printing
      // the definitions
      out << "#ifndef NO_GLR_SOURCELOC\n";
      out << "  #define NO_GLR_SOURCELOC\n";
      out << "#endif\n";
    #else
      out << "// GLR source location information is enabled\n";
    #endif
    out << "\n";
    out << "#include \"" << hFname << "\"     // " << g.actionClassName << "\n";
    out << "#include \"parsetables.h\" // ParseTables\n";
    out << "#include \"srcloc.h\"      // SourceLoc\n";
    out << "\n";
    out << "#include <assert.h>      // assert\n";
    out << "#include <iostream>      // std::cout\n";
    out << "#include <stdlib.h>      // abort\n";
    out << "\n";

    NOSOURCELOC(
      out << "// parser-originated location information is disabled by\n"
          << "// NO_GLR_SOURCELOC; any rule which refers to 'loc' will get this one\n"
          << "static SourceLoc loc = SL_UNKNOWN;\n"
          << "\n\n";
    )
  #endif // 0

  emitMLDescriptions(g, out);
  // 'emitMLDescriptions' prints two newlines itself..

  // impl_verbatim sections
  //
  // 2005-06-23: Moved these to near the top of the file so that
  // the actions can refer to them.  This is especially important
  // in OCaml since you can't forward-declare in OCaml (!).
  for (auto& locstr : g.implVerbatim) {
    emitMLUserCode(out, locstr, false /*braces*/);
  }

  emitMLActions(g, out, dcl);
  out << "\n";
  out << "\n";

  emitMLDupDelMerge(g, out, dcl);
  out << "\n";
  out << "\n";

  // wrap all the action stuff up as a struct
  out << "let " << g.actionClassName << "UserActions = {\n";
  #define COPY(name) \
    out << "  " #name " = " #name "Func;\n";
  COPY(reductionAction)
  COPY(duplicateTerminalValue)
  COPY(duplicateNontermValue)
  COPY(deallocateTerminalValue)
  COPY(deallocateNontermValue)
  COPY(mergeAlternativeParses)
  COPY(keepNontermValue)
  COPY(terminalDescription)
  COPY(nonterminalDescription)
  COPY(terminalName)
  COPY(nonterminalName)
  #undef COPY
  out << "}\n"
      << "\n"
      << "\n"
      ;

  g.tables->finishTables();
  g.tables->emitMLConstructionCode(out, string(g.actionClassName), "makeTables");

  #if 0   // not implemented
    // I put this last in the context class, and make it public
    dcl << "\n"
        << "// the function which makes the parse tables\n"
        << "public:\n"
        << "  virtual ParseTables *makeTables();\n"
        << "};\n"
        << "\n"
        << "#endif // " << latchName << "\n"
        ;
  #endif // 0
}


void emitMLUserCode(EmitCode &out, LocString const &code, bool braces)
{
  out << "\n";
  if (false/*TODO:fix*/ && code.validLoc()) {
    out << lineDirective(code.loc);
  }

  // 7/27/03: swapped so that braces are inside the line directive
  if (braces) {
    out << "(";
  }

  out << code;

  // the final brace is on the same line so errors reported at the
  // last brace go to user code
  if (braces) {
    out << " )";
  }

  out << "\n";
  if (false/*TODO:fix*/ && code.validLoc()) {
    out.restoreLine();
  }
}


// bit of a hack: map "void" to "SemanticValue" so that the compiler
// won't mind when I try to declare parameters of that type
static char const *notVoid(char const *type)
{
  if (0==strcmp(type, "void")) {     // ML: Q: should this now be "unit"?
    return "tSemanticValue";
  }
  else {
    return type;
  }
}


// yield the given type, but if it's NULL, then yield
// something to use instead
static char const *typeString(char const *type, LocString const &tag)
{
  if (!type) {
    std::cout << tag.locString() << ": Production tag \"" << tag
         << "\" on a symbol with no type.\n";
    return "__error_no_type__";     // will make compiler complain
  }
  else {
    return notVoid(type);
  }
}


void emitMLDescriptions(GrammarAnalysis const &g, EmitCode &out)
{
  // emit a map of terminal ids to their names
  {
    out << "let termNamesArray: string array = [|\n";
    for (int code=0; code < g.numTerminals(); code++) {
      Terminal const *t = g.getTerminal(code);
      if (!t) {
        // no terminal for that code
        out << "  \"(no terminal)\";  (* " << code << " *)\n";
      }
      else {
        out << "  \"" << t->name << "\";  (* " << code << " *)\n";
      }
    }
    out << "  \"\"   (* dummy final value for ';' separation *)\n"
        << "|]\n"
        << "\n";
  }

  // emit a function to describe terminals; at some point I'd like to
  // extend my grammar format to allow the user to supply
  // token-specific description functions, but for now I will just
  // use the information easily available the synthesize one;
  // I print "sval % 100000" so I get a 5-digit number, which is
  // easy for me to compare for equality without adding much clutter
  //
  // ML: I could do something like this using Obj, but I'd rather
  // not abuse that interface unnecessarily.
  out << "let terminalDescriptionFunc (termId:int) (sval:tSemanticValue) : string =\n"
      << "begin\n"
      << "  termNamesArray.(termId)\n"
      << "end\n"
      << "\n"
      << "\n"
      ;

  // emit a map of nonterminal ids to their names
  {
    out << "let nontermNamesArray: string array = [|\n";
    for (int code=0; code < g.numNonterminals(); code++) {
      Nonterminal const *nt = g.getNonterminal(code);
      if (!nt) {
        // no nonterminal for that code
        out << "  \"(no nonterminal)\";  (* " << code << " *)\n";
      }
      else {
        out << "  \"" << nt->name << "\";  (* " << code << " *)\n";
      }
    }
    out << "  \"\"   (* dummy final value for ';' separation *)\n"
        << "|]\n"
        << "\n";
  }

  // and a function to describe nonterminals also
  out << "let nonterminalDescriptionFunc (nontermId:int) (sval:tSemanticValue)\n"
      << "  : string =\n"
      << "begin\n"
      << "  nontermNamesArray.(nontermId)\n"
      << "end\n"
      << "\n"
      << "\n"
      ;

  // emit functions to get access to the static maps
  out << "let terminalNameFunc (termId:int) : string =\n"
      << "begin\n"
      << "  termNamesArray.(termId)\n"
      << "end\n"
      << "\n"
      << "let nonterminalNameFunc (nontermId:int) : string =\n"
      << "begin\n"
      << "  nontermNamesArray.(nontermId)\n"
      << "end\n"
      << "\n"
      << "\n"
      ;
}


void emitMLActions(Grammar const &g, EmitCode &out, EmitCode &/*dcl*/)
{
  out << "(* ------------------- actions ------------------ *)\n"
      << "let reductionActionArray : (tSemanticValue array -> tSemanticValue) array = [|\n"
      << "\n"
      ;

  // iterate over productions, emitting action function closures
  for (auto const& prod : g.productions) {

    // there's no syntax for a typeless nonterminal, so this shouldn't
    // be triggerable by the user
    xassert(prod.left->type);

    // put the production in comments above the defn
    out << "(* " << prod.toString() << " *)\n";

    out << "(fun svals ->\n";

    // iterate over RHS elements, emitting bindings for each with a tag
    int index=-1;
    for (Production::RHSElt const& elt : prod.right) {
      index++;
      if (elt.tag.length() == 0) continue;

      // example:
      //   let e1 = (Obj.obj svals.(0) : int) in
      out << "  let " << elt.tag << " = (Obj.obj svals.(" << index << ") : "
          << typeString(elt.sym->type, elt.tag) << ") in\n";
    }

    // give a name to the yielded value so we can ensure it conforms to
    // the declared type
    out << "  let __result: " << prod.left->type << " =";

    // now insert the user's code, to execute in this environment of
    // properly-typed semantic values
    emitMLUserCode(out, prod.action, true /*braces*/);

    out << "  in (Obj.repr __result)\n"     // cast to tSemanticValue
        << ");\n"
        << "\n"
        ;
  }

  // finish the array; one dummy element for ';' separation
  out << "(fun _ -> (failwith \"bad production index\"))   (* no ; *)"
      << "\n"
      << "|]\n"
      << "\n"
      ;

  // main action function; uses the array emitted above
  out << "let reductionActionFunc (productionId:int) (svals: tSemanticValue array)\n"
      << "  : tSemanticValue =\n"
      << "begin\n"
      << "  (reductionActionArray.(productionId) svals)\n"
      << "end\n"
      << "\n"
      ;


  #if 0  // shouldn't be needed
  if (0==strcmp(prod.left->type, "void")) {
    // cute hack: turn the expression into a comma expression, with
    // the value returned being 0
    out << ", 0";
  }
  #endif // 0
}


void emitMLDupDelMerge(GrammarAnalysis const &g, EmitCode &out, EmitCode &dcl)
{
  out << "(* ---------------- dup/del/merge/keep nonterminals --------------- *)\n"
      << "\n";

  // emit inlines for dup/del/merge of nonterminals
  for (const auto& nt : g.nonterminals) {
    emitMLDDMInlines(g, out, dcl, nt);
  }

  // emit dup-nonterm
  emitMLSwitchCode(g, out,
    "let duplicateNontermValueFunc (nontermId:int) (sval:tSemanticValue) : tSemanticValue",
    "nontermId",
    reinterpret_cast<std::list<Symbol> const &>(g.nonterminals), /*FIXME this is a bad hack*/
    0 /*dupCode*/,
    "      (Obj.repr (dup_$symName ((Obj.obj sval) : $symType)))\n",
    NULL);

  // emit del-nonterm
  emitMLSwitchCode(g, out,
    "let deallocateNontermValueFunc (nontermId:int) (sval:tSemanticValue) : unit",
    "nontermId",
    reinterpret_cast<std::list<Symbol> const&>(g.nonterminals), /*FIXME this is a bad hack*/
    1 /*delCode*/,
    "      (del_$symName ((Obj.obj sval) : $symType));\n",
    "deallocate nonterm");

  // emit merge-nonterm
  emitMLSwitchCode(g, out,
    "let mergeAlternativeParsesFunc (nontermId:int) (left:tSemanticValue)\n"
    "                               (right:tSemanticValue) : tSemanticValue",
    // SOURCELOC?
    "nontermId",
    reinterpret_cast<std::list<Symbol> const&>(g.nonterminals), /*FIXME this is a bad hack*/
    2 /*mergeCode*/,
    "      (Obj.repr (merge_$symName ((Obj.obj left) : $symType) ((Obj.obj right) : $symType)))\n",
    "merge nonterm");

  // emit keep-nonterm
  emitMLSwitchCode(g, out,
    "let keepNontermValueFunc (nontermId:int) (sval:tSemanticValue) : bool",
    "nontermId",
    reinterpret_cast<std::list<Symbol> const &>(g.nonterminals), /*FIXME this is a bad hack*/
    3 /*keepCode*/,
    "      (keep_$symName ((Obj.obj sval) : $symType))\n",
    NULL);


  out << "\n";
  out << "(* ---------------- dup/del/classify terminals --------------- *)";
  // emit inlines for dup/del of terminals
  for (auto const &term : g.terminals) {
    emitMLDDMInlines(g, out, dcl, term);
  }

  // emit dup-term
  emitMLSwitchCode(g, out,
    "let duplicateTerminalValueFunc (termId:int) (sval:tSemanticValue) : tSemanticValue",
    "termId",
    reinterpret_cast<std::list<Symbol> const&>(g.terminals), /*FIXME this is a bad hack*/
    0 /*dupCode*/,
    "      (Obj.repr (dup_$symName ((Obj.obj sval) : $symType)))\n",
    NULL);

  // emit del-term
  emitMLSwitchCode(g, out,
    "let deallocateTerminalValueFunc (termId:int) (sval:tSemanticValue) : unit",
    "termId",
    reinterpret_cast<std::list<Symbol> const&>(g.terminals), /*FIXME this is a bad hack*/
    1 /*delCode*/,
    "      (del_$symName ((Obj.obj sval) : $symType));\n",
    "deallocate terminal");

  // emit classify-term
  emitMLSwitchCode(g, out,
    "let reclassifyTokenFunc (oldTokenType:int) (sval:tSemanticValue) : int",
    "oldTokenType",
    reinterpret_cast<std::list<Symbol> const&>(g.terminals), /*FIXME this is a bad hack*/
    4 /*classifyCode*/,
    "      (classify_$symName ((Obj.obj sval) : $symType))\n",
    NULL);
}


// emit both the function decl for the .h file, and the beginning of
// the function definition for the .cc file
void emitMLFuncDecl(Grammar const &, EmitCode &out, EmitCode &/*dcl*/,
                    char const *rettype, rostring params)
{
  out << "(*inline*) let " << params << ": " << rettype << " =";
}


void emitMLDDMInlines(Grammar const &g, EmitCode &out, EmitCode &dcl,
                      Symbol const &sym)
{
  Terminal const *term = sym.ifTerminalC();
  Nonterminal const *nonterm = sym.ifNonterminalC();

  if (sym.dupCode) {
    emitMLFuncDecl(g, out, dcl, sym.type,
      fmt::format("dup_{} ({}: {}) ",
        sym.name.str, sym.dupParam, sym.type));
    emitMLUserCode(out, sym.dupCode);
    out << "\n";
  }

  if (sym.delCode) {
    emitMLFuncDecl(g, out, dcl, "unit",
      fmt::format("del_{} ({}: {}) ",
        sym.name.str, (sym.delParam ? sym.delParam : "_"),
        sym.type));
    emitMLUserCode(out, sym.delCode);
    out << "\n";
  }

  if (nonterm && nonterm->mergeCode) {
    emitMLFuncDecl(g, out, dcl, notVoid(sym.type),
      fmt::format("merge_{} ({}: {})  ({}: {}) ",
        sym.name.str,
        nonterm->mergeParam1, notVoid(sym.type),
        nonterm->mergeParam2, notVoid(sym.type)));
    emitMLUserCode(out, nonterm->mergeCode);
    out << "\n";
  }

  if (nonterm && nonterm->keepCode) {
    emitMLFuncDecl(g, out, dcl, "bool",
      fmt::format("keep_{} ({}: {}) ",
        sym.name.str, nonterm->keepParam, sym.type));
    emitMLUserCode(out, nonterm->keepCode);
    out << "\n";
  }

  if (term && term->classifyCode) {
    emitMLFuncDecl(g, out, dcl, "int",
      fmt::format("classify_{} ({}: {}) ",
        sym.name.str, term->classifyParam, sym.type));
    emitMLUserCode(out, term->classifyCode);
    out << "\n";
  }
}

void emitMLSwitchCode(Grammar const &g, EmitCode &out,
                      rostring signature, char const *switchVar,
                      std::list<Symbol> const &syms, int whichFunc,
                      char const *templateCode, char const */*actUpon*/)
{
  out << replace(signature, "$acn", string(g.actionClassName)) << " =\n"
         "begin\n"
         "  match " << switchVar << " with\n"
         ;

  for (const auto& sym : syms) {

    if ((whichFunc==0 && sym.dupCode) ||
        (whichFunc==1 && sym.delCode) ||
        (whichFunc==2 && sym.asNonterminalC().mergeCode) ||
        (whichFunc==3 && sym.asNonterminalC().keepCode) ||
        (whichFunc==4 && sym.asTerminalC().classifyCode)) {
      out << "  | " << sym.getTermOrNontermIndex() << " -> (\n";
      out << replace(replace(templateCode,
               "$symName", string(sym.name)),
               "$symType", notVoid(sym.type));
      out << "    )\n";
    }
  }

  out << "  | _ -> (\n";
  switch (whichFunc) {
    default:
      xfailure("bad func code");

    // in ML it's not such a good idea to yield cNULL_SVAL, since the
    // runtime engine might get more confused than a C program
    // with a NULL pointer.. so always do the gc-defaults thing

    case 0:    // unspecified dup
      out << "      sval\n";
      break;

    case 1:    // unspecified del
      // ignore del
      out << "      ()\n";
      break;

    case 2:    // unspecified merge: warn, but then use left (arbitrarily)
      out << "      (Printf.printf \"WARNING: no action to merge nonterm %s\\n\"\n"
          << "                     nontermNamesArray.(" << switchVar << "));\n"
          << "      (flush stdout);\n"
          << "      left\n"
          ;
      break;

    case 3:    // unspecified keep: keep it
      out << "      true\n";
      break;

    case 4:    // unspecified classifier: identity map
      out << "      oldTokenType\n";
      break;
  }

  out << "    )\n"
         "end\n"
         "\n";
}


// ----------------- second half: table emission ------------------
// create literal tables
template <class EltType>
void emitMLTable(EmitCode &out, EltType const *table, int size, int rowLength,
                 char const *tableName)
{
  if (!table || !size) {
    out << "  " << tableName << " = [| |];      (* 0 elements *)\n"
        << "\n"
        ;
    return;
  }

  bool printHex = false;
  #if 0   // not needed?
                  0==strcmp(typeName, "ErrorBitsEntry") ||
                  (ENABLE_CRS_COMPRESSION && 0==strcmp(typeName, "ActionEntry")) ||
                  (ENABLE_CRS_COMPRESSION && 0==strcmp(typeName, "GotoEntry")) ;
  bool needCast = 0==strcmp(typeName, "StateId");
  #endif // 0

  if (size * sizeof(*table) > 50) {    // suppress small ones
    //out << "  // storage size: " << size * sizeof(*table) << " bytes\n";
    if (size % rowLength == 0) {
      out << "  (* rows: " << (size/rowLength) << "  cols: " << rowLength << " *)\n";
    }
  }

  int rowNumWidth = fmt::formatted_size("{}", size / rowLength /*round down*/);

  out << "  " << tableName << " = [|           (* " << size << " elements *)";
  int row = 0;
  for (int i=0; i<size; i++) {
    if (i % rowLength == 0) {    // one row per state
      out << fmt::format("\n    (*{:{}}*) ", row++, rowNumWidth);
    }

    #if 0
    if (needCast) {
      out << "(" << typeName << ")";             // ML: not used
    }
    #endif // 0

    if (printHex) {
      out << fmt::format("0x{:02X}", table[i]);  // ML: not used
    }
    else if (sizeof(table[i]) == 1) {
      // little bit of a hack to make sure 'unsigned char' gets
      // printed as an int; the casts are necessary because this
      // code gets compiled even when EltType is ProdInfo
      out << (int)(*((unsigned char*)(table+i)));
    }
    else {
      // print the other int-sized things, or ProdInfo using
      // the overloaded '<<' below
      out << table[i];
    }

    if (i != size-1) {
      out << "; ";
    }
  }
  out << "\n"
      << "  |];\n"
      << "\n"
      ;
}

template <class EltType>
void emitMLTable(EmitCode& out, const std::vector<EltType> &table, int rowLength,
  char const* tableName)
{
  emitMLTable(out, &table[0], table.size(), rowLength, tableName);
}

#if 0   // not used
// used to emit the elements of the prodInfo table
stringBuilder& operator<< (stringBuilder &sb, ParseTables::ProdInfo const &info)
{
  sb << "{" << (int)info.rhsLen << "," << (int)info.lhsIndex << "}";
  return sb;
}


// like 'emitTable', but also set a local called 'tableName'
template <class EltType>
void emitMLTable2(EmitCode &out, EltType const *table, int size, int rowLength,
                  char const *typeName, char const *tableName)
{
  string tempName = stringc << tableName << "_static";
  emitMLTable(out, table, size, rowLength, typeName, tempName);
  out << "  " << tableName << " = const_cast<" << typeName << "*>("
      << tempName << ");\n\n";
}


template <class EltType>
void emitMLOffsetTable(EmitCode &out, EltType **table, EltType *base, int size,
                       char const *typeName, char const *tableName, char const *baseName)
{
  if (!table) {
    out << "  " << tableName << " = NULL;\n\n";
    return;
  }

  // make the pointers persist by storing a table of offsets
  std::vector<int> offsets(size, UNASSIGNED);
  bool allUnassigned = true;
  for (int i=0; i < size; i++) {
    if (table[i]) {
      offsets[i] = table[i] - base;
      allUnassigned = false;
    }
  }

  if (allUnassigned) {
    // for example, an LALR(1) grammar has no ambiguous entries in its tables
    size = 0;
  }

  if (size > 0) {
    out << "  " << tableName << " = new " << typeName << " [" << size << "];\n";

    emitTable(out, (int*)offsets, size, 16, "int", stringc << tableName << "_offsets");

    // at run time, interpret the offsets table
    out << "  for (int i=0; i < " << size << "; i++) {\n"
        << "    int ofs = " << tableName << "_offsets[i];\n"
        << "    if (ofs >= 0) {\n"
        << "      " << tableName << "[i] = " << baseName << " + ofs;\n"
        << "    }\n"
        << "    else {\n"
        << "      " << tableName << "[i] = NULL;\n"
        << "    }\n"
        << "  }\n\n";
  }
  else {
    out << "  // offset table is empty\n"
        << "  " << tableName << " = NULL;\n\n";
  }
}


// for debugging
template <class EltType>
void printMLTable(EltType const *table, int size, int rowLength,
                  char const *typeName, char const *tableName)
{
  // disabled for now since I don't need it anymore, and it adds
  // a link dependency on emitcode.cc ...
  #if 0
  {
    EmitCode out("printTable.tmp");
    emitTable(out, table, size, rowLength, typeName, tableName);
  }

  system("cat printTable.tmp; rm printTable.tmp");
  #endif // 0
}
#endif // 0


// emit code for a function which, when compiled and executed, will
// construct this same table (except the constructed table won't own
// the table data, since it will point to static program data)
void ParseTables::emitMLConstructionCode
  (EmitCode &out, rostring className, rostring /*funcName*/)
{
  // must have already called 'finishTables'
  xassert(!temp);

  out << "(* a literal tParseTables;\n"
      << " * the code is written by ParseTables::emitConstructionCode()\n"
      << " * in " << __FILE__ << " *)\n"
      << "let " << className << "ParseTables:tParseTables = {\n";
      ;

  #define SET_VAR(var) \
    out << "  " #var " = " << var << ";\n";

  SET_VAR(numTerms);
  SET_VAR(numNonterms);
  SET_VAR(numProds);
  out << "\n";

  SET_VAR(numStates);
  out << "\n";

  SET_VAR(actionCols);
  emitMLTable(out, actionTable, actionTableSize(),
              actionCols, "actionTable");

  SET_VAR(gotoCols);
  emitMLTable(out, gotoTable, gotoTableSize(),
              gotoCols, "gotoTable");

  // break the prodInfo into two arrays
  {
    std::vector<int> rhsLen(numProds, 0);
    std::vector<int> lhsIndex(numProds, 0);

    for (int i=0; i < numProds; i++) {
      rhsLen[i] = prodInfo[i].rhsLen;
      lhsIndex[i] = prodInfo[i].lhsIndex;
    }

    emitMLTable(out, rhsLen,
                16 /*columns; arbitrary*/, "prodInfo_rhsLen");
    emitMLTable(out, lhsIndex,
                16 /*columns; arbitrary*/, "prodInfo_lhsIndex");
  }

  emitMLTable(out, stateSymbol, numStates,
              16, "stateSymbol");

  SET_VAR(ambigTableSize);
  emitMLTable(out, ambigTable, ambigTableSize,
              16, "ambigTable");

  emitMLTable(out, nontermOrder, nontermOrderSize(),
              16, "nontermOrder");

  SET_VAR(startState);

  // no semicolon for last one
  out << "  finalProductionIndex = " << finalProductionIndex << "\n";

  out << "}\n"
      << "\n"
      ;
}


// EOF
