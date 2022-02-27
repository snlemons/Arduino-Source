/*  Stat Accumulator
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_StatAccumulator_H
#define PokemonAutomation_CommonFramework_StatAccumulator_H

#include <stdint.h>
#include <string>

namespace PokemonAutomation{

class LoggerQt;


class StatAccumulatorI32{
public:
    using StatObject = double;

public:
    void operator+=(uint32_t x);

    uint64_t count() const{ return m_count; };
    uint32_t min() const{ return m_min; }
    uint32_t max() const{ return m_max; }
    double mean() const;
    double stddev() const;

    std::string dump(const char* units, double divider) const;
    void log(LoggerQt& logger, const std::string& label, const char* units, double divider) const;

private:
    uint64_t m_count = 0;
    uint64_t m_sum = 0;
    uint64_t m_sqr = 0;
    uint32_t m_min = (uint32_t)-1;
    uint32_t m_max = 0;
};



class FloatStatAccumulator{
public:
    using StatObject = double;

public:
    FloatStatAccumulator();

    void operator+=(double x);

    uint64_t count() const{ return m_count; };
    double min() const{ return m_min; }
    double max() const{ return m_max; }
    double mean() const;
    double stddev() const;
    double diff_metric(double reference) const;

private:
    uint64_t m_count;
    double m_sum;
    double m_sqr;
    double m_min;
    double m_max;
};


}
#endif
