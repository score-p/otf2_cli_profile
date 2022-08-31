/*
 This is part of the OTF-Profiler. Copyright by ZIH, TU Dresden 2016-2018.
 Authors: Maximillian Neumann, Denis Hünich, Jens Doleschal
*/

#ifndef MAIN_STRUCTS_H
#define MAIN_STRUCTS_H

#include <cassert>
#include <map>
#include <unordered_map>
#include <cstdint>
#include <limits>

enum class MetricDataType : uint8_t {
    UINT64,  // OTF2_TYPE_UINT64
    INT64,   // OTF2_TYPE_INT64
    DOUBLE   // OTF2_TYPE_DOUBLE
};

enum class MetricMode : uint8_t {
    ACCUMULATED_START,
    ACCUMULATED_POINT,
    ACCUMULATED_LAST,
    ACCUMULATED_NEXT,
    ABSOLUTE_POINT,
    ABSOLUTE_LAST,
    ABSOLUTE_NEXT,
    RELATIVE_POINT,
    RELATIVE_LAST,
    RELATIVE_NEXT
};

enum class MetricType : uint8_t {
    OTHER,
    PAPI,
    RUSAGE,
    USER
};

enum class MetricBase : uint8_t{
    BINARY,
    DECIMAL
};

enum class MetricOccurrence : uint8_t{
    SYNCHRONOUS_STRICT,
    SYNCHRONOUS,
    ASYNCHRONOUS
};

enum class RecorderKind : uint8_t{
    UNKNOWN,
    ABSTRACT,
    CPU,
    GPU
};

// TODO metrics funktionieren momentan NUR für strict sync + accumulated start
//  --> benötigt wird noch eine sinnvolle strategie wie man mit verschiedenen Arten von Metriken umgehen kann
//      --> problematisch dabei ist OTF da die Einschränkung im Sinne von Synchronität etc. nicht gibt
//  --> die metric-callbacks sind Fehlerintollerant was oft zu segmentation faults oder falschen Daten führen kann
//  --> für nicht synchrone metrics w#re ex möglich zusätzliche Conatainer zu erstellen => Nutzung abhängig von Ausgabe
//  (gleiches gilt für andere asynchrone Daten z.B. rma)

struct MetricData {
    MetricDataType type;

    union Data {
        uint64_t u;  //~ .unsigned_int in OTF2
        int64_t  s;  //~ .signed_int in OTF2
        double   d;  //~ .floating_point in OTF2

        Data() : u(0) {}

        Data(int64_t value) : s(value){};
        Data(uint64_t value) : u(value){};
        Data(double value) : d(value){};

        operator int64_t() const { return s; }
        operator uint64_t() const { return u; }
        operator double() const { return d; }

        int64_t  operator+(const int64_t rhs) { return s + rhs; }
        uint64_t operator+(const uint64_t rhs) { return u + rhs; }
        double   operator+(const double rhs) { return d + rhs; }

        int64_t& operator+=(const int64_t rhs) {
            s += rhs;
            return s;
        }
        uint64_t& operator+=(const uint64_t rhs) {
            u += rhs;
            return u;
        }
        double& operator+=(const double rhs) {
            d += rhs;
            return d;
        }

        int64_t  operator-(const int64_t rhs) { return s - rhs; }
        uint64_t operator-(const uint64_t rhs) { return u - rhs; }
        double   operator-(const double rhs) { return d - rhs; }

        int64_t& operator-=(const int64_t rhs) {
            s -= rhs;
            return s;
        }
        uint64_t& operator-=(const uint64_t rhs) {
            u -= rhs;
            return u;
        }
        double& operator-=(const double rhs) {
            d -= rhs;
            return d;
        }
    } data_incl, data_excl;

    MetricData() = default;

    void add_incl(const MetricData& rhs) {
        assert(type == rhs.type);
        switch (type) {
            case MetricDataType::UINT64:
                data_excl += (uint64_t)rhs.data_incl;
                break;

            case MetricDataType::INT64:
                data_excl += (int64_t)rhs.data_incl;
                break;

            case MetricDataType::DOUBLE:
                data_excl += (double)rhs.data_incl;
                break;
        }
    }
    // funktion um geziehlt incl. data von excl. data von parent abzuziehen
    void sub_incl(const MetricData& rhs) {
        assert(type == rhs.type);
        switch (type) {
            case MetricDataType::UINT64:
                data_excl -= (uint64_t)rhs.data_incl;
                break;

            case MetricDataType::INT64:
                data_excl -= (int64_t)rhs.data_incl;
                break;

            case MetricDataType::DOUBLE:
                data_excl -= (double)rhs.data_incl;
                break;
        }
    }

    MetricData& operator-=(const MetricData& rhs) {
        assert(type == rhs.type);
        switch (type) {
            case MetricDataType::UINT64:
                data_incl -= (uint64_t)rhs.data_incl;
                data_excl -= (uint64_t)rhs.data_excl;
                return *this;

            case MetricDataType::INT64:
                data_incl -= (int64_t)rhs.data_incl;
                data_excl -= (int64_t)rhs.data_excl;
                return *this;

            case MetricDataType::DOUBLE:
                data_incl -= (double)rhs.data_incl;
                data_excl -= (double)rhs.data_excl;
                return *this;
        }

        return *this;
    }

