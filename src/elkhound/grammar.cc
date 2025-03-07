// grammar.cc            see license.txt for copyright and terms of use
// code for grammar.h

#include "grammar.h"   // this module
#include "syserr.h"    // xsyserror
#include "strtokp.h"   // StrtokParse
#include "trace.h"     // trace
#include "exc.h"       // xBase
#include "strutil.h"   // quoted, parseQuotedString
#include "flatten.h"   // Flatten
#include "flatutil.h"  // various xfer helpers

#include <algorithm>   // std::max
#include <stdarg.h>    // variable-args stuff
#include <stdio.h>     // FILE, etc.
#include <ctype.h>     // isupper
#include <stdlib.h>    // atoi


// print a variable value
#define PVAL(var) os << " " << #var "=" << var;


StringTable grammarStringTable;


// ---------------------- Symbol --------------------
Symbol::Symbol(LocString const &n, bool t, bool e)
  : name(n),
    isTerm(t),
    isEmptyString(e),
    type(NULL),
    dupParam(NULL),
    dupCode(),
    delParam(NULL),
    delCode(),
    reachable(false)
{}

Symbol::~Symbol()
{}


Symbol::Symbol(Flatten &flat)
  : name(flat),
    isTerm(false),
    isEmptyString(false),
    type(NULL),
    dupParam(NULL),
    delParam(NULL)
{}

void Symbol::xfer(Flatten &flat)
{
  // have to break constness to unflatten
  const_cast<LocString&>(name).xfer(flat);
  flat.xferBool(const_cast<bool&>(isTerm));
  flat.xferBool(const_cast<bool&>(isEmptyString));

  flattenStrTable->xfer(flat, type);

  flattenStrTable->xfer(flat, dupParam);
  dupCode.xfer(flat);

  flattenStrTable->xfer(flat, delParam);
  delCode.xfer(flat);

  flat.xferBool(reachable);
}


int Symbol::getTermOrNontermIndex() const
{
  if (isTerminal()) {
    return asTerminalC().termIndex;
  }
  else {
    return asNonterminalC().ntIndex;
  }
}


void Symbol::print(std::ostream &os) const
{
  os << name;
  if (type) {
    os << "[" << type << "]";
  }
  os << ":";
  PVAL(isTerm);
}


void Symbol::printDDM(std::ostream &os) const
{
  // don't print anything if no handlers
  if (!anyDDM()) return;

  // print with roughly the same syntax as input
  os << "  " << (isTerminal()? "token" : "nonterm");
  if (type) {
    os << "[" << type << "]";
  }
  os << " " << name << " {\n";

  internalPrintDDM(os);

  os << "  }\n";
}


void Symbol::internalPrintDDM(std::ostream &os) const
{
  if (dupCode.isNonNull()) {
    os << "    dup(" << dupParam << ") [" << dupCode << "]\n";
  }

  if (delCode.isNonNull()) {
    os << "    del(" << (delParam? delParam : "") << ") [" << delCode << "]\n";
  }
}


bool Symbol::anyDDM() const
{
  return dupCode.isNonNull() ||
         delCode.isNonNull();
}


Terminal const &Symbol::asTerminalC() const
{
  xassert(isTerminal());
  return (Terminal const &)(*this);
}

Nonterminal const &Symbol::asNonterminalC() const
{
  xassert(isNonterminal());
  return (Nonterminal const &)(*this);
}


Terminal const *Symbol::ifTerminalC() const
{
  return isTerminal()? (Terminal const *)this : NULL;
}

Nonterminal const *Symbol::ifNonterminalC() const
{
  return isNonterminal()? (Nonterminal const *)this : NULL;
}



// -------------------- Terminal ------------------------
Terminal::Terminal(Flatten &flat)
  : Symbol(flat),
    alias(flat),
    classifyParam(NULL)
{}

void Terminal::xfer(Flatten &flat)
{
  Symbol::xfer(flat);

  alias.xfer(flat);

  flat.xferInt(precedence);
  flat.xferInt((int&)associativity);

  flat.xferInt(termIndex);

  flattenStrTable->xfer(flat, classifyParam);
  classifyCode.xfer(flat);
}


