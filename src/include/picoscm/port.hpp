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

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <variant>

namespace pscm {

struct Cell;
enum class Intern;

/**
 * Stream manipulator type, to switch stream for scheme (display <expr>) output.
 */
template <typename T>
struct DisplayManip {
    const T& value;
};

template <typename T>
auto display(const T& val) { return DisplayManip<T>{ val }; }

/**
 * Output stream operator for scheme (display <expr>) output.
 */
std::ostream& operator<<(std::ostream& os, DisplayManip<Cell> cell);

/**
 * Output stream operator for scheme (write <expr>) output.
 */
std::ostream& operator<<(std::ostream& os, const Cell& cell);

/**
 * Output stream operator to write essential opcodes
 * as scheme symbol names.
 */
std::ostream& operator<<(std::ostream& os, Intern opcode);

/**
 * Scheme io-port fascade to represent either an std::iostream,
 * std::fstream or std::stringstream port.
 */
template <typename Char>
class Port {
    using stream_type = std::basic_iostream<Char, std::char_traits<Char>>;

public:
    using openmode = std::ios_base::openmode;

    virtual void close() = 0;
    virtual stream_type& getStream() = 0;

    virtual bool isOpen() = 0;
    virtual bool isInput() = 0;
    virtual bool isOutput() = 0;
    virtual bool isBinary() = 0;
    virtual bool isStandardPort() = 0;
    virtual bool isFilePort() = 0;
    virtual bool isStringPort() = 0;
};

template <typename Char>
class StandardPort : public Port<Char> {
    using stream_type = std::basic_iostream<Char, std::char_traits<Char>>;
    using openmode = typename Port<Char>::openmode;

public:
    explicit StandardPort(openmode mode /*= std::ios_base::in | std::ios_base::out*/)
        : stream{ std::cout.rdbuf() }
        , mode{ mode }
    {
    }

    void close() final
    {
        stream.flush().clear();
        stream.setstate(std::ios_base::eofbit);
    }

    stream_type& getStream() final { return stream; }
    bool isOpen() final { return stream.good(); }
    bool isInput() final { return mode & std::ios_base::in; }
    bool isOutput() final { return mode & std::ios_base::out; }
    bool isBinary() final { return mode & std::ios_base::binary; }
    bool isStandardPort() final { return true; }
    bool isFilePort() final { return false; }
    bool isStringPort() final { return false; }

private:
    stream_type stream;
    openmode mode;
};

template <typename Char>
class FilePort : public Port<Char> {
    using stream_type = std::basic_fstream<Char, std::char_traits<Char>>;
    using openmode = typename Port<Char>::openmode;

public:
    explicit FilePort(const std::string& filename, openmode mode)
        : stream{ filename, mode }
        , mode{ mode }
    {
        stream.exceptions(std::ios_base::failbit | std::ios_base::eofbit | std::ios_base::badbit);
    }
    void close() final
    {
        if (stream.is_open()) {
            stream.flush().clear();
            stream.close();
        }
    }
    stream_type& getStream() final
    {
        isOpen() || ((void)(throw std::invalid_argument("port is closed")), 0);
        return stream;
    }

    bool isOpen() final { return stream.is_open(); }
    bool isInput() final { return mode & std::ios_base::in; }
    bool isOutput() final { return mode & std::ios_base::out; }
    bool isBinary() final { return mode & std::ios_base::binary; }
    bool isStandardPort() final { return false; }
    bool isFilePort() final { return true; }
    bool isStringPort() final { return false; }

private:
    stream_type stream;
    openmode mode;
};

template <typename Char>
class StringPort : public Port<Char> {
    using stream_type = std::basic_stringstream<Char, std::char_traits<Char>>;
    using string_type = std::basic_string<Char, std::char_traits<Char>>;
    using openmode = typename Port<Char>::openmode;

public:
    explicit StringPort(openmode mode /*= std::ios_base::in | std::ios_base::out*/)
        : stream{ mode }
        , mode{ mode }
    {
    }
    explicit StringPort(const string_type& str,
        openmode mode /*= std::ios_base::in | std::ios_base::out*/)
        : stream{ str, mode }
        , mode{ mode }
    {
    }
    void close() final
    {
        stream.flush().clear();
        stream.setstate(std::ios_base::eofbit);
    }
    stream_type& getStream() final { return stream; }

    bool isOpen() final { return stream.good(); }
    bool isInput() final { return mode & std::ios_base::in; }
    bool isOutput() final { return mode & std::ios_base::out; }
    bool isBinary() final { return mode & std::ios_base::binary; }
    bool isStandardPort() final { return false; }
    bool isFilePort() final { return false; }
    bool isStringPort() final { return true; }

private:
    stream_type stream;
    openmode mode;
};

template <typename Char, typename T>
Port<Char>& operator<<(Port<Char>& port, const T& x)
{
    (port.isOpen() && port.isOutput())
        || ((void)(throw std::invalid_argument("port is closed or not an output port")), 0);

    port.getStream() << x;
    return port;
}

template <typename Char, typename T>
Port<Char>& operator>>(Port<Char>& port, T& x)
{
    (port.isOpen() && port.isInput())
        || ((void)(throw std::invalid_argument("port is closed or not an input port")), 0);

    port.getStream() >> x;
    return port;
}

} // namespace pscm
#endif // STREAM_HPP
