#pragma once
#include <chrono>
#include <unordered_map>
#include <string>

namespace epi {
    class timer {
        static std::unordered_map<const char*, std::chrono::duration<double>>& TimeTable() {
            static std::unordered_map<const char*, std::chrono::duration<double>> s_time_map;
            return s_time_map;
        };
    public:
        static void clearTimers() {
            TimeTable().clear();
        };

        class scope {
            std::chrono::time_point<std::chrono::steady_clock> m_start;
            const char* m_name;
        public:
            scope(const char* name_) : m_name(name_) {
                m_start = std::chrono::steady_clock::now();
            }
            ~scope() {
                auto stop = std::chrono::steady_clock::now();
                timer::TimeTable()[m_name] += stop - m_start;
            }
        };
        
        class Get {
            const char* cur_name;
        public:
            double sec() {
                if(timer::TimeTable().contains(cur_name))
                    return std::chrono::duration_cast<std::chrono::seconds>(timer::TimeTable().at(cur_name)).count();
                else
                    return -1;
            }
            double ms() {
                if(timer::TimeTable().contains(cur_name))
                    return std::chrono::duration_cast<std::chrono::milliseconds>(timer::TimeTable().at(cur_name)).count();
                else
                    return -1;
            }
            double um() {
                if(timer::TimeTable().contains(cur_name))
                    return std::chrono::duration_cast<std::chrono::microseconds>(timer::TimeTable().at(cur_name)).count();
                else
                    return -1;
            }
            Get(const char* name_) : cur_name(name_) {}
        };
        static void Reset(const char* name) {
            TimeTable()[name] = std::chrono::duration<double>();
        }

    };

};
