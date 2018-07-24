/*********************************************************************************/ /**
 * @file
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#ifndef PRIMOP_HPP
#define PRIMOP_HPP

#include <vector>

#include "types.hpp"

namespace pscm {

/**
 * Call a primary scheme function.
 * @param senv   The current symbol environment.
 * @param primop Scheme function opcode as defined by enum class @ref pscm::Intern.
 * @param args   Function argument vector.
 * @return Function result or special symbol @ref pscm::none for a void function.
 */
Cell call(const SymenvPtr& senv, Intern primop, const std::vector<Cell>& args);

}; // namespace pscm
#endif // PRIMOP_HPP
