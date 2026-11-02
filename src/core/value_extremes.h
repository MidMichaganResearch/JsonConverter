#pragma once

#include <string>

namespace plotconv {

/// Tracks the smallest and largest value seen, together with the exact raw
/// token each one came from, so the JSON can quote them without any rounding.
class ValueExtremes
{
    public:
        void add(double value, const std::string& token)
        {
            if (!hasValues_)
            {
                min_ = max_ = value;
                minToken_ = maxToken_ = token;
                hasValues_ = true;
                return;
            }

            if (value < min_)
            {
                min_ = value;
                minToken_ = token;
            }
            if (value > max_)
            {
                max_ = value;
                maxToken_ = token;
            }
        }

        void merge(const ValueExtremes& other)
        {
            if (!other.hasValues_)
            {
                return;
            }
            add(other.min_, other.minToken_);
            add(other.max_, other.maxToken_);
        }

        bool empty() const { return !hasValues_; }

        double min() const { return min_; }
        double max() const { return max_; }
        const std::string& minToken() const { return minToken_; }
        const std::string& maxToken() const { return maxToken_; }

    private:
        bool hasValues_ = false;
        double min_ = 0.0;
        double max_ = 0.0;
        std::string minToken_;
        std::string maxToken_;
    };

}