    MetricData& operator+=(const MetricData& rhs) {
        assert(type == rhs.type);
        switch (type) {
            case MetricDataType::UINT64:
                data_incl += (uint64_t)rhs.data_incl;
                data_excl += (uint64_t)rhs.data_excl;
                return *this;

            case MetricDataType::INT64:
                data_incl += (int64_t)rhs.data_incl;
                data_excl += (int64_t)rhs.data_excl;
                return *this;

            case MetricDataType::DOUBLE:
                data_incl += (double)rhs.data_incl;
                data_excl += (double)rhs.data_excl;
                return *this;
        }

        return *this;
    }
};

struct metric_class_data {
    // class id -> zahl der reihelfolge = "metric id"? -> nope wirkliche metric_id
    uint64_t                       class_id = 0;
    std::map<uint64_t, MetricData> met_data;

    metric_class_data() : class_id(0), met_data() {}

    metric_class_data(uint64_t _class_id) : class_id(_class_id) {}
};

template<typename T>
struct MinMaxSum {
    T sum = 0;
    T min = std::numeric_limits<T>::max();
    T max = 0;

    MinMaxSum(const T& value)
    : sum(value), min(value), max(value) {
    }

    MinMaxSum()
    : sum(), min(std::numeric_limits<T>::max()), max() {
    }

    MinMaxSum& operator+=(const T& value) {

        sum += value;
        if(value < min) {
            min = value;
        }
        if(value > max) {
            max = value;
        }

        return *this;
    }

    MinMaxSum& operator+=(const MinMaxSum& other) {

        sum += other.sum;
        if(other.min < min) {
            min = other.min;
        }
        if(other.max > max) {
            max = other.max;
        }

        return *this;
    }
};

struct FunctionData {
    uint64_t count;
    uint64_t incl_time;
    uint64_t excl_time;
};
#include <iostream>
struct FunctionDataStats {
    uint64_t count = 0;
    MinMaxSum<uint64_t> incl_time{};
    MinMaxSum<uint64_t> excl_time{};

    FunctionDataStats(const FunctionData& fdata)
    : count(fdata.count), incl_time(fdata.incl_time), excl_time(fdata.excl_time) {

        if(fdata.incl_time == 0) std::cout << "WTF" << std::endl;
    }

    FunctionDataStats()
    : count(), incl_time(), excl_time() {
    }

    FunctionDataStats& operator+=(const FunctionData& rhs) {
        count += rhs.count;
        incl_time += rhs.incl_time;
        excl_time += rhs.excl_time;

        return *this;
    }

    FunctionDataStats& operator+=(const FunctionDataStats& rhs) {
        count += rhs.count;
        incl_time += rhs.incl_time;
        excl_time += rhs.excl_time;

        return *this;
    }
};

// TODO MessageData und CollopData sind prinzipiell ziemlich ähnlich
// unterschiedlich können sie erst werden wenn eine Art Messagematching implementiert wird.
//  -> selbst dann kann ein struct beides abbilden sofern man die Matchinginformationen nicht anbindet
//  ==> eine Art MessageSummary-Container wäre denkbar in dem abgelegt wird welches Prozess mit wem wie viel
//  kommuniziert hat
//      -> für collop wäre der Aufwand höher (zumindest in OTF2) da die Kommunikatoren berücksichtigt werden müssen,
//      während man bei den P2P-Daten die Kommunikatoren ignorieren kann (so lang man nicht genau Kommunikatoren
//      basierende Daten angelegen will)

struct MessageData {
    uint64_t count_send;
    uint64_t count_recv;
    uint64_t bytes_send;
    uint64_t bytes_recv;

    MessageData& operator+=(const MessageData& rhs) {
        count_send += rhs.count_send;
        count_recv += rhs.count_recv;
        bytes_send += rhs.bytes_send;
        bytes_recv += rhs.bytes_recv;

        return *this;
    }
};

struct CollopData {
    uint64_t count_send;
    uint64_t count_recv;
    uint64_t bytes_send;
    uint64_t bytes_recv;

    CollopData& operator+=(const CollopData& rhs) {
        count_send += rhs.count_send;
        count_recv += rhs.count_recv;
        bytes_send += rhs.bytes_send;
        bytes_recv += rhs.bytes_recv;

        return *this;
    }
};
// TODO momentan ungenutzt --> sehr ähnlich zu message und collop
struct RmaData {
    uint64_t rma_put_cnt;
    uint64_t rma_get_cnt;
    uint64_t rma_put_bytes;
    uint64_t rma_get_bytes;

    RmaData& operator+=(const RmaData& rhs) {
        rma_put_cnt += rhs.rma_put_cnt;
        rma_get_cnt += rhs.rma_get_cnt;
        rma_put_bytes += rhs.rma_put_bytes;
        rma_get_bytes += rhs.rma_get_bytes;

        return *this;
    }
};

struct NodeData {
    FunctionDataStats f_data;
    MessageData       m_data;
    CollopData        c_data;

    std::map<uint64_t, MetricData> metrics;
    NodeData() : f_data(), m_data(), c_data() {}

    NodeData(const FunctionDataStats& _f_data) : f_data(_f_data), m_data(), c_data() {}

    NodeData(const MessageData& _m_data) : f_data(), m_data(_m_data), c_data() {}

    NodeData(const CollopData& _c_data) : f_data(), m_data(), c_data(_c_data) {}
};

struct IoData {
    uint64_t num_operations;
    uint64_t num_bytes;
    uint64_t transfer_time;
    uint64_t nontransfer_time;
    IoData() : num_operations(0), num_bytes(0), transfer_time(0), nontransfer_time(0) {}
};

#endif
