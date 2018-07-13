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

namespace pscm {

/**
 * @brief Scheme vector class with shared underlying container type.
 */
template <typename T, typename Vec>
class SharedVector {
public:
    using value_type = T;
    using container_type = Vec;
    using iterator = typename Vec::iterator;
    using const_iterator = typename Vec::const_iterator;

    explicit SharedVector(size_t size = 0, const T& val = {})
        : pvec{ std::make_shared<Vec>(size, val) }
    {
    }

    explicit SharedVector(const Vec& vec)
        : pvec{ std::make_shared<Vec>(vec) }
    {
    }

    template <typename InputIt>
    SharedVector(InputIt& first, InputIt& last)
        : pvec{ std::make_shared<Vec>(first, last) }
    {
    }

    SharedVector(const SharedVector<T, Vec>&) = default;
    SharedVector(SharedVector<T, Vec>&&) = default;
    SharedVector<T, Vec>& operator=(const SharedVector<T, Vec>&) = default;
    SharedVector<T, Vec>& operator=(SharedVector<T, Vec>&&) = default;

    iterator begin() { return pvec->begin(); }

    const_iterator begin() const { return pvec->begin(); };

    const_iterator end() const { return pvec->end(); }

    bool operator!=(const SharedVector& vec) const noexcept
    {
        return pvec != vec.pvec;
    }

    //! Test for same vector.
    bool operator==(const SharedVector& vec) const noexcept
    {
        return pvec == vec.pvec;
    }

    //! Test for equivalent vectors with equal values.
    bool equal(const SharedVector& vec) const noexcept
    {
        return *this == vec || *pvec == *vec.pvec;
    }
    //! Return the vector size.
    size_t size() const noexcept { return pvec->size(); }

    /**
     * @brief Retrieve const reference to vector value at checked index position
     * @throws std::out_of_range for an out of vector boundary index position.
     */
    const T& at(size_t idx) const { return pvec->at(idx); }

    /**
     * @brief Retrieve non const reference to vector value at checked index position
     * @throws std::out_of_range for an out of vector boundary index position.
     */
    T& at(size_t idx) { return pvec->at(idx); };

    //! Append value copies from input iterator range.
    template <typename InputIt>
    void append(InputIt& first, InputIt& last)
    {
        std::copy(first, last, std::back_inserter(*pvec));
    }
    //! Append a single value.
    void append(const T& val) { pvec->push_back(val); }

    //! Fill whole vector with values or optional a subrange only.
    void fill(const T& val, const std::pair<size_t, size_t>& range = { 0, ~0 })
    {
        const size_t size = std::min(range.second, pvec->size());
        range.first <= size || (throw std::invalid_argument("invalid vector range"), 0);

        std::fill(pvec->begin() + range.first, pvec->begin() + size, val);
    }
    /**
     * @brief Copy values from input iterator range into the vector,
     *        starting at optional offset position.
     *
     * @throws std::invalid_argument for an out of vector boundary range.
     */
    template <typename InputIt>
    void copy(InputIt& first, InputIt& last, size_t offset = 0)
    {
        offset + std::distance(first, last) <= pvec->size()
            || (throw std::invalid_argument("invalid vector offset"), 0);

        std::copy(first, last, pvec->begin() + offset);
    }

private:
    std::shared_ptr<Vec> pvec;
};

}; // namespace pscm;

#endif // SVECTOR_HPP
