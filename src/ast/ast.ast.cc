// ast.ast.cc
// *** DO NOT EDIT ***
// generated automatically by astgen, from ast.ast

#include "ast.ast.h"      // this module


// ------------------ ASTSpecFile -------------------
// *** DO NOT EDIT ***
ASTSpecFile::~ASTSpecFile()
{
  deleteAll(forms);
}

void ASTSpecFile::debugPrint(std::ostream &os, int indent, char const *subtreeName) const
{
  PRINT_HEADER(subtreeName, ASTSpecFile);

  PRINT_LIST(ToplevelForm, forms);
  (void)os, (void)indent;
}

ASTSpecFile *ASTSpecFile::clone() const
{
  ASTSpecFile *ret = new ASTSpecFile(
    cloneASTList(forms)
  );
  return ret;
}


// ------------------ ToplevelForm -------------------
// *** DO NOT EDIT ***
ToplevelForm::~ToplevelForm()
{
}

char const * const ToplevelForm::kindNames[ToplevelForm::NUM_KINDS] = {
  "TF_verbatim",
  "TF_impl_verbatim",
  "TF_class",
  "TF_option",
  "TF_custom",
  "TF_enum",
};

void ToplevelForm::debugPrint(std::ostream &os, int indent, char const *subtreeName) const
{
  (void)os, (void)indent, (void)subtreeName;
}

DEFN_AST_DOWNCASTS(ToplevelForm, TF_verbatim, TF_VERBATIM)

TF_verbatim::~TF_verbatim()
{
}

void TF_verbatim::debugPrint(std::ostream &os, int indent, char const *subtreeName) const
{
  PRINT_HEADER(subtreeName, TF_verbatim);

  ToplevelForm::debugPrint(os, indent, subtreeName);

  PRINT_STRING(code);
}

TF_verbatim *TF_verbatim::clone() const
{
  TF_verbatim *ret = new TF_verbatim(
    code
  );
  return ret;
}

DEFN_AST_DOWNCASTS(ToplevelForm, TF_impl_verbatim, TF_IMPL_VERBATIM)

TF_impl_verbatim::~TF_impl_verbatim()
{
}

void TF_impl_verbatim::debugPrint(std::ostream &os, int indent, char const *subtreeName) const
{
  PRINT_HEADER(subtreeName, TF_impl_verbatim);

  ToplevelForm::debugPrint(os, indent, subtreeName);

  PRINT_STRING(code);
}

TF_impl_verbatim *TF_impl_verbatim::clone() const
{
  TF_impl_verbatim *ret = new TF_impl_verbatim(
    code
  );
  return ret;
}

DEFN_AST_DOWNCASTS(ToplevelForm, TF_class, TF_CLASS)

TF_class::~TF_class()
{
  delete super;
  deleteAll(ctors);
}

void TF_class::debugPrint(std::ostream &os, int indent, char const *subtreeName) const
{
  PRINT_HEADER(subtreeName, TF_class);

  ToplevelForm::debugPrint(os, indent, subtreeName);

  PRINT_SUBTREE(super);
  PRINT_LIST(ASTClass, ctors);
}

TF_class *TF_class::clone() const
{
  TF_class *ret = new TF_class(
    super? super->clone() : NULL,
    cloneASTList(ctors)
  );
  return ret;
}

DEFN_AST_DOWNCASTS(ToplevelForm, TF_option, TF_OPTION)

TF_option::~TF_option()
{
  args.clear();
}

void TF_option::debugPrint(std::ostream &os, int indent, char const *subtreeName) const
{
  PRINT_HEADER(subtreeName, TF_option);

  ToplevelForm::debugPrint(os, indent, subtreeName);

  PRINT_STRING(name);
  PRINT_LIST(string, args);
}

TF_option *TF_option::clone() const
{
  TF_option *ret = new TF_option(
    name,
    shallowCloneASTList(args)
  );
  return ret;
}

DEFN_AST_DOWNCASTS(ToplevelForm, TF_custom, TF_CUSTOM)

TF_custom::~TF_custom()
{
  delete cust;
}

void TF_custom::debugPrint(std::ostream &os, int indent, char const *subtreeName) const
{
  PRINT_HEADER(subtreeName, TF_custom);

  ToplevelForm::debugPrint(os, indent, subtreeName);

  PRINT_SUBTREE(cust);
}

TF_custom *TF_custom::clone() const
{
  TF_custom *ret = new TF_custom(
    cust? cust->clone() : NULL
  );
  return ret;
}

DEFN_AST_DOWNCASTS(ToplevelForm, TF_enum, TF_ENUM)

TF_enum::~TF_enum()
{
  enumerators.clear();
}

void TF_enum::debugPrint(std::ostream &os, int indent, char const *subtreeName) const
{
  PRINT_HEADER(subtreeName, TF_enum);

  ToplevelForm::debugPrint(os, indent, subtreeName);

  PRINT_STRING(name);
  PRINT_LIST(string, enumerators);
}

TF_enum *TF_enum::clone() const
{
  TF_enum *ret = new TF_enum(
    name,
    shallowCloneASTList(enumerators)
  );
  return ret;
}


// ------------------ ASTClass -------------------
// *** DO NOT EDIT ***
ASTClass::~ASTClass()
{
  deleteAll(args);
  deleteAll(lastArgs);
  deleteAll(bases);
  deleteAll(decls);
}

void ASTClass::debugPrint(std::ostream &os, int indent, char const *subtreeName) const
{
  PRINT_HEADER(subtreeName, ASTClass);

  PRINT_STRING(name);
  PRINT_LIST(CtorArg, args);
  PRINT_LIST(CtorArg, lastArgs);
  PRINT_LIST(BaseClass, bases);
  PRINT_LIST(Annotation, decls);
  (void)os, (void)indent;
}