void Terminal::print(std::ostream &os) const
{
  os << "[" << termIndex << "]";
  if (precedence) {
    os << "(" << ::toString(associativity) << " " << precedence << ")";
  }
  os << " ";
  Symbol::print(os);
}


void Terminal::internalPrintDDM(std::ostream &os) const
{
  Symbol::internalPrintDDM(os);

  if (classifyCode.isNonNull()) {
    os << "    classify(" << classifyParam << ") [" << classifyCode << "]\n";
  }
}


bool Terminal::anyDDM() const
{
  return Symbol::anyDDM() ||
         classifyCode.isNonNull();
}


string Terminal::toString(bool quoteAliases) const
{
  if (alias.length() > 0) {
    if (quoteAliases) {
      return fmt::format("\"{}\"", ::toString(alias));
    }
    else {
      return ::toString(alias);
    }
  }
  else {
    return ::toString(name);
  }
}


// ----------------- Nonterminal ------------------------
Nonterminal::Nonterminal(LocString const &name, bool isEmpty)
  : Symbol(name, false /*terminal*/, isEmpty),
    mergeParam1(NULL),
    mergeParam2(NULL),
    mergeCode(),
    keepParam(NULL),
    keepCode(),
    maximal(false),
    subsets(),
    ntIndex(-1),
    cyclic(false),
    first(0),
    follow(0),
    superset(NULL)
{}

Nonterminal::~Nonterminal()
{}


Nonterminal::Nonterminal(Flatten &flat)
  : Symbol(flat),
    mergeParam1(NULL),
    mergeParam2(NULL),
    keepParam(NULL),
    first(flat),
    follow(flat),
    superset(NULL)
{}

void Nonterminal::xfer(Flatten &flat)
{
  Symbol::xfer(flat);

  flattenStrTable->xfer(flat, mergeParam1);
  flattenStrTable->xfer(flat, mergeParam2);
  mergeCode.xfer(flat);

  flattenStrTable->xfer(flat, keepParam);
  keepCode.xfer(flat);
}

void Nonterminal::xferSerfs(Flatten &flat, Grammar &)
{
  // annotation
  flat.xferInt(ntIndex);
  flat.xferBool(cyclic);
  first.xfer(flat);
  follow.xfer(flat);
}


void Nonterminal::print(std::ostream &os, Grammar const *grammar) const
{
  os << "[" << ntIndex << "] ";
  Symbol::print(os);

  // cyclic?
  if (cyclic) {
    os << " (cyclic!)";
  }

  if (grammar) {
    // first
    os << " first={";
    first.print(os, *grammar);
    os << "}";

    // follow
    os << " follow={";
    follow.print(os, *grammar);
    os << "}";
  }
}


void Nonterminal::internalPrintDDM(std::ostream &os) const
{
  Symbol::internalPrintDDM(os);

  if (mergeCode.isNonNull()) {
    os << "    merge(" << mergeParam1 << ", " << mergeParam2
       << ") [" << mergeCode << "]\n";
  }

  if (keepCode.isNonNull()) {
    os << "    keep(" << keepParam << ") [" << keepCode << "]\n";
  }
}


bool Nonterminal::anyDDM() const
{
  return Symbol::anyDDM() ||
         mergeCode.isNonNull() ||
         keepCode.isNonNull();
}


// -------------------- TerminalSet ------------------------
STATICDEF Terminal const *TerminalSet::suppressExcept = NULL;

TerminalSet::TerminalSet(int numTerms)
{
  reset(numTerms);
}

void TerminalSet::reset(int numTerms)
{
  // numTerms can be zero
  // allocate enough space for one bit per terminal; I assume
  // 8 bits per byte
  int bitmapLen = (numTerms + 7) / 8;
  bitmap.resize(0);
  bitmap.resize(bitmapLen);
}


TerminalSet::TerminalSet(Flatten&)
{}

void TerminalSet::xfer(Flatten &flat)
{
  int bitmapLen = bitmap.size();
  flat.xferInt(bitmapLen);

  if (!bitmap.empty()) {
    if (flat.reading()) {
      bitmap.resize(bitmapLen);
    }
    flat.xferSimple(bitmap.data(), bitmapLen);
  }
}

