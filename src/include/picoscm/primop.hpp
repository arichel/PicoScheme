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

class Scheme;

namespace pscm {

/**
 * Call a primary scheme function.
 * @param senv   The current symbol environment.
 * @param primop Scheme function opcode as defined by enum class @ref pscm::Intern.
 * @param args   Function argument vector.
 * @return Function result or special symbol @ref pscm::none for a void function.
 */
Cell call(Scheme& scm, const SymenvPtr& senv, Intern primop, const std::vector<Cell>& args);

//! Install scheme opcodes, standard symbols and common mathematical and physical constants.
void add_environment_defaults(Scheme& scm);

} // namespace pscm
#endif // PRIMOP_HPP
