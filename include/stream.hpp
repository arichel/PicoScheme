/*********************************************************************************/ /**
 * @file stream.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#ifndef STREAM_HPP
#define STREAM_HPP

#include <ostream>

#include "cell.hpp"

namespace pscm {

std::ostream& operator<<(std::ostream& os, const Cell& cell);

}; // namespace pscm
#endif // STREAM_HPP
