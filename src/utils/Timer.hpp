#pragma once
/**
 * @file   Timer.hpp
 * @brief  Мини-набор высокоточных таймеров и счётчиков FPS.
 *
 *  ✔ Header-only  — не требует отдельной линковки.
 *  ✔ std::chrono  — кросс-платформенно, наносекундная точность.
 *  ✔ Zero-overhead: все методы inline, компилятор оптимизирует вызовы.
 *
 * © 2025 YourCompany — MIT License.
 */
#include <chrono>
#include <cstdint>
#include <atomic>
#include <string>
#include <iostream>
#include <iomanip>

namespace qrslam::util {

//--------------------------------------------------------------
// StopWatch — ручной старт / стоп
//--------------------------------------------------------------
class StopWatch {
public:
    StopWatch(bool start_now = true) {
        if (start_now)  _tp = clock_t::now();
    }

    /// перезапустить отсчёт
    inline void reset() { _tp = clock_t::now(); }

    /// прошедшее время, сек.
    [[nodiscard]] inline double elapsed() const {
        using namespace std::chrono;
        return duration_cast<duration<double>>(clock_t::now() - _tp).count();
    }

    /// удобно для `printf("dt=%.3f\n", sw.lap())`;
    inline double lap() { double e = elapsed(); reset(); return e; }

private:
    using clock_t = std::chrono::steady_clock;
    clock_t::time_point _tp = clock_t::now();
};

//--------------------------------------------------------------
// ScopedTimer — RAII-таймер участка кода
//--------------------------------------------------------------
class ScopedTimer {
public:
    ScopedTimer(std::string name,
                bool auto_print = true,
                std::ostream& os = std::cerr)
        : name_{std::move(name)}, os_{os}, print_{auto_print}, sw_{true} {}

    ~ScopedTimer() {
        if (print_) {
            os_ << std::fixed << std::setprecision(3)
                << "[TIMER] " << name_ << " = "
                << sw_.elapsed()*1e3 << " ms\n";
        }
    }

    /// вручную отключить вывод
    void cancel() { print_ = false; }

private:
    std::string name_;
    std::ostream& os_;
    bool print_;
    StopWatch sw_;
};

//--------------------------------------------------------------
// FpsMeter — сглаженный счётчик FPS
//--------------------------------------------------------------
class FpsMeter {
public:
    explicit FpsMeter(double alpha = 0.9) : alpha_{alpha} {}

    /// вызвать на каждом кадре; возвращает текущий FPS
    inline double tick() {
        double now = nowSec();
        if (_prev < 0.0) { _prev = now; return 0.0; }

        double dt = now - _prev;
        _prev = now;
        if (dt <= 0.0) return _fps; // защита от деления на ноль

        double inst = 1.0 / dt;
        _fps = (_fps < 1e-3) ? inst               // первичная инициализация
                             : alpha_ * _fps + (1.0 - alpha_) * inst;
        return _fps;
    }

    [[nodiscard]] inline double fps() const { return _fps; }

private:
    static inline double nowSec() {
        using namespace std::chrono;
        return duration_cast<duration<double>>(steady_clock::now().time_since_epoch()).count();
    }

    double alpha_;
    double _prev = -1.0;
    double _fps  = 0.0;
};

//--------------------------------------------------------------
// Thread-safe FPS counter (для мультипоточных пайплайнов)
//--------------------------------------------------------------
class AtomicFpsMeter {
public:
    explicit AtomicFpsMeter(double alpha = 0.9) : alpha_{alpha} {}

    /** вызывать из любого потока,
     *  возвращает сглаженный FPS (окно `alpha`). */
    double tick() {
        double now = nowSec();
        double prev = _prev.exchange(now);
        if (prev < 0.0)  return _fps.load();

        double dt = now - prev;
        if (dt <= 0.0)  return _fps.load();
        double inst = 1.0 / dt;

        // exp-moving-average без блокировок
        double old_fps = _fps.load();
        double new_fps;
        do {
            new_fps = (old_fps < 1e-3) ? inst
                                       : alpha_ * old_fps + (1.0 - alpha_) * inst;
        } while (!_fps.compare_exchange_weak(old_fps, new_fps,
                                             std::memory_order_relaxed,
                                             std::memory_order_relaxed));
        return new_fps;
    }

    [[nodiscard]] double fps() const { return _fps.load(); }

private:
    static inline double nowSec() {
        using namespace std::chrono;
        return duration_cast<duration<double>>(steady_clock::now().time_since_epoch()).count();
    }

    double alpha_;
    std::atomic<double> _prev{-1.0};
    std::atomic<double> _fps {0.0};
};

} // namespace qrslam::util