ASTClass *ASTClass::clone() const
{
  ASTClass *ret = new ASTClass(
    name,
    cloneASTList(args),
    cloneASTList(lastArgs),
    cloneASTList(bases),
    cloneASTList(decls)
  );
  return ret;
}


// ------------------ AccessMod -------------------
// *** DO NOT EDIT ***
AccessMod::~AccessMod()
{
  mods.clear();
}

void AccessMod::debugPrint(std::ostream &os, int indent, char const *subtreeName) const
{
  PRINT_HEADER(subtreeName, AccessMod);

  PRINT_GENERIC(acc);
  PRINT_LIST(string, mods);
  (void)os, (void)indent;
}

AccessMod *AccessMod::clone() const
{
  AccessMod *ret = new AccessMod(
    acc,
    shallowCloneASTList(mods)
  );
  return ret;
}


// ------------------ Annotation -------------------
// *** DO NOT EDIT ***
Annotation::~Annotation()
{
}

char const * const Annotation::kindNames[Annotation::NUM_KINDS] = {
  "UserDecl",
  "CustomCode",
};

void Annotation::debugPrint(std::ostream &os, int indent, char const *subtreeName) const
{
  (void)os, (void)indent, (void)subtreeName;
}

DEFN_AST_DOWNCASTS(Annotation, UserDecl, USERDECL)

UserDecl::~UserDecl()
{
  delete amod;
}

void UserDecl::debugPrint(std::ostream &os, int indent, char const *subtreeName) const
{
  PRINT_HEADER(subtreeName, UserDecl);

  Annotation::debugPrint(os, indent, subtreeName);

  PRINT_SUBTREE(amod);
  PRINT_STRING(code);
  PRINT_STRING(init);
}

UserDecl *UserDecl::clone() const
{
  UserDecl *ret = new UserDecl(
    amod? amod->clone() : NULL,
    code,
    init
  );
  return ret;
}

DEFN_AST_DOWNCASTS(Annotation, CustomCode, CUSTOMCODE)

CustomCode::~CustomCode()
{
}

void CustomCode::debugPrint(std::ostream &os, int indent, char const *subtreeName) const
{
  PRINT_HEADER(subtreeName, CustomCode);

  Annotation::debugPrint(os, indent, subtreeName);

  PRINT_STRING(qualifier);
  PRINT_STRING(code);
}

CustomCode *CustomCode::clone() const
{
  CustomCode *ret = new CustomCode(
    qualifier,
    code
  );
  return ret;
}


// ------------------ CtorArg -------------------
// *** DO NOT EDIT ***
CtorArg::~CtorArg()
{
}

void CtorArg::debugPrint(std::ostream &os, int indent, char const *subtreeName) const
{
  PRINT_HEADER(subtreeName, CtorArg);

  PRINT_BOOL(isOwner);
  PRINT_STRING(type);
  PRINT_STRING(name);
  PRINT_STRING(defaultValue);
  (void)os, (void)indent;
}

CtorArg *CtorArg::clone() const
{
  CtorArg *ret = new CtorArg(
    isOwner,
    type,
    name,
    defaultValue
  );
  return ret;
}


// ------------------ BaseClass -------------------
// *** DO NOT EDIT ***
BaseClass::~BaseClass()
{
}

void BaseClass::debugPrint(std::ostream &os, int indent, char const *subtreeName) const
{
  PRINT_HEADER(subtreeName, BaseClass);

  PRINT_GENERIC(access);
  PRINT_STRING(name);
  (void)os, (void)indent;
}

BaseClass *BaseClass::clone() const
{
  BaseClass *ret = new BaseClass(
    access,
    name
  );
  return ret;
}


// *** DO NOT EDIT ***


#include "strutil.h"      // stringToupper

string toString(AccessCtl acc)
{
  char const *arr[] = {
    "public",
    "private",
    "protected",
    "ctor",
    "dtor",
    "pure_virtual"
  };
  static_assert(TABLESIZE(arr) == NUM_ACCESSCTLS,
                "arr has wrong size relative to AccessCtl enum");
  xassert((unsigned)acc < NUM_ACCESSCTLS);
  return string(arr[acc]);
}

string ASTClass::classKindName() const
{
  string ret = stringToupper(name);
  if (ret == name) {
    // this simplemindedly avoids collisions with itself, and I think
    // it even avoids collisions with other classes, since if they would
    // collide with this, they'd collide with themselves too, and hence
    // get an extra "KIND_" prepended..
    ret.append("KIND_");
  }
  return ret;
}

bool AccessMod::hasMod(char const *mod) const
{
  FOREACH_ASTLIST(string, mods, iter) {
    if (*iter == mod) {
      return true;
    }
  }
  return false;      // not found
}

bool AccessMod::hasModPrefix(char const *mod) const
{
  string mod0(mod);
  FOREACH_ASTLIST(string, mods, iter) {
    rostring i = *iter;
    if (prefixEquals(i, mod0)) {
      return true;
    }
  }
  return false;      // not found
}

string AccessMod::getModSuffixFromPrefix(char const *mod) const
{
  string mod0(mod);
  string ret;
  bool found = false;
  FOREACH_ASTLIST(string, mods, iter) {
    rostring s = *iter;
    if (prefixEquals(s, mod0)) {
      if (found) {
        xfailure("two modifiers with this prefix found {}", mod);
      }
      int len = strlen(mod);
      ret = s.substr(len, s.length()-len);
      found = true;
    }
  }
  if (!found) {
    xfailure("no such prefix found {}", mod);
  }
  return ret;
}