static inline constexpr int getBit(int terminalId)
{
  return ((unsigned)terminalId % 8);
}

unsigned char *TerminalSet::getByte(int id) const
{
  int offset = (unsigned)id / 8;
  xassert(offset < bitmap.size());

  return const_cast<unsigned char*>(&bitmap[offset]);
}


bool TerminalSet::contains(int id) const
{
  unsigned char *p = getByte(id);
  return ((*p >> getBit(id)) & 1) == 1;
}


bool TerminalSet::isEqual(TerminalSet const &obj) const
{
  xassert(obj.bitmap.size() == bitmap.size());
  return 0==memcmp(bitmap.data(), obj.bitmap.data(), bitmap.size());
}


void TerminalSet::add(int id)
{
  unsigned char *p = getByte(id);
  *p |= (unsigned char)(1 << getBit(id));
}


void TerminalSet::remove(int id)
{
  unsigned char *p = getByte(id);
  *p &= (unsigned char)(~(1 << getBit(id)));
}


void TerminalSet::clear()
{
  memset(bitmap.data(), 0, bitmap.size());
}


void TerminalSet::copy(TerminalSet const &obj)
{
  xassert(obj.bitmap.size() == bitmap.size());
  memcpy(bitmap.data(), obj.bitmap.data(), bitmap.size());
}


bool TerminalSet::merge(TerminalSet const &obj)
{
  bool changed = false;
  for (int i=0; i<bitmap.size(); i++) {
    unsigned before = bitmap[i];
    unsigned after = before | obj.bitmap[i];
    if (after != before) {
      changed = true;
      bitmap[i] = after;
    }
  }
  return changed;
}


bool TerminalSet::removeSet(TerminalSet const &obj)
{
  xassertdb(obj.bitmap.size() == bitmap.size());
  bool changed = false;
  for (int i=0; i<bitmap.size(); i++) {
    unsigned before = bitmap[i];
    unsigned after = before & ~(obj.bitmap[i]);
    if (after != before) {
      changed = true;
      bitmap[i] = after;
    }
  }
  return changed;
}


void TerminalSet::print(std::ostream &os, Grammar const &g, char const *lead) const
{
  int ct=0;
  for (const auto& t : g.terminals) {
    if (!contains(t.termIndex)) continue;

    if (suppressExcept &&                  // suppressing..
        suppressExcept != &t) continue;    // and this isn't the exception

    if (ct++ == 0) {
      // by waiting until now to print this, if the set has no symbols
      // (e.g. we're in SLR(1) mode), then the comma won't be printed
      // either
      os << lead;
    }
    else {
      os << "/";
    }

    os << t.toString();
  }
}


// -------------------- Production::RHSElt -------------------------
Production::RHSElt::~RHSElt()
{}


Production::RHSElt::RHSElt(Flatten &flat)
  : sym(NULL),
    tag(flat)
{}

void Production::RHSElt::xfer(Flatten &flat)
{
  tag.xfer(flat);
}

void Production::RHSElt::xferSerfs(Flatten &flat, Grammar &)
{
  xferSerfPtr(flat, sym);
}



// -------------------- Production -------------------------
Production::Production(Nonterminal *L, char const *Ltag)
  : left(L),
    right(),
    precedence(0),
    rhsLen(-1),
    prodIndex(-1),
    firstSet(0)       // don't allocate bitmap yet
{
  (void)Ltag;
}

Production::~Production()
{}


Production::Production(Flatten &flat)
  : left(NULL),
    action(flat),
    firstSet(flat)
{}

void Production::xfer(Flatten &flat)
{
  /*FIXME*/ //xferObjList(flat, right);
  action.xfer(flat);
  flat.xferInt(precedence);
  forbid.xfer(flat);

  flat.xferInt(rhsLen);
  flat.xferInt(prodIndex);
  firstSet.xfer(flat);
}

