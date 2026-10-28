#include "TrafficLight.h"

TrafficLight::TrafficLight()
    : currentColor(LIGHT_RED), direction(DIR_NORTH), posX(0), posY(0) {
}

TrafficLight::TrafficLight(Direction dir, int x, int y)
    : currentColor(LIGHT_RED), direction(dir), posX(x), posY(y) {
}

void TrafficLight::setColor(LightColor c) {
    currentColor = c;
}

LightColor TrafficLight::getColor() const {
    return currentColor;
}

Direction TrafficLight::getDirection() const {
    return direction;
}

void TrafficLight::setPosition(int x, int y) {
    posX = x;
    posY = y;
}

void TrafficLight::draw(HDC hdc) const {
    int boxW = 22;
    int boxH = 58;

    HBRUSH housingBrush = CreateSolidBrush(RGB(24, 24, 28));
    HPEN borderPen = CreatePen(PS_SOLID, 2, RGB(70, 75, 80));
    HGDIOBJ oldBrush = SelectObject(hdc, housingBrush);
    HGDIOBJ oldPen = SelectObject(hdc, borderPen);

    RoundRect(hdc, posX - boxW / 2, posY - boxH / 2, posX + boxW / 2, posY + boxH / 2, 6, 6);

    HPEN nullPen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
    SelectObject(hdc, nullPen);

    COLORREF redCol = (currentColor == LIGHT_RED) ? RGB(255, 30, 30) : RGB(70, 15, 15);
    COLORREF yellowCol = (currentColor == LIGHT_YELLOW) ? RGB(255, 215, 20) : RGB(70, 55, 10);
    COLORREF greenCol = (currentColor == LIGHT_GREEN) ? RGB(30, 235, 30) : RGB(15, 65, 15);

    int radius = 6;
    int lampY1 = posY - 17;
    int lampY2 = posY;
    int lampY3 = posY + 17;

    HBRUSH redB = CreateSolidBrush(redCol);
    SelectObject(hdc, redB);
    Ellipse(hdc, posX - radius, lampY1 - radius, posX + radius, lampY1 + radius);
    DeleteObject(redB);

    HBRUSH yellowB = CreateSolidBrush(yellowCol);
    SelectObject(hdc, yellowB);
    Ellipse(hdc, posX - radius, lampY2 - radius, posX + radius, lampY2 + radius);
    DeleteObject(yellowB);

    HBRUSH greenB = CreateSolidBrush(greenCol);
    SelectObject(hdc, greenB);
    Ellipse(hdc, posX - radius, lampY3 - radius, posX + radius, lampY3 + radius);
    DeleteObject(greenB);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(housingBrush);
    DeleteObject(borderPen);
    DeleteObject(nullPen);
}

TrafficLightController::TrafficLightController()
    : currentPhase(PHASE_NS_GREEN), previousPhase(PHASE_NS_GREEN),
      timer(0.0f), greenDuration(5.5f), yellowDuration(1.6f), allRedDuration(0.9f),
      emergencyActive(false), emergencyLane(DIR_NORTH), manualMode(false) {
    applyPhaseLights();
}

void TrafficLightController::initialize(int cx, int cy, int roadW) {
    int halfW = roadW / 2;
    lights[DIR_NORTH] = TrafficLight(DIR_NORTH, cx - halfW - 22, cy - halfW - 32);
    lights[DIR_SOUTH] = TrafficLight(DIR_SOUTH, cx + halfW + 22, cy + halfW + 32);
    lights[DIR_EAST]  = TrafficLight(DIR_EAST,  cx + halfW + 32, cy - halfW - 22);
    lights[DIR_WEST]  = TrafficLight(DIR_WEST,  cx - halfW - 32, cy + halfW + 22);
    applyPhaseLights();
}

void TrafficLightController::applyPhaseLights() {
    if (currentPhase == PHASE_NS_GREEN) {
        lights[DIR_NORTH].setColor(LIGHT_GREEN);
        lights[DIR_SOUTH].setColor(LIGHT_GREEN);
        lights[DIR_EAST].setColor(LIGHT_RED);
        lights[DIR_WEST].setColor(LIGHT_RED);
    } else if (currentPhase == PHASE_NS_YELLOW) {
        lights[DIR_NORTH].setColor(LIGHT_YELLOW);
        lights[DIR_SOUTH].setColor(LIGHT_YELLOW);
        lights[DIR_EAST].setColor(LIGHT_RED);
        lights[DIR_WEST].setColor(LIGHT_RED);
    } else if (currentPhase == PHASE_ALL_RED_1 || currentPhase == PHASE_ALL_RED_2) {
        lights[DIR_NORTH].setColor(LIGHT_RED);
        lights[DIR_SOUTH].setColor(LIGHT_RED);
        lights[DIR_EAST].setColor(LIGHT_RED);
        lights[DIR_WEST].setColor(LIGHT_RED);
    } else if (currentPhase == PHASE_EW_GREEN) {
        lights[DIR_NORTH].setColor(LIGHT_RED);
        lights[DIR_SOUTH].setColor(LIGHT_RED);
        lights[DIR_EAST].setColor(LIGHT_GREEN);
        lights[DIR_WEST].setColor(LIGHT_GREEN);
    } else if (currentPhase == PHASE_EW_YELLOW) {
        lights[DIR_NORTH].setColor(LIGHT_RED);
        lights[DIR_SOUTH].setColor(LIGHT_RED);
        lights[DIR_EAST].setColor(LIGHT_YELLOW);
        lights[DIR_WEST].setColor(LIGHT_YELLOW);
    } else if (currentPhase == PHASE_EMERGENCY) {
        if (emergencyLane == DIR_NORTH || emergencyLane == DIR_SOUTH) {
            lights[DIR_NORTH].setColor(LIGHT_GREEN);
            lights[DIR_SOUTH].setColor(LIGHT_GREEN);
            lights[DIR_EAST].setColor(LIGHT_RED);
            lights[DIR_WEST].setColor(LIGHT_RED);
        } else {
            lights[DIR_NORTH].setColor(LIGHT_RED);
            lights[DIR_SOUTH].setColor(LIGHT_RED);
            lights[DIR_EAST].setColor(LIGHT_GREEN);
            lights[DIR_WEST].setColor(LIGHT_GREEN);
        }
    }
}

