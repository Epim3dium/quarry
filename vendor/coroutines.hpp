#include <chrono>

namespace epi {
    struct Coroutine
    {
        int line;
        std::chrono::time_point<std::chrono::steady_clock> start_time;
        bool timer_running = false;
    };

#define EPI_CoroutineBegin(coro) switch (coro->line) {case 0: coro->line = 0;
#define EPI_CoroutineYield(coro) do { coro->line = __LINE__; return; case __LINE__:;} while(0)
#define EPI_CoroutineYieldUntil(coro, condition) while (!(condition)) { EPI_CoroutineYield(coro); }

#define EPI_CoroutineWait(coro, duration)\
    do {\
        if (!coro->timer_running) {\
            coro->start_time = std::chrono::steady_clock::now(); coro->timer_running = true;\
        }\
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - coro->start_time).count();\
        EPI_CoroutineYieldUntil(coro, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - coro->start_time).count() > duration * 1000.0); coro->timer_running = false; }while (0)
#define EPI_CoroutineExit(coro) do { coro->line = __LINE__; } while (0); }
#define EPI_CoroutineReset(coro) do { coro->line = 0; } while (0); }
}
