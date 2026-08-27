#include <csignal>

// just testing it out
volatile std::sig_atomic_t running = 1;

void signalInterruption(int)
{
    running = 0;
}

int main() {

    return 0;
}