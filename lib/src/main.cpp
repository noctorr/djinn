#include <csignal>
#include <cstdlib>

static volatile std::sig_atomic_t running = 1;

void signalInterruption(int)
{
    running = 0;
}

int main(void)
{

    return EXIT_SUCCESS;
}