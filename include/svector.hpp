/********************************************************************************/ /**
 * @file svector.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#ifndef SVECTOR_HPP
#define SVECTOR_HPP

#include <algorithm>
#include <memory>
#include <vector>

namespace pscm {

template <typename T, typename Vec = std::vector<T>>
class SharedVector {
public:
    using value_type = T;
    using container_type = Vec;
    using iterator = typename Vec::iterator;
    using const_iterator = typename Vec::const_iterator;

    SharedVector()
        : pvec{ new Vec }
    {
    }

    SharedVector(size_t size, const T& val)
        : pvec{ new Vec{ size, val } }
    {
    }

    SharedVector(const Vec& vec)
        : pvec{ new Vec{ vec } }
    {
    }

    template <typename InputIt>
    SharedVector(InputIt& first, InputIt& last)
        : pvec{ new Vec{ first, last } }
    {
    }

    SharedVector(const SharedVector<T, Vec>&) = default;
    SharedVector(SharedVector<T, Vec>&&) = default;
    SharedVector<T, Vec>& operator=(const SharedVector<T, Vec>&) = default;
    SharedVector<T, Vec>& operator=(SharedVector<T, Vec>&&) = default;

    iterator begin() { return pvec->begin(); }
    const_iterator begin() const { return pvec->begin(); };
    const_iterator end() const { return pvec->end(); }

    bool operator!=(const SharedVector<T>& vec) const noexcept
    {
        return pvec != vec.pvec;
    }

    bool operator==(const SharedVector<T>& vec) const noexcept
    {
        return pvec == vec.pvec;
    }

    bool equal(const SharedVector<T>& vec) const noexcept
    {
        return *this == vec || *pvec == *vec.pvec;
    }
    template <typename InputIt>
    void copy(size_t pos, InputIt& first, InputIt& last)
    {
        pos + std::distance(first, last) <= pvec->size()
            || (throw std::invalid_argument("invalid vector index position"), 0);

        std::copy(first, last, pvec->begin() + pos);
    }

    template <typename InputIt>
    void append(InputIt& first, InputIt& last)
    {
        std::copy(first, last, std::back_inserter(*pvec));
    }

    void push_back(const T& val)
    {
        pvec->push_back(val);
    }

    void fill(const T& val, size_t pos = 0, size_t end = ~0)
    {
        const size_t size = std::min(end, pvec->size());
        pos <= size || (throw std::invalid_argument("invalid vector position"), 0);

        std::fill(pvec->begin() + pos, pvec->begin() + size, val);
    }
    size_t size() const noexcept { return pvec->size(); }

    const T& at(size_t idx) const { return pvec->at(idx); }
    T& at(size_t idx) { return pvec->at(idx); };

private:
    std::shared_ptr<Vec> pvec;
};

}; // namespace pscm;

#endif // SVECTOR_HPP
