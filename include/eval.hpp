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

#include "cell.hpp"

namespace pscm {

Cell eval(Symenv senv, Cell expr);
Cell call(const Symenv& senv, Intern primop, const std::vector<Cell>& args);

Cell eval_list(const Symenv& senv, Cell list);

} // namespace pscm

#endif // EVAL_HPP
