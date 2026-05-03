#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FastAccelStepper.h>
#include <driver/pcnt.h>
#include <esp_err.h>

#include <cstdint>
#include <new>

#include "Service/Log.h"

// Temporary pulse timing diagnostics. Remove this file and the IStepper pulse
// trace plumbing when the motor timing investigation is done.
class StepperPulseTrace {
public:
    struct Sample {
        int32_t pcnt = 0;
        uint32_t stepDtUs = 0;
    };

    ~StepperPulseTrace() {
        detachInterrupt();
        delete[] samples_;
    }

    bool configureAndSetup(FastAccelStepper* stepper, const char* motorName, JsonObjectConst configObj) {
        autoTrace_ = configObj["Trace"] | 0;
        autoTrace_ = autoTrace_ || (configObj["TraceAuto"] | 0);
        preferredPcntUnit_ = configObj["TracePcnt"] | 0;
        int32_t sampleCapacity = configObj["TraceSamples"] | 512;
        if (sampleCapacity < 1) sampleCapacity = 1;
        if (sampleCapacity > 4096) sampleCapacity = 4096;
        configuredSampleCapacity_ = static_cast<uint16_t>(sampleCapacity);
        return setupConfigured(stepper, motorName);
    }

    void beginAuto(FastAccelStepper* stepper, const char* motorName) {
        if (!autoTrace_ || stepper == nullptr) return;
        if (!attached_) setupConfigured(stepper, motorName);
        if (attached_ && start(true)) {
            moveActive_ = true;
        }
    }

    void finishAutoNow() {
        if (!moveActive_) return;
        stop(true);
        moveActive_ = false;
    }

    void finishAutoIfBlocking(bool blocking) {
        if (blocking) finishAutoNow();
    }

    void process(bool motorRunning) {
        if (active_) sample();
        if (moveActive_ && !motorRunning) finishAutoNow();
    }

    bool attach(FastAccelStepper* stepper,
                const char* motorName,
                int preferredPcntUnit = 0,
                uint16_t sampleCapacity = 512,
                int16_t lowValue = -16384,
                int16_t highValue = 16384) {
        if (attached_) return true;
        if (stepper == nullptr) return false;

        int unit = preferredPcntUnit;
        if (unit < 0) {
            unit = 0;
            Log::L("[StepperTrace] %s: TracePcnt not set, fallback to PCNT=%d",
                   safeName(motorName),
                   unit);
        }
        if (unit >= kMaxPcntUnits) {
            Log::L("[StepperTrace] %s: invalid PCNT unit=%d", safeName(motorName), unit);
            return false;
        }

        const uint16_t capacity = sampleCapacity > 0 ? sampleCapacity : 512;
        Sample* buffer = new (std::nothrow) Sample[capacity];
        if (buffer == nullptr) {
            Log::L("[StepperTrace] %s: sample buffer allocation failed", safeName(motorName));
            return false;
        }

        if (!stepper->attachToPulseCounter(static_cast<uint8_t>(unit), lowValue, highValue)) {
            delete[] buffer;
            Log::L("[StepperTrace] %s: attachToPulseCounter(%d) failed", safeName(motorName), unit);
            return false;
        }

        if (!attachInterrupt(static_cast<pcnt_unit_t>(unit))) {
            delete[] buffer;
            Log::L("[StepperTrace] %s: PCNT ISR attach failed unit=%d", safeName(motorName), unit);
            return false;
        }

        delete[] samples_;
        samples_ = buffer;
        capacity_ = capacity;
        stepper_ = stepper;
        motorName_ = motorName;
        pcntUnit_ = unit;
        attached_ = true;
        active_ = false;
        resetBuffer();

        Log::L("[StepperTrace] %s: attached PCNT=%d samples=%u",
               safeName(motorName_),
               pcntUnit_,
               static_cast<unsigned>(capacity_));
        return true;
    }

    bool start(bool clearCounter = true) {
        if (!attached_ || stepper_ == nullptr) return false;
        active_ = false;
        if (clearCounter) stepper_->clearPulseCounter();
        resetBuffer();
        cumulativePcnt_ = 0;
        lastPulseUs_ = 0;
        pcnt_counter_clear(static_cast<pcnt_unit_t>(pcntUnit_));
        pcnt_counter_resume(static_cast<pcnt_unit_t>(pcntUnit_));
        active_ = true;
        return true;
    }

    void sample() {
    }

    void stop(bool printCsv = true) {
        if (!active_) return;
        active_ = false;
        if (printCsv) print();
    }

