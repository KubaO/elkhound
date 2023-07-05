// format.h            see license.txt for copyright and terms of use

#ifndef ELK_FORMAT_H
#define ELK_FORMAT_H

#include "fmt/core.h"


template <typename T> struct DelegatingFormatter
{
  constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }
  fmt::format_context::iterator format(const T& obj, fmt::format_context& ctx) const
  {
    return obj.format(ctx);
  }
};

#endif // ELK_FORMAT_H
