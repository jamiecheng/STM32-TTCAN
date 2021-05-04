//
// Created by Jamie on 3-5-2021.
//

#ifndef TTCAN_SCHEDULE_MATRIX_H
#define TTCAN_SCHEDULE_MATRIX_H

#include <cstdint>

namespace cni
{
class ScheduleMatrix
{
    static const uint8_t MAX_COLUMNS = 64;

public:
    ScheduleMatrix(uint8_t basic_cycles, uint8_t columns);

    void set_column_duration(uint8_t column, uint16_t duration);

    uint16_t get_column_duration(uint8_t column);

    uint16_t get_column_duration();

    uint8_t step_column();

    void step_basix_cycle();

    uint16_t get_column();

    uint16_t get_basic_cycle();

    uint32_t get_basic_cycle_duration();

private:
    uint16_t m_column_durations[MAX_COLUMNS]{};
    const uint8_t m_basic_cycles{};
    const uint8_t m_columns{};

    uint16_t m_cur_basic_cycle{};
    uint16_t m_cur_column{};
};
}

#endif //TTCAN_SCHEDULE_MATRIX_H
