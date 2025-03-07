// trivbison.cc            see license.txt for copyright and terms of use
// driver file for a Bison-parser with the trivial lexer

#include "trivbison.h"  // this module
#include "lexer2.h"     // Lexer2
#include "trivlex.h"    // trivialLexer
#include "trace.h"      // traceProgress
#include "syserr.h"     // xsyserror
#include "ptreenode.h"  // PTreeNode
#include "cyctimer.h"   // CycleTimer

#include <stdio.h>      // printf
#include <iostream>     // std::cout, etc.

// global list of L2 tokens for yielding to Bison
Lexer2 lexer2;
Lexer2Token const *lastTokenYielded = NULL;

// parsing entry point
int yyparse();

// returns token types until EOF, at which point L2_EOF is returned
int yylex()
{
  // prepare to return tokens
  static std::deque<Lexer2Token>::iterator
    iter = lexer2.tokens.begin(), end = lexer2.tokens.end();


  if (iter != end) {
    // grab type to return
    lastTokenYielded = &*iter;
    Lexer2TokenType ret = iter->type;

    // advance to next token
    iter++;

    // return one we just advanced past
    return ret;
  }
  else {
    // done; don't bother freeing things
    lastTokenYielded = NULL;
    return L2_EOF;
  }
}


void yyerror(char const *s)
{
  if (lastTokenYielded) {
    printf("%s: ", lastTokenYielded->loc.toString().pcharc());
  }
  else {
    printf("<eof>: ");
  }
  printf("%s\n", s);
}


int main(int argc, char *argv[])
{
  char const *progname = argv[0];

  if (argc >= 2 &&
      0==strcmp(argv[1], "-d")) {
    #ifdef YYDEBUG
      yydebug = 1;
    #else
      printf("debugging is disabled because YYDEBUG isn't set\n");
      return 2;
    #endif

    argc--;
    argv++;
  }

  if (argc < 2) {
    printf("usage: %s [-d] inputfile\n", progname);
    printf("  -d: turn on yydebug, so it prints shift/reduce actions\n");
    return 0;
  }

  char const *inputFname = argv[1];

  traceAddSys("progress");

  // run lexer
  traceProgress() << "lexical analysis...\n";
  trivialLexer(inputFname, lexer2);

  // start Bison-parser
  traceProgress() << "starting parse..." << std::endl;
  CycleTimer timer;

  if (yyparse() != 0) {
    std::cout << "yyparse returned with an error\n";
  }

  traceProgress() << "finished parse (" << timer.elapsed() << ")" << std::endl;

  std::cout << "tree nodes: " << PTreeNode::allocCount
       << std::endl;

  return 0;
}
