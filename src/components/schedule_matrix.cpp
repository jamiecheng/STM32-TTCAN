//
// Created by Jamie on 3-5-2021.
//

#include "schedule_matrix.h"
#include <cassert>

cni::ScheduleMatrix::ScheduleMatrix(uint8_t basic_cycles, uint8_t columns) :
    m_basic_cycles(basic_cycles),
    m_columns(columns)
{

}

void cni::ScheduleMatrix::set_column_duration(uint8_t column, uint16_t duration)
{
    assert(column < m_columns);
    m_column_durations[column] = duration;
}

uint16_t cni::ScheduleMatrix::get_column_duration(uint8_t column)
{
    assert(column < m_columns);
    return m_column_durations[column];
}

uint16_t cni::ScheduleMatrix::get_column_duration()
{
    return m_column_durations[m_cur_column];
}

uint8_t cni::ScheduleMatrix::step_column()
{
    uint8_t ret = 0;

    if(++m_cur_column == m_columns)
    {
        ret = 1;
        m_cur_column = 0;
    }

    return ret;
}

void cni::ScheduleMatrix::step_basix_cycle()
{
    if(++m_cur_basic_cycle == m_basic_cycles)
    {
        m_cur_basic_cycle = 0;
        m_cur_column = 0;
    }
}

uint16_t cni::ScheduleMatrix::get_column()
{
    return m_cur_column;
}

uint16_t cni::ScheduleMatrix::get_basic_cycle()
{
    return m_cur_basic_cycle;
}

uint32_t cni::ScheduleMatrix::get_basic_cycle_duration()
{
    uint32_t sum{};

    for(int i = 0; i < m_columns; i++)
        sum += m_column_durations[i];

    return sum;
}

