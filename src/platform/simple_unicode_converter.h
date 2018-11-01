#pragma once
#include <stdint.h>
#include <string_view>

namespace xlang::impl
{
    //
    // A simple, flexible, self contained and stateless UTF8<->UTF16 converter.
    //
    template <class Traits> class simple_unicode_converter
    {

        // The Traits parameter should contain static functions "data_error()"
        // to report malformed input and "buffer_error()" to report an
        // insufficient output buffer. These functions may never return.
        [[noreturn]] static void invalid() { Traits::data_error(); }
        [[noreturn]] static void buffer_error() { Traits::buffer_error(); }

      public:
        // 'convert' tries to convert the *complete* range from 'in_start' to
        // 'in_end' and writes the result to the 'out_start' iterator, after
        // processing the data with SrcFilter and DestFilter.
        //
        // 'src_filter' reads data in input format from its supplied reader
        // argument and produces UTF-32 codepoints.
        //
        // 'dst_filter' converts the codepoints to the output format
        // and writers the result to its supplied writer argument.
        //
        // if 'count_only' is true, then no data is written to the output
        // iterator. The number of output characters however is counted
        // and returned.
        //
        // The function returns the number of characters written to the
        // output iterator.
        // if a complete conversion requires more input than available, then
        // the input is considered malformed.
        // if the conversion needs more output buffer than present, then
        // buffer_error() is signaled.
        //
        // Actually, this routines does not know anything about the data
        // processed.
        template <class In, class Out, class SrcFilter, class DestFilter>
        static size_t convert(In in_start, In in_end, Out out_start,
                              Out out_end, SrcFilter src_filter,
                              DestFilter dst_filter, bool count_only)
        {
            // the reader closure reads from the input iterator
            auto reader = [&in_start, &in_end]() {
                if (in_start < in_end)
                    return *in_start++;
                else
                    invalid(); // input too short, missing data
            };

            // the writer closures writes to the output iterator
            // and counts the number of written characters.
            size_t written = 0;
            auto writer = [&out_start, &out_end, &written,
                           &count_only](auto item) {
                if (!count_only)
                {
                    if (out_start < out_end)
                        *out_start++ = item;
                    else
                        buffer_error();
                }
                written++;
            };

            while (in_start < in_end)
            {
                auto cp = src_filter.read(reader);
                dst_filter.write(cp, writer);
            }
            return written;
        }

        // codepoints in the surrogate area are invalid
        static constexpr bool is_valid_range(uint32_t u)
        {
            return !((u >= 0xd800) && (u <= 0xdfff));
        }

        class utf16_filter
        {
          private:
            static constexpr bool is_high_surrogate(uint32_t u)
            {
                return ((u >= 0xd800) && (u <= 0xdbff));
            }
            static constexpr bool is_low_surrogate(uint32_t u)
            {
                return ((u >= 0xdc00) && (u <= 0xdfff));
            }

          public:
            // read up to two UTF-16 codepoints from 'in' and try to make
            // a valid UTF-32 codepoint from it.
            template <class In> static uint32_t read(In in)
            {
                uint16_t h = in();
                if (is_high_surrogate(h))
                {
                    uint16_t l = in();
                    if (!is_low_surrogate(l))
                    {
                        invalid();
                    }
                    return ((h - 0xd800u) << 10) + (l - 0xdc00u) + 0x10000u;
                }
                return h;
            }

            // write up to two UTF-16 codepoints to 'out'. The output byte order
            // is the native byte order.
            template <class Out> static int write(uint32_t c, Out out)
            {
                if (!is_valid_range(c))
                {
                    invalid(); // reserved
                }
                if (c < 0x10000u)
                {
                    out(c);
                    return 1;
                }
                else
                {
                    uint16_t p0, p1;
                    c -= 0x10000u;
                    uint16_t h = 0xd800u + (c >> 10);
                    if (!is_high_surrogate(h))
                        invalid();
                    uint16_t l = 0xdc00u + (c & 0x3ffu);
                    if (!is_low_surrogate(l))
                        invalid();
                    out(h);
                    out(l);
                    return 2;
                }
            }
        };

        class utf8_filter
        {
          private:
            // Read 'Count' bits starting from 'Start' in cp and put 'Mark' over
            // the octet result.
            template <unsigned Mark, unsigned Start, unsigned Count>
            static uint8_t fetch(uint32_t cp)
            {
                return Mark | ((cp >> Start) & ((1u << Count) - 1));
            }
            // The opposite of fetch:
            // If b contains the bits from 'Mark', then
            // store the lowest 'Count' bits from b in to 'cp' at position
            // 'Start' If the mark is not present, the input data is malformed.
            template <unsigned Mark, unsigned Start, unsigned Count>
            static void store(uint32_t &cp, uint8_t b)
            {
                //
                auto mask = ((1u << Count) - 1);
                if ((b & ~mask) == Mark)
                {
                    cp |= (b & mask) << Start;
                }
                else
                {
                    invalid();
                }
            }

          public:
            // Read up to 4 input bytes as UTF-8 and produce a UTF-32 codepoint
            // in native byte order.
            template <class In> static uint32_t read(In in)
            {
                uint32_t cp = 0;
                uint8_t b = in();
                if (b <= 0x7f)
                {
                    return b;
                }
                if (b <= 0xdf)
                {
                    store<0xc0, 6, 5>(cp, b);
                    store<0x80, 0, 6>(cp, in());
                    if (cp <= 0x7f)
                        invalid(); // overlong encoding
                    return cp;
                }
                if (b <= 0xef)
                {
                    store<0xe0, 12, 4>(cp, b);
                    store<0x80, 6, 6>(cp, in());
                    store<0x80, 0, 6>(cp, in());
                    if (cp <= 0x7ff)
                        invalid(); // overlong encoding
                    return cp;
                }
                if (b <= 0xf7)
                {
                    store<0xf0, 18, 3>(cp, b);
                    store<0x80, 12, 6>(cp, in());
                    store<0x80, 6, 6>(cp, in());
                    store<0x80, 0, 6>(cp, in());
                    if (cp <= 0xffff)
                        invalid(); // overlong encoding
                    return cp;
                }
                invalid();
            }

            // Convert a UTF-32 codepoint into up to 4 UTF-8 code points and
            // write them to 'out' callable.
            template <class Out> static int write(uint32_t cp, Out out)
            {
                if (!is_valid_range(cp))
                {
                    invalid();
                }

                if (cp <= 0x7f)
                {
                    out(static_cast<uint8_t>(cp));
                    return 1;
                }
                if (cp <= 0x7ff)
                {
                    out(fetch<0xc0, 6, 5>(cp));
                    out(fetch<0x80, 0, 6>(cp));
                    return 2;
                }
                if (cp <= 0xffff)
                {
                    out(fetch<0xe0, 12, 4>(cp));
                    out(fetch<0x80, 6, 6>(cp));
                    out(fetch<0x80, 0, 6>(cp));
                    return 3;
                }
                if (cp <= 0x10FFFF)
                {
                    out(fetch<0xf0, 18, 3>(cp));
                    out(fetch<0x80, 12, 6>(cp));
                    out(fetch<0x80, 6, 6>(cp));
                    out(fetch<0x80, 0, 6>(cp));
                    return 4;
                }
                invalid();
            }
        };
    };

} // namespace xlang::impl
