#include <iomanip>

#include "gc.hpp"
#include "scheme.hpp"

#define car get<0>
#define cdr get<1>
#define mrk get<2>

namespace pscm {

void GCollector::collect(Scheme& scm, const SymenvPtr& env)
{
    // Mark phase: mark all reacheable cons-cells
    vset.clear();
    eset.clear();

    mark(env);

    vset.clear();
    eset.clear();
    size_t size = scm.store.size();

    // Sweep phase: remove all unmarked cons-cells
    scm.store.remove_if([](Cons& cons) {
        mrk(cons) = !mrk(cons);
        return mrk(cons);
    });

    // Optional log number of released cells
    if (logon) {
        size_t dlta = size - scm.store.size();
        std::cerr << "msg> garbage collector released " << dlta
                  << " cons-cells from " << size << " in total\n";
    }
}

void GCollector::logging(bool ok) { logon = ok; }

void GCollector::dump(const Scheme& scm, const Port<Char>& port)
{
    auto& os = port.getStream();

    os << "Store size: " << scm.store.size() << '\n';
    size_t ic = 0;
    for (auto& cons : scm.store) {
        os << ic++ << " | mark: " << mrk(cons) << " | "
           << std::left << std::setw(25)
           << car(cons) << " : " << cdr(cons) << '\n';
    }
}

//! Return true if a Cons-cell is marked.
bool GCollector::is_marked(const Cons& cons) const noexcept { return mrk(cons); }

//! Visit a scheme cell and mark.
void GCollector::mark(const Cell& cell)
{
    // clang-format off
    std::visit(overloads{
        [this](Cons* cons)            { mark(*cons); },
        [this](const Procedure& proc) { mark(proc); },
        [this](const VectorPtr& vec)  { mark(vec); },
        [this](const SymenvPtr& env)  { mark(env); },
        [](auto&)                     { return; } },
        static_cast<const Cell::base_type&>(cell));
    // clang-format on
}

//! Mark cells reacheable from the argument symbol environment.
void GCollector::mark(const SymenvPtr& env)
{
    using Cursor = SymenvPtr::element_type::Cursor;
    std::optional<Cursor> next = env->cursor();

    do {
        Cursor cursor{ next.value() };
        auto env = cursor.symenv();

        auto [pos, ok] = eset.insert(env.get());
        if (!ok)
            return;

        for (auto& [sym, cell] : cursor) {
            mark(cell);
        }
        next = cursor.next();
    } while (next.has_value());
}

//! Mark code and argument list and closure environment of a scheme procedure.
void GCollector::mark(const Procedure& proc)
{
    const Cons& code = *get<Cons*>(proc.code());

    if (is_marked(code))
        return;

    mark(proc.code());
    mark(proc.args());
    mark(proc.senv());
    return;
}

//! Mark all cons-cells if any, contained in a scheme vector.
void GCollector::mark(const VectorPtr& vec)
{
    auto [pos, ok] = vset.insert(vec.get());
    if (!ok)
        return;

    for (auto& cell : *vec) {
        mark(cell);
    }
}

//! Mark all Cons-cells in a list.
void GCollector::mark(Cons& cons)
{
    Cell cell{ &cons };
    do {
        Cons& next = *get<Cons*>(cell);

        if (is_marked(next))
            return;

        mrk(next) = !mrk(next);
        mark(car(next));
        cell = cdr(next);

    } while (is_pair(cell));

    if (!is_nil(cell))
        mark(cell);
}
}
