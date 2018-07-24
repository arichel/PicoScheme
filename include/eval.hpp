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

/**
 * Evaluate a scheme expression at the argument symbol environment.
 *
 * @param senv Shared pointer to the symbol environment, where to
 *             to evaluate expr.
 * @param expr Scheme expression to evaluate.
 * @return Evaluation result or special symbol @em none for no result.
 */
Cell eval(SymenvPtr senv, Cell expr);

Cell eval_list(const SymenvPtr& senv, Cell list, bool is_list = true);

namespace syntax {
    /**
     * Scheme begin syntax to evaluate a sequence of scheme
     * expressions in consecutive order.
     *
     * @return Unevaluated last expression or special symbol none for an
     *         empty argument list.
     */
    Cell _begin(const SymenvPtr& senv, Cell args);
}

/**
 * Interactive scheme read eval print loop.
 *
 * @param symenv
 *        Optional, unless null-pointer, a shared parent environment pointer.
 *        or use default environment as parent.
 *
 * @param in  Optional, an input stream or use default std::cin.
 * @param out Optional, an output stream or use default std::cout.
 */
void repl(const SymenvPtr& symenv = nullptr,
    std::istream& in = std::cin, std::ostream& out = std::cout);

} // namespace pscm

#endif // EVAL_HPP
