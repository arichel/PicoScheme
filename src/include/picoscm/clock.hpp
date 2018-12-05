#ifndef CLOCK_HPP
#define CLOCK_HPP

#include <chrono>
#include <string>

namespace pscm {

using std::chrono::steady_clock;

class Clock {
public:
    //! The constructor acts like 'tic':
    Clock() { startTime = steady_clock::now(); }

    //! Reset clock:
    void tic();

    //! Returns time since previous tic or object creation in nanoseconds.
    //! Takes into account any accumulated time via pause/resume.
    double toc() const;

    //! Pause the clock:
    void pause();

    //! Resume the clock:
    void resume();

private:
    steady_clock::time_point startTime;
    double accumTime = 0.;
    bool isPaused = false;
};

//! Output stream the elapsed time since previous tic/toc.
template <typename OStream>
OStream& operator<<(OStream& os, const Clock& clock)
{
    auto elapsedTime = clock.toc();

    // If in the nanoseconds range:
    if (elapsedTime < 1000)
        return os << elapsedTime << " ns";

    // If in the microseconds range:
    if ((elapsedTime /= 1000.) < 1000)
        return os << elapsedTime << " us";

    // If in the milliseconds range:
    if ((elapsedTime /= 1000.) < 1000)
        return os << elapsedTime << " ms";

    // Otherwise we are in the seconds range:
    return os << (elapsedTime /= 1000.) << " s";
}

} // namespace pscm
#endif // CLOCK_HPP
