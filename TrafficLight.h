#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <windows.h>
#include "Vehicle.h"

enum LightColor {
    LIGHT_RED = 0,
    LIGHT_YELLOW = 1,
    LIGHT_GREEN = 2
};

enum SignalPhase {
    PHASE_NS_GREEN = 0,
    PHASE_NS_YELLOW = 1,
    PHASE_ALL_RED_1 = 2,
    PHASE_EW_GREEN = 3,
    PHASE_EW_YELLOW = 4,
    PHASE_ALL_RED_2 = 5,
    PHASE_EMERGENCY = 6
};

class TrafficLight {
private:
    LightColor currentColor;
    Direction direction;
    int posX;
    int posY;

public:
    TrafficLight();
    TrafficLight(Direction dir, int x, int y);

    void setColor(LightColor c);
    LightColor getColor() const;
    Direction getDirection() const;
    void setPosition(int x, int y);
    void draw(HDC hdc) const;
};

class TrafficLightController {
private:
    SignalPhase currentPhase;
    SignalPhase previousPhase;
    float timer;
    float greenDuration;
    float yellowDuration;
    float allRedDuration;
    TrafficLight lights[4];
    bool emergencyActive;
    Direction emergencyLane;
    bool manualMode;

    void applyPhaseLights();

public:
    TrafficLightController();

    void initialize(int cx, int cy, int roadW);
    void update(float dt);
    void triggerEmergency(Direction lane);
    void clearEmergency();
    void toggleManualMode();
    void cycleManual();

    SignalPhase getPhase() const;
    float getRemainingTime() const;
    bool isEmergencyActive() const;
    Direction getEmergencyDirection() const;
    bool isManual() const;
    LightColor getLightColor(Direction dir) const;
    void draw(HDC hdc) const;
};

#endif