void TrafficLightController::update(float dt) {
    if (manualMode || emergencyActive) {
        return;
    }

    timer += dt;

    if (currentPhase == PHASE_NS_GREEN && timer >= greenDuration) {
        timer = 0.0f;
        currentPhase = PHASE_NS_YELLOW;
        applyPhaseLights();
    } else if (currentPhase == PHASE_NS_YELLOW && timer >= yellowDuration) {
        timer = 0.0f;
        currentPhase = PHASE_ALL_RED_1;
        applyPhaseLights();
    } else if (currentPhase == PHASE_ALL_RED_1 && timer >= allRedDuration) {
        timer = 0.0f;
        currentPhase = PHASE_EW_GREEN;
        applyPhaseLights();
    } else if (currentPhase == PHASE_EW_GREEN && timer >= greenDuration) {
        timer = 0.0f;
        currentPhase = PHASE_EW_YELLOW;
        applyPhaseLights();
    } else if (currentPhase == PHASE_EW_YELLOW && timer >= yellowDuration) {
        timer = 0.0f;
        currentPhase = PHASE_ALL_RED_2;
        applyPhaseLights();
    } else if (currentPhase == PHASE_ALL_RED_2 && timer >= allRedDuration) {
        timer = 0.0f;
        currentPhase = PHASE_NS_GREEN;
        applyPhaseLights();
    }
}

void TrafficLightController::triggerEmergency(Direction lane) {
    if (!emergencyActive) {
        previousPhase = currentPhase;
    }
    emergencyActive = true;
    emergencyLane = lane;
    currentPhase = PHASE_EMERGENCY;
    applyPhaseLights();
}

void TrafficLightController::clearEmergency() {
    emergencyActive = false;
    currentPhase = previousPhase;
    timer = 0.0f;
    applyPhaseLights();
}

void TrafficLightController::toggleManualMode() {
    manualMode = !manualMode;
}

void TrafficLightController::cycleManual() {
    if (!manualMode || emergencyActive) return;

    if (currentPhase == PHASE_NS_GREEN) {
        currentPhase = PHASE_NS_YELLOW;
    } else if (currentPhase == PHASE_NS_YELLOW) {
        currentPhase = PHASE_ALL_RED_1;
    } else if (currentPhase == PHASE_ALL_RED_1) {
        currentPhase = PHASE_EW_GREEN;
    } else if (currentPhase == PHASE_EW_GREEN) {
        currentPhase = PHASE_EW_YELLOW;
    } else if (currentPhase == PHASE_EW_YELLOW) {
        currentPhase = PHASE_ALL_RED_2;
    } else {
        currentPhase = PHASE_NS_GREEN;
    }
    timer = 0.0f;
    applyPhaseLights();
}

SignalPhase TrafficLightController::getPhase() const {
    return currentPhase;
}

float TrafficLightController::getRemainingTime() const {
    if (emergencyActive) return 0.0f;
    if (currentPhase == PHASE_NS_GREEN || currentPhase == PHASE_EW_GREEN) {
        float rem = greenDuration - timer;
        return rem > 0.0f ? rem : 0.0f;
    } else if (currentPhase == PHASE_NS_YELLOW || currentPhase == PHASE_EW_YELLOW) {
        float rem = yellowDuration - timer;
        return rem > 0.0f ? rem : 0.0f;
    } else {
        float rem = allRedDuration - timer;
        return rem > 0.0f ? rem : 0.0f;
    }
}

bool TrafficLightController::isEmergencyActive() const {
    return emergencyActive;
}

Direction TrafficLightController::getEmergencyDirection() const {
    return emergencyLane;
}

bool TrafficLightController::isManual() const {
    return manualMode;
}

LightColor TrafficLightController::getLightColor(Direction dir) const {
    return lights[dir].getColor();
}

void TrafficLightController::draw(HDC hdc) const {
    for (int i = 0; i < 4; ++i) {
        lights[i].draw(hdc);
    }
}