void Production::xferSerfs(Flatten &flat, Grammar &g)
{
  // must break constness in xfer

  /*FIXME*/
  //xferSerfPtrToList(flat, const_cast<Nonterminal*&>(left),
  //                        g.nonterminals);

  // xfer right's 'sym' pointers
  for (auto& elt : right) {
    elt.xferSerfs(flat, g);
  }

  // compute derived data
  if (flat.reading()) {
    computeDerived();
  }
}


#if 0   // optimized away, using 'rhsLen' instead
int Production::rhsLength() const
{
  if (!right.isEmpty()) {
    // I used to have code here which handled this situation by returning 0;
    // since it should now never happen, I'll check that here instead
    xassert(!right.nthC(0)->sym->isEmptyString);
  }

  return right.count();
}
#endif // 0


#if 0    // useful for verifying 'finish' is called before rhsLen
int Production::rhsLength() const
{
  xassert(rhsLen != -1);     // otherwise 'finish' wasn't called
  return rhsLen;
}
#endif // 0


int Production::numRHSNonterminals() const
{
  int ct = 0;
  for (auto const& elt : right) {
    if (elt.sym->isNonterminal()) {
      ct++;
    }
  }
  return ct;
}


bool Production::rhsHasSymbol(Symbol const *sym) const
{
  for (auto const& elt : right) {
    if (elt.sym == sym) {
      return true;
    }
  }
  return false;
}


void Production::getRHSSymbols(SymbolList &output) const
{
  for (auto const& elt : right) {
    output.push_back(elt.sym);
  }
}


void Production::append(Symbol *sym, LocString const &tag)
{
  // my new design decision (6/26/00 14:24) is to disallow the
  // emptyString nonterminal from explicitly appearing in the
  // productions
  xassert(!sym->isEmptyString);

  right.emplace_back(sym, tag);
}


void Production::finished(int numTerms)
{
  computeDerived();
  firstSet.reset(numTerms);
}

void Production::computeDerived()
{
  rhsLen = right.size();
}


// basically strcmp but without the segfaults when s1 or s2
// is null; return true if strings are equal
// update: now that they're StringRef, simple "==" suffices
bool tagCompare(StringRef s1, StringRef s2)
{
  return s1 == s2;
}


int Production::findTag(StringRef tag) const
{
  // walk RHS list looking for a match
  int index=1;
  for (auto const& elt : right) {
    if (tagCompare(elt.tag, tag)) {
      return index;
    }
    index++;
  }

  // not found
  return -1;
}


// assemble a possibly tagged name for printing
string taggedName(rostring name, char const *tag)
{
  if (tag == NULL || tag[0] == 0) {
    return name;
  }
  else {
    return fmt::format("{}:{}", tag, name);
  }
}


string Production::symbolTag(int index) const
{
  // no longer have tags for LHS
  xassert(index != 0);

  // find index in RHS list
  index--;
  return string(right[index].tag);
}


Symbol const *Production::symbolByIndexC(int index) const
{
  // check LHS
  if (index == 0) {
    return left;
  }

  // find index in RHS list
  index--;
  return right[index].sym;
}


#if 0
DottedProduction const *Production::getDProdC(int dotPlace) const
{
  xassert(0 <= dotPlace && dotPlace < numDotPlaces);
  return &dprods[dotPlace];
}
#endif // 0


// it's somewhat unfortunate that I have to be told the
// total number of terminals, but oh well
void Production::addForbid(Terminal *t, int numTerminals)
{
  if (forbid.empty()) {
    forbid.reset(numTerminals);
  }

  forbid.add(t->termIndex);
}


void Production::print(std::ostream &os) const
{
  os << toString();
}


string Production::toString(bool printType, bool printIndex) const
{
  // LHS "->" RHS
  stringBuilder sb;
  if (printIndex) {
    sb << "[" << prodIndex << "] ";
  }

  sb << left->name;
  if (printType && left->type) {
    sb << "[" << left->type << "]";
  }
  sb << " -> " << rhsString();

  if (printType && precedence) {
    // take this as licence to print prec too
    sb << " %prec(" << precedence << ")";
  }
  return sb;
}


