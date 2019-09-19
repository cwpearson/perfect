#include "perfect/cpu_turbo.hpp"

int main(void) {

    perfect::CpuTurboState state;
    perfect::get_cpu_turbo_state(&state);

    perfect::disable_cpu_turbo();

    // do things with CPU turbo disabled

    perfect::set_cpu_turbo_state(state);

}