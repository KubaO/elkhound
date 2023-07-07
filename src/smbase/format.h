// format.h            see license.txt for copyright and terms of use

#ifndef ELK_FORMAT_H
#define ELK_FORMAT_H

#include "fmt/format.h"


template <typename T> struct DelegatingFormatter
{
  constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }
  fmt::format_context::iterator format(const T& obj, fmt::format_context& ctx) const
  {
    return obj.format(ctx);
  }
};


/*
* Invokes T::format(ctx, options)
* - options come from the format string, e.g. '{:foo@}' yields options=='foo@'
*/
template <typename T> struct PolymorphicFormatter {
  fmt::string_view formatOpts;
  // {:....@} options are supported and passed to the format method
  constexpr fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx)
  {
    auto begin = ctx.begin(), it = ctx.begin(), end = ctx.end();
    if (it != end && *it != '@') {
      while (it != end && *it != '@')
        ++it;
      if (it != begin && it != end && *it == '@') {
        formatOpts = fmt::string_view(begin, it - begin);
        ++it; // skip '@'
      }
      else
        return begin;
    }
    return it;
  }
  fmt::format_context::iterator format(const T& instance, fmt::format_context& ctx) const {
    return instance.format(ctx, formatOpts);
  }
};


/*
* Extracts typed arguments from argument lists in fmt::format_context
*/

// A visitor for fmt::format_args
template <typename T>
struct FormatContextVisitor {
  T value = {};

  FormatContextVisitor() = default;
  FormatContextVisitor(const T& v) : value(v) {}
  FormatContextVisitor(T&& v) : value(std::move(v)) {}
  template <typename Q> void operator()(Q) {}
  template <> void operator()(T&& v) { value = std::move(v); }
  template <> void operator()(const T& v) { value = v; }
};

template <typename T>
T fmtGetArg(fmt::format_context& ctx, fmt::string_view name, T default = {})
{
  FormatContextVisitor<T> vis(default);
  auto arg = ctx.arg(name);
  if (arg)
    fmt::visit_format_arg(vis, arg);
  return vis.value;
}


inline fmt::memory_buffer& operator<<(fmt::memory_buffer& buf, nonstd::string_view view)
{
  buf.append(fmt::string_view(view.data(), view.size()));
  return buf;
}

template <typename... T>
inline auto format_to(fmt::memory_buffer& buf, fmt::string_view fmt, T&&...args)
{
  return fmt::format_to(fmt::appender(buf), fmt, std::forward<T>(args)...);
}

template <typename... T>
inline auto format_to(std::string& str, fmt::string_view fmt, T&&...args)
{
  return fmt::format_to(std::back_inserter(str), fmt, std::forward<T>(args)...);
}

inline auto vformat_to(std::string& str, fmt::string_view fmt, fmt::format_args args)
{
  return fmt::vformat_to(std::back_inserter(str), fmt, args);
}


#endif // ELK_FORMAT_H
