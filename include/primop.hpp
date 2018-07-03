/*********************************************************************************/ /**
 * @file primop.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#ifndef PRIMOP_HPP
#define PRIMOP_HPP

#include <vector>

#include "cell.hpp"

namespace pscm {

Cell call(const Symenv& senv, Intern primop, const std::vector<Cell>& args);

}; // namespace pscm
#endif // PRIMOP_HPP
