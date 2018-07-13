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

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <variant>

namespace pscm {

struct Cell;

std::ostream& operator<<(std::ostream& os, const Cell& cell);

class Port {
public:
    Port();

    bool is_output() const noexcept;
    bool is_input() const noexcept;
    bool is_strport() const noexcept;
    bool is_fileport() const noexcept;
    bool is_open() const noexcept;

    std::iostream& stream();

    bool open(const std::filesystem::path& path, std::ios_base::openmode mode = std::ios_base::out | std::ios_base::in);

    bool open_str(const std::string& str = {}, std::ios_base::openmode mode = std::ios_base::out | std::ios_base::in);

    void close();

    std::string str() const;

    template <typename T>
    explicit operator T&() { return std::get<T>(*pstream); }

    bool operator!=(const Port& stream) const noexcept;
    bool operator==(const Port& stream) const noexcept;

private:
    using stream_variant = std::variant<std::iostream, std::fstream, std::stringstream>;

    template <typename T>
    bool is_type() const
    {
        return std::holds_alternative<T>(*pstream);
    }
    std::shared_ptr<stream_variant> pstream;
};
}; // namespace pscm
#endif // STREAM_HPP