    void stopMoveTrace(bool printCsv = true) {
        stop(printCsv);
        moveActive_ = false;
    }

    void print() const {
        if (!attached_) return;

        Serial.printf("# stepper_trace name=%s pcnt=%d samples=%u overflow=%u\r\n",
                      safeName(motorName_),
                      pcntUnit_,
                      static_cast<unsigned>(count_),
                      static_cast<unsigned>(overflow_));
        Serial.println("pcnt,dt");
        for (uint16_t i = 0; i < count_; ++i) {
            const Sample& s = samples_[i];
            Serial.printf("%ld,%.6f\r\n",
                          static_cast<long>(s.pcnt),
                          static_cast<double>(s.stepDtUs) / 1000000.0);
        }
        Serial.println("# stepper_trace_end");
    }

    bool isAttached() const { return attached_; }
    bool isActive() const { return active_; }
    int pcntUnit() const { return pcntUnit_; }
    uint16_t count() const { return count_; }
    uint16_t overflow() const { return overflow_; }

private:
    static constexpr int kMaxPcntUnits = 8;

    static void pcntIsr(void* arg) {
        static_cast<StepperPulseTrace*>(arg)->handlePcntIsr();
    }

    static const char* safeName(const char* name) {
        return (name != nullptr && name[0] != '\0') ? name : "?";
    }

    bool setupConfigured(FastAccelStepper* stepper, const char* motorName) {
        if (!autoTrace_) return false;
        return attach(stepper, motorName, preferredPcntUnit_, configuredSampleCapacity_);
    }

    void resetBuffer() {
        count_ = 0;
        overflow_ = 0;
    }

    bool attachInterrupt(pcnt_unit_t unit) {
        esp_err_t rc = pcnt_isr_service_install(0);
        if (rc != ESP_OK && rc != ESP_ERR_INVALID_STATE) return false;

        pcnt_intr_disable(unit);
        pcnt_set_event_value(unit, PCNT_EVT_THRES_1, 1);
        pcnt_set_event_value(unit, PCNT_EVT_THRES_0, -1);
        pcnt_event_enable(unit, PCNT_EVT_THRES_1);
        pcnt_event_enable(unit, PCNT_EVT_THRES_0);
        pcnt_counter_clear(unit);

        rc = pcnt_isr_handler_add(unit, pcntIsr, this);
        if (rc != ESP_OK) return false;

        pcnt_intr_enable(unit);
        isrAttached_ = true;
        return true;
    }

    void detachInterrupt() {
        if (!isrAttached_ || pcntUnit_ < 0) return;
        pcnt_intr_disable(static_cast<pcnt_unit_t>(pcntUnit_));
        pcnt_isr_handler_remove(static_cast<pcnt_unit_t>(pcntUnit_));
        isrAttached_ = false;
    }

    void handlePcntIsr() {
        if (pcntUnit_ < 0) return;

        const pcnt_unit_t unit = static_cast<pcnt_unit_t>(pcntUnit_);
        uint32_t status = 0;
        int16_t raw = 0;
        pcnt_get_event_status(unit, &status);
        pcnt_get_counter_value(unit, &raw);
        pcnt_counter_clear(unit);

        if (!active_) return;

        if (raw == 0) {
            if ((status & PCNT_EVT_THRES_1) != 0) raw = 1;
            else if ((status & PCNT_EVT_THRES_0) != 0) raw = -1;
            else return;
        }

        cumulativePcnt_ += raw;
        const uint32_t now = micros();
        const uint32_t dt = (lastPulseUs_ == 0) ? 0 : now - lastPulseUs_;
        lastPulseUs_ = now;

        if (count_ >= capacity_) {
            if (overflow_ < UINT16_MAX) ++overflow_;
            return;
        }

        Sample& s = samples_[count_++];
        s.pcnt = cumulativePcnt_;
        s.stepDtUs = dt;
    }

    FastAccelStepper* stepper_ = nullptr;
    const char* motorName_ = nullptr;
    Sample* samples_ = nullptr;
    volatile uint16_t capacity_ = 0;
    volatile uint16_t count_ = 0;
    volatile uint16_t overflow_ = 0;
    volatile uint32_t lastPulseUs_ = 0;
    volatile int32_t cumulativePcnt_ = 0;
    int pcntUnit_ = -1;
    int preferredPcntUnit_ = 0;
    uint16_t configuredSampleCapacity_ = 512;
    bool attached_ = false;
    bool isrAttached_ = false;
    bool autoTrace_ = false;
    bool moveActive_ = false;
    volatile bool active_ = false;
};
