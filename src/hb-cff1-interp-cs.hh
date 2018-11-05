/*
 * Copyright © 2018 Adobe Systems Incorporated.
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 * Adobe Author(s): Michiharu Ariza
 */
#ifndef HB_CFF1_INTERP_CS_HH
#define HB_CFF1_INTERP_CS_HH

#include "hb.hh"
#include "hb-cff-interp-cs-common.hh"

namespace CFF {

using namespace OT;

typedef BiasedSubrs<CFF1Subrs>   CFF1BiasedSubrs;

struct CFF1CSInterpEnv : CSInterpEnv<Number, CFF1Subrs>
{
  template <typename ACC>
  inline void init (const ByteStr &str, ACC &acc, unsigned int fd)
  {
    SUPER::init (str, *acc.globalSubrs, *acc.privateDicts[fd].localSubrs);
    processed_width = false;
    has_width = false;
    arg_start = 0;
  }

  inline void fini (void)
  {
    SUPER::fini ();
  }

  inline void set_width (bool has_width_)
  {
    if (likely (!processed_width && (SUPER::argStack.get_count () > 0)))
    {
      if (has_width_)
      {
        width = SUPER::argStack[0];
        has_width = true;
        arg_start = 1;
      }
    }
    processed_width = true;
  }

  inline void clear_args (void)
  {
    arg_start = 0;
    SUPER::clear_args ();
  }

  bool          processed_width;
  bool          has_width;
  unsigned int  arg_start;
  Number        width;

  private:
  typedef CSInterpEnv<Number, CFF1Subrs> SUPER;
};

template <typename OPSET, typename PARAM, typename PATH=PathProcsNull<CFF1CSInterpEnv, PARAM> >
struct CFF1CSOpSet : CSOpSet<Number, OPSET, CFF1CSInterpEnv, PARAM, PATH>
{
  /* PostScript-originated legacy opcodes (OpCode_add etc) are unsupported */

  static inline void check_width (OpCode op, CFF1CSInterpEnv &env, PARAM& param)
  {
    if (!env.processed_width)
    {
      bool  has_width = false;
      switch (op)
      {
        default:
        case OpCode_endchar:
          has_width = (env.argStack.get_count () > 0);
          break;
        case OpCode_hstem:
        case OpCode_hstemhm:
        case OpCode_hintmask:
        case OpCode_cntrmask:
          has_width = ((env.argStack.get_count () & 1) != 0);
          break;
        case OpCode_hmoveto:
        case OpCode_vmoveto:
          has_width = (env.argStack.get_count () > 1);
          break;
        case OpCode_rmoveto:
          has_width = (env.argStack.get_count () > 2);
          break;
      }
      env.set_width (has_width);
    }
  }

  static inline void flush_args (CFF1CSInterpEnv &env, PARAM& param)
  {
    SUPER::flush_args (env, param);
    env.clear_args ();  /* pop off width */
  }

  private:
  typedef CSOpSet<Number, OPSET, CFF1CSInterpEnv, PARAM, PATH>  SUPER;
};

template <typename OPSET, typename PARAM>
struct CFF1CSInterpreter : CSInterpreter<CFF1CSInterpEnv, OPSET, PARAM> {};

} /* namespace CFF */

#endif /* HB_CFF1_INTERP_CS_HH */