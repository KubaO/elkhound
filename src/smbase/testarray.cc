// testarray.cc
// test the array.h module

#include "array.h"     // module to test
#include "ckheap.h"    // malloc_stats

#include <deque>       // std::deque as a reference
#include <stdio.h>     // printf
#include <stdlib.h>    // exit


int maxLength = 0;

// one round of testing
void round(int ops)
{
  // implementations to test
  ArrayStack<int> arrayStack;
  ArrayStackEmbed<int, 10> arrayStackEmbed;

  // "trusted" implementation to compare with
  std::deque<int> dequeStack;

  while (ops--) {
    // check that the arrays and list agree
    {
      int length = dequeStack.size();
      if (length > 0) {
        xassert(dequeStack.front() == arrayStack.top());
        xassert(dequeStack.front() == arrayStackEmbed.top());
      }

      int index = length-1;
      for (int ref : dequeStack) {
        xassert(ref == arrayStack[index]);
        xassert(ref == arrayStackEmbed[index]);
        index--;
      }
      xassert(index == -1);
      xassert(length == arrayStack.length());
      xassert(length == arrayStackEmbed.length());
      xassert(arrayStack.isEmpty() == dequeStack.empty());
      xassert(arrayStackEmbed.isEmpty() == dequeStack.empty());
      xassert(arrayStack.isNotEmpty() == !dequeStack.empty());
      xassert(arrayStackEmbed.isNotEmpty() == !dequeStack.empty());

      if (length > maxLength) {
        maxLength = length;
      }
    }

    // do a random operation
    int op = rand() % 100;
    if (op < 40 && arrayStack.isNotEmpty()) {
      // pop
      int i = arrayStack.pop();
      int j = arrayStackEmbed.pop();
      int k = dequeStack.front();
      dequeStack.pop_front();
      xassert(i == k);
      xassert(j == k);
    }
    else {
      // push
      int elt = rand() % 100;
      arrayStack.push(elt);
      arrayStackEmbed.push(elt);
      dequeStack.push_front(elt);
    }
  }
}


int main()
{
  for (int i=0; i<20; i++) {
    round(1000);
  }

  malloc_stats();
  printf("arrayStack appears to work; maxLength=%d\n", maxLength);
  return 0;
}


// EOF
