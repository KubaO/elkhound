// allocstats.h            see license.txt for copyright and terms of use
// A mixin class to track object allocations

#ifndef ELK_ALLOCSTATS_H
#define ELK_ALLOCSTATS_H

// for a class that maintains allocated-node stats
#define ALLOC_STATS_DECLARE                     \
  static int numAllocd;                         \
  static int maxAllocd;                         \
  static void printAllocStats(bool anyway);

// these would go in a .cc file, whereas above goes in .h file
#define ALLOC_STATS_DEFINE(classname)                      \
  int classname::numAllocd = 0;                            \
  int classname::maxAllocd = 0;                            \
  STATICDEF void classname::printAllocStats(bool anyway)   \
  {                                                        \
    if (anyway || numAllocd != 0) {                        \
      std::cout << #classname << " nodes: " << numAllocd   \
           << ", max  nodes: " << maxAllocd                \
           << std::endl;                                   \
    }                                                      \
  }

#define ALLOC_STATS_IN_CTOR                     \
  INC_HIGH_WATER(numAllocd, maxAllocd);

#define ALLOC_STATS_IN_DTOR                     \
  numAllocd--;

#endif // ELK_ALLOCSTATS_H
