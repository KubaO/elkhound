// allocstats.h            see license.txt for copyright and terms of use
// A mixin class to track object allocations

#ifndef ELK_ALLOCSTATS_H
#define ELK_ALLOCSTATS_H

template <class For>
class AllocStats
{
public:
  static void printAllocStats(bool anyway);

protected:
  static int numAllocd;
  static int maxAllocd;

  AllocStats()
  {
    numAllocd++;
    if (numAllocd > maxAllocd) {
      maxAllocd = numAllocd;
    }
  }

  ~AllocStats()
  {
    numAllocd--;
  }
};


// these would go in a .cc files
#define ALLOC_STATS_DEFINE(classname)                                \
  int AllocStats<classname>::numAllocd = 0;                          \
  int AllocStats<classname>::maxAllocd = 0;                          \
  STATICDEF void AllocStats<classname>::printAllocStats(bool anyway) \
  {                                                                  \
    if (anyway || numAllocd != 0) {                                  \
      std::cout << #classname << " nodes: " << numAllocd             \
           << ", max  nodes: " << maxAllocd                          \
           << std::endl;                                             \
    }                                                                \
  }

#endif // ELK_ALLOCSTATS_H
