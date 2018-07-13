/********************************************************************************/ /**
 * @file eval.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#ifndef EVAL_HPP
#define EVAL_HPP

#include "types.hpp"

namespace pscm {

Cell eval(Symenv senv, Cell expr);

Cell eval_list(const Symenv& senv, Cell list, bool is_list = true);

} // namespace pscm

#endif // EVAL_HPP