string Production::rhsString(bool printTags, bool quoteAliases) const
{
  stringBuilder sb;

  if (!right.empty()) {
    // print the RHS symbols
    int ct=0;
    for (auto const& elt : right) {
      if (ct++ > 0) {
        sb << " ";
      }

      string symName;
      if (elt.sym->isNonterminal()) {
        symName = elt.sym->name;
      }
      else {
        // print terminals as aliases if possible
        symName = elt.sym->asTerminalC().toString(quoteAliases);
      }

      if (printTags) {
        // print tag if present
        sb << taggedName(symName, elt.tag);
      }
      else {
        sb << symName;
      }
    }
  }

  else {
    // empty RHS
    sb << "empty";
  }

  return sb;
}


string Production::toStringMore(bool printCode) const
{
  stringBuilder sb;
  sb << toString();

  if (printCode && !action.isNull()) {
    sb << "\t\t[" << action.strref() << "]";
  }

  sb << "\n";

  return sb;
}


// ------------------ Grammar -----------------
Grammar::Grammar()
  : startSymbol(NULL),
    emptyString(LocString(HERE_SOURCELOC, "empty"),
                true /*isEmptyString*/),
    targetLang("C++"),
    useGCDefaults(false),
    defaultMergeAborts(false),
    expectedSR(-1),
    expectedRR(-1),
    expectedUNRNonterms(-1),
    expectedUNRTerms(-1)
{}


Grammar::~Grammar()
{}


void Grammar::xfer(Flatten &flat)
{
  // owners
  flat.checkpoint(0xC7AB4D86);
  /*FIXME*/ //xferObjList(flat, nonterminals);
  /*FIXME*/ //xferObjList(flat, terminals);
  /*FIXME*/ //xferObjList(flat, productions);

  // emptyString is const

  /*FIXME*/ //xferObjList(flat, verbatim);

  actionClassName.xfer(flat);
  /*FIXME*/ //xferObjList(flat, actionClasses);

  /*FIXME*/ //xferObjList(flat, implVerbatim);

  flat.xferString(targetLang);
  flat.xferBool(useGCDefaults);
  flat.xferBool(defaultMergeAborts);

  flat.xferInt(expectedSR);
  flat.xferInt(expectedRR);
  flat.xferInt(expectedUNRNonterms);
  flat.xferInt(expectedUNRTerms);

  // serfs
  flat.checkpoint(0x8580AAD2);

  for (auto& nt : nonterminals) {
    nt.xferSerfs(flat, *this);
  }
  for (auto& p : productions) {
    p.xferSerfs(flat, *this);
  }

  /*FIXME*/ //xferSerfPtrToList(flat, startSymbol, nonterminals);

  flat.checkpoint(0x2874DB95);
}


int Grammar::numTerminals() const
{
  return terminals.size();
}

int Grammar::numNonterminals() const
{
  // everywhere, we regard emptyString as a nonterminal
  return nonterminals.size() + 1;
}


void Grammar::printSymbolTypes(std::ostream &os) const
{
  os << "Grammar terminals with types or precedence:\n";
  for (const auto& t : terminals) {
    t.printDDM(os);
    if (t.precedence) {
      os << "  " << t.name << " " << ::toString(t.associativity)
         << " %prec " << t.precedence << std::endl;
    }
  }

  os << "Grammar nonterminals with types:\n";
  for (const auto& nt : nonterminals) {
    nt.printDDM(os);
  }
}


void Grammar::printProductions(std::ostream &os, bool code) const
{
  os << "Grammar productions:\n";
  for (auto const& prod : productions) {
    os << "  " << prod.toStringMore(code);
  }
}


#if 0
void Grammar::addProduction(Nonterminal *lhs, Symbol *firstRhs, ...)
{
  va_list argptr;                   // state for working through args
  Symbol *arg;
  va_start(argptr, firstRhs);       // initialize 'argptr'

  Production *prod = new Production(lhs, NULL /*tag*/);
  prod->append(firstRhs, NULL /*tag*/);
  for(;;) {
    arg = va_arg(argptr, Symbol*);  // get next argument
    if (arg == NULL) {
      break;    // end of list
    }

    prod->append(arg, NULL /*tag*/);
  }

  addProduction(prod);
}
#endif // 0


void Grammar::addProduction(Production &&prod)
{
  // I used to add emptyString if there were 0 RHS symbols,
  // but I've now switched to not explicitly saying that

  prod.prodIndex = productions.size();
  productions.emplace_back(std::move(prod));

  // if the start symbol isn't defined yet, we can here
  // implement the convention that the LHS of the first
  // production is the start symbol
  if (startSymbol == NULL) {
    startSymbol = prod.left;
  }
}


// add a token to those we know about
bool Grammar::declareToken(LocString const &symbolName, int code,
                           LocString const &alias)
{
  // verify that this token hasn't been declared already
  if (findSymbolC(symbolName)) {
    std::cout << "token " << symbolName << " has already been declared\n";
    return false;
  }

  // create a new terminal class
  Terminal *term = getOrMakeTerminal(symbolName);

  // assign fields specified in %token declaration
  term->termIndex = code;
  term->alias = alias;

  return true;
}


// well-formedness check
void Grammar::checkWellFormed() const
{
  // after removing some things, now there's nothing to check...
}


// syntax for identifying tokens in Bison output
string bisonTokenName(Terminal const *t)
{
  // this worked with older versions of Bison
  //return stringc << "\"" << t->name << "\"";

  // but the newer ones don't like quoted terminal names..
  return string(t->name.str);
}

// print the grammar in a form that Bison likes
void Grammar::printAsBison(std::ostream &os) const
{
  os << "/* automatically generated grammar */\n\n";

  os << "/* -------- tokens -------- */\n";
  for (const auto& t : terminals) {
    // I'll surround all my tokens with quotes and see how Bison likes it
    // TODO: the latest bison does *not* like it!
    os << "%token " << bisonTokenName(&t) << " "
       << t.termIndex << "\n";
  }
  os << "\n\n";

  os << "/* -------- precedence and associativity ---------*/\n"
        "/* low precedence */\n";
  {
    // first, compute the highest precedence used anywhere in the grammar
    int highMark=0;
    for (const auto& t : terminals) {
      highMark = (std::max)(t.precedence, highMark);
    }

    // map AssocKind to bison declaration; map stuff bison doesn't
    // have to %nonassoc
    static char const * const kindMap[NUM_ASSOC_KINDS] =
      { "%left", "%right", "%nonassoc", "%nonassoc", "%nonassoc" };

    // now iterate over the precedence levels (level 0 is skipped
    // because it means 'unspecified')
    for (int level=1; level <= highMark; level++) {
      AssocKind kind = NUM_ASSOC_KINDS;   // means we haven't seen any kind yet
      for (const auto& t : terminals) {
        if (t.precedence == level) {
          if (kind == NUM_ASSOC_KINDS) {
            // first token at this level
            kind = t.associativity;
            os << kindMap[kind];
          }
          else if (kind != t.associativity) {
            xfailure("different associativities at same precedence?!");
          }

          // print the token itself
          os << " " << bisonTokenName(&t);
        }
      }

      // end of the level
      os << "\n";
    }
  }
  os << "/* high precedence */\n"
        "\n\n";

  os << "/* -------- productions ------ */\n"
        "%%\n\n";
  // print every nonterminal's rules
  for (const auto& nt : nonterminals) {
    // look at every rule where this nonterminal is on LHS
    bool first = true;
    for (auto const& prod : productions) {
      if (prod.left == &nt) {

        if (first) {
          os << nt.name << ":";
        }
        else {
          os << "\n";
          INTLOOP(i, 0, nt.name.length()) {
            os << " ";
          }
          os << "|";
        }

        // print RHS symbols
        for (auto const& elt : prod.right) {
          Symbol const *sym = elt.sym;
          if (sym != &emptyString) {
            if (sym->isTerminal()) {
              os << " " << bisonTokenName(&( sym->asTerminalC() ));
            }
            else {
              os << " " << sym->name;
            }
          }
        }

        // or, if empty..
        if (prod.rhsLength() == 0) {
          os << " /* empty */";
        }

        // precedence?
        if (prod.precedence) {
          // search for a terminal with the required precedence level
          bool found=false;
          for (const auto& t : terminals) {
            if (t.precedence == prod.precedence) {
              // found suitable token
              os << " %prec " << bisonTokenName(&t);
              found = true;
              break;
            }
          }
          if (!found) {
            std::cout << "warning: cannot find token for precedence level "
                      << prod.precedence << std::endl;
            os << " /* no token precedence level "/* */
               << prod.precedence << " */";
          }
        }

        // dummy action to help while debugging
        os << " { $$=" << prod.prodIndex << "; }";

        first = false;
      }
    }

    if (first) {
      // no rules..
      os << "/* no rules for " << nt.name << " */";
    }
    else {
      // finish the rules with a semicolon
      os << "\n";
      INTLOOP(i, 0, nt.name.length()) {
        os << " ";
      }
      os << ";";
    }

    os << "\n\n";
  }
}



