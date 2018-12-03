#include <iomanip>

#include "gc.hpp"
#include "scheme.hpp"

#define car get<0>
#define cdr get<1>
#define mrk get<2>

namespace pscm {

void GCollector::collect(Scheme& scm, const SymenvPtr& env)
{
    vset.clear();
    eset.clear();

    mark(env);

    vset.clear();
    eset.clear();

    size_t size = scm.store.size();

    scm.store.remove_if([](Cons& cons) {
        mrk(cons) = !mrk(cons);
        return mrk(cons);
    });
    if (logon) {
        size_t dlta = size - scm.store.size();
        std::cerr << "msg> garbage collector released " << dlta
                  << " cons-cells from " << size << " in total\n";
    }
}

void GCollector::logging(bool ok) { logon = ok; }

void GCollector::dump(const Scheme& scm, std::wostream& os)
{
    os << L"Store size: " << scm.store.size() << '\n';
    size_t ic = 0;
    for (auto& cons : scm.store) {
        os << ic++ << L" | mark: " << mrk(cons) << L" | "
           << std::left << std::setw(25)
           << car(cons) << " : " << cdr(cons) << '\n';
    }
}

bool GCollector::is_marked(const Cons& cons) const noexcept { return mrk(cons); }

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

void GCollector::mark(const VectorPtr& vec)
{
    auto [pos, ok] = vset.insert(vec.get());
    if (!ok)
        return;

    for (auto& cell : *vec) {
        mark(cell);
    }
}

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
