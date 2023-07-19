// postorder.cc            see license.txt for copyright and terms of use
// given the AST for a function, compute a reverse postorder
// enumeration of all the statements

#include "c.ast.gen.h"     // C AST stuff, including decl for this module
#include "algo.h"          // sm::contains

#include <unordered_set>   // std::unordered_set

using StatementSet = std::unordered_set<Statement const *>;

// DFS from 'node', having arrived at node with 'isContinue'
// disposition; 'seen' is those nodes either currently being
// considered somewhere in the call chain ("gray"), or else finished
// entirely ("black"), and 'seenCont' is the same thing but for the
// continue==true halves of the nodes
static void p_dfs(NextPtrList &order, Statement const *node, bool isContinue,
                  StatementSet &seen, StatementSet &seenCont)
{
  // we're now considering 'node'
  (isContinue? seenCont : seen).insert(node);

  // consider each of this node's successors
  NextPtrList successors;
  node->getSuccessors(successors, isContinue);

  for (NextPtr np : successors) {
    Statement const *succ = nextPtrStmt(np);
    bool succCont = nextPtrContinue(np);

    if (sm::contains((succCont? seenCont : seen), succ)) {
      // we're already considering, or have already considered, this node;
      // do nothing with it
    }
    else {
      // visit this new child
      p_dfs(order, succ, succCont, seen, seenCont);
    }
  }

  // add the node in postorder
  order.push_back(makeNextPtr(node, isContinue));
}


void reversePostorder(NextPtrList &order, TF_func const &func)
{
  xassert(order.empty());

  // DFS from the function start, computing the spanning tree implicitly,
  // and the postorder explicitly
  StatementSet seen, seenCont;
  // first, we get postorder
  p_dfs(order, func.body, false /*isContinue*/, seen, seenCont);
  // then we reverse it to get reverse postorder
  std::reverse(order.begin(), order.end());
}