// ------------------- symbol access -------------------
Nonterminal const *Grammar::findNonterminalC(char const *name) const
{
  // check for empty first, since it's not in the list
  if (emptyString.name.equals(name)) {
    return &emptyString;
  }

  for (const auto& nt : nonterminals) {
    if (nt.name.equals(name)) {
      return &nt;
    }
  }
  return NULL;
}


Terminal const *Grammar::findTerminalC(char const *name) const
{
  for (const auto& t : terminals) {
    if (t.name.equals(name) || t.alias.equals(name)) {
      return &t;
    }
  }
  return NULL;
}


Symbol const *Grammar::findSymbolC(char const *name) const
{
  // try nonterminals
  Nonterminal const *nt = findNonterminalC(name);
  if (nt) {
    return nt;
  }

  // now try terminals; if it fails, we fail
  return findTerminalC(name);
}



Nonterminal *Grammar::getOrMakeNonterminal(LocString const &name)
{
  Nonterminal *nt = findNonterminal(name);
  if (nt != NULL) {
    return nt;
  }

  nonterminals.emplace_back(name);
  return &nonterminals.back();
}

Terminal *Grammar::getOrMakeTerminal(LocString const &name)
{
  Terminal *term = findTerminal(name);
  if (term != NULL) {
    return term;
  }

  terminals.emplace_back(name);
  return &terminals.back();
}

Symbol *Grammar::getOrMakeSymbol(LocString const &name)
{
  Symbol *sym = findSymbol(name);
  if (sym != NULL) {
    return sym;
  }

  // Since name is not already defined, we don't know whether
  // it will be a nonterminal or a terminal.  For now, I will
  // use the lexical convention that nonterminals are
  // capitalized and terminals are not.
  if (isupper(name[0])) {
    return getOrMakeNonterminal(name);
  }
  else {
    return getOrMakeTerminal(name);
  }
}


int Grammar::getProductionIndex(Production const *prod) const
{
  // A production that wasn't inserted into the productions
  // list will have the default index of -1
  int ret = prod->prodIndex;
  xassert(ret != -1);
  return ret;
}


string symbolSequenceToString(SymbolList const &list)
{
  stringBuilder sb;   // collects output

  bool first = true;
  for (Symbol const* sym : list) {
    if (!first) {
      sb << " ";
    }

    if (sym->isTerminal()) {
      sb << sym->asTerminalC().toString();
    }
    else {
      sb << sym->name;
    }
    first = false;
  }

  return sb;
}


string terminalSequenceToString(TerminalList const &list)
{
  // this works because access is read-only
  /*FIXME this is a bad hack*/
  return symbolSequenceToString(reinterpret_cast<SymbolList const&>(list));
}


// ------------------ emitting C++ code ---------------------
#if 0     // not done
void Grammar::emitSelfCC(std::ostream &os) const
{
  os << "void buildGrammar(Grammar *g)\n"
        "{\n";

  FOREACH_OBJLIST(Terminal, terminals, termIter) {
    Terminal const *term = termIter.data();

    os << "g->declareToken(" << term->name
       << ", " << term->termIndex
       << ", " << quoted(term->alias)
       << ");\n";
  }

  FOREACH_OBJLIST(Nonterminal, nonterminals, ntIter) {
    Nonterminal const *nt = ntIter.data();

    os << ...
  }

  os << "}\n";

  // todo: more
}
#endif // 0
