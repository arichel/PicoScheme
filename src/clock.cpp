
#include <iostream>

#include "clock.hpp"

namespace pscm {

void Clock::tic()
{
    accumTime = 0;
    isPaused = false;
    startTime = steady_clock::now();
}

double Clock::toc() const
{
    using namespace std::chrono;

    if (!isPaused)
        return accumTime + duration_cast<nanoseconds>(steady_clock::now() - startTime).count();

    return accumTime;
}

void Clock::pause()
{
    using namespace std::chrono;

    if (!isPaused) {
        accumTime += duration_cast<nanoseconds>(steady_clock::now() - startTime).count();
        isPaused = true;
    }
}

void Clock::resume()
{
    if (isPaused) {
        isPaused = false;
        startTime = steady_clock::now();
    }
}

} // namespace pscm
