#include "Vehicle.h"
#include <cmath>

Vehicle::Vehicle(int vId, Direction dir, int sLane, VehicleType vType, int prio, bool isEmerg, COLORREF color, int w, int h, float maxSpd, float accel)
    : id(vId), direction(dir), subLane(sLane), type(vType), priority(prio), emergency(isEmerg),
      bodyColor(color), width(w), height(h), maxSpeed(maxSpd), acceleration(accel),
      speed(0.0f), stopped(false), cleared(false), x(0.0f), y(0.0f),
      currentLaneOffset(0.0f), targetLaneOffset(0.0f), changingLane(false), yielding(false),
      laneChangeCooldown(0.0f) {
}

Vehicle::~Vehicle() {
}

void Vehicle::changeToSubLane(int newSubLane, float targetOffset) {
    subLane = newSubLane;
    targetLaneOffset = targetOffset;
    changingLane = true;
    laneChangeCooldown = 3.0f;
}

void Vehicle::update(float dt) {
    if (laneChangeCooldown > 0.0f) {
        laneChangeCooldown -= dt;
        if (laneChangeCooldown < 0.0f) laneChangeCooldown = 0.0f;
    }

    if (changingLane) {
        float diff = targetLaneOffset - currentLaneOffset;
        float step = 60.0f * dt;
        if (fabs(diff) <= step) {
            currentLaneOffset = targetLaneOffset;
            changingLane = false;
        } else {
            if (diff > 0.0f) currentLaneOffset += step;
            else currentLaneOffset -= step;
        }
    }

    float currentMax = yielding ? (maxSpeed * 0.40f) : maxSpeed;

    if (stopped) {
        speed -= 350.0f * dt;
        if (speed < 0.0f) speed = 0.0f;
    } else {
        speed += acceleration * dt;
        if (speed > currentMax) speed = currentMax;
    }

    if (direction == DIR_NORTH) {
        y -= speed * dt;
    } else if (direction == DIR_SOUTH) {
        y += speed * dt;
    } else if (direction == DIR_EAST) {
        x += speed * dt;
    } else if (direction == DIR_WEST) {
        x -= speed * dt;
    }
}

int Vehicle::getId() const {
    return id;
}

float Vehicle::getX() const {
    return x;
}

float Vehicle::getY() const {
    return y;
}

float Vehicle::getDrawX() const {
    if (direction == DIR_NORTH || direction == DIR_SOUTH) {
        return x + currentLaneOffset;
    }
    return x;
}

float Vehicle::getDrawY() const {
    if (direction == DIR_EAST || direction == DIR_WEST) {
        return y + currentLaneOffset;
    }
    return y;
}

void Vehicle::setPosition(float nx, float ny) {
    x = nx;
    y = ny;
}

float Vehicle::getSpeed() const {
    return speed;
}

void Vehicle::setSpeed(float s) {
    speed = s;
}

float Vehicle::getMaxSpeed() const {
    return maxSpeed;
}

int Vehicle::getWidth() const {
    return width;
}

int Vehicle::getHeight() const {
    return height;
}

Direction Vehicle::getDirection() const {
    return direction;
}

VehicleType Vehicle::getType() const {
    return type;
}

int Vehicle::getPriority() const {
    return priority;
}

bool Vehicle::isEmergency() const {
    return emergency;
}

bool Vehicle::isStopped() const {
    return stopped;
}

void Vehicle::setStopped(bool stop) {
    stopped = stop;
}

bool Vehicle::isCleared() const {
    return cleared;
}

void Vehicle::setCleared(bool clr) {
    cleared = clr;
}

COLORREF Vehicle::getColor() const {
    return bodyColor;
}

int Vehicle::getSubLane() const {
    return subLane;
}

void Vehicle::setSubLane(int sl) {
    subLane = sl;
}

bool Vehicle::isChangingLane() const {
    return changingLane;
}

bool Vehicle::isYielding() const {
    return yielding;
}

void Vehicle::setYielding(bool yld) {
    yielding = yld;
}

float Vehicle::getLaneChangeCooldown() const {
    return laneChangeCooldown;
}

void Vehicle::setLaneChangeCooldown(float cd) {
    laneChangeCooldown = cd;
}

RECT Vehicle::getBoundingBox() const {
    float dx = getDrawX();
    float dy = getDrawY();
    RECT r;
    r.left = (LONG)(dx - width / 2.0f);
    r.top = (LONG)(dy - height / 2.0f);
    r.right = (LONG)(dx + width / 2.0f);
    r.bottom = (LONG)(dy + height / 2.0f);
    return r;
}

RegularVehicle::RegularVehicle(int vId, Direction dir, int sLane, VehicleType vType, COLORREF color, int w, int h, float maxSpd, float accel)
    : Vehicle(vId, dir, sLane, vType, 0, false, color, w, h, maxSpd, accel) {
}

RegularVehicle::~RegularVehicle() {
}

void RegularVehicle::update(float dt) {
    Vehicle::update(dt);
}

EmergencyVehicle::EmergencyVehicle(int vId, Direction dir, int sLane, VehicleType vType, int prio, COLORREF color, int w, int h, float maxSpd, float accel)
    : Vehicle(vId, dir, sLane, vType, prio, true, color, w, h, maxSpd, accel), sirenTimer(0.0f), sirenPhase(false) {
}

EmergencyVehicle::~EmergencyVehicle() {
}

void EmergencyVehicle::update(float dt) {
    Vehicle::update(dt);
    sirenTimer += dt;
    if (sirenTimer >= 0.14f) {
        sirenTimer = 0.0f;
        sirenPhase = !sirenPhase;
    }
}

void EmergencyVehicle::drawSiren(HDC hdc, int sx, int sy, int sWidth, int sHeight) {
    COLORREF c1 = sirenPhase ? RGB(255, 30, 30) : RGB(30, 100, 255);
    COLORREF c2 = sirenPhase ? RGB(30, 100, 255) : RGB(255, 30, 30);

    HBRUSH b1 = CreateSolidBrush(c1);
    HBRUSH b2 = CreateSolidBrush(c2);
    HPEN nullPen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));

    HGDIOBJ oldBrush = SelectObject(hdc, b1);
    HGDIOBJ oldPen = SelectObject(hdc, nullPen);

    Rectangle(hdc, sx - sWidth / 2, sy - sHeight / 2, sx, sy + sHeight / 2);
    SelectObject(hdc, b2);
    Rectangle(hdc, sx, sy - sHeight / 2, sx + sWidth / 2, sy + sHeight / 2);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(b1);
    DeleteObject(b2);
    DeleteObject(nullPen);
}

Car::Car(int vId, Direction dir, int sLane, float startX, float startY)
    : RegularVehicle(vId, dir, sLane, TYPE_CAR, RGB(50, 120, 215), 
                     (dir == DIR_NORTH || dir == DIR_SOUTH) ? 24 : 40,
                     (dir == DIR_NORTH || dir == DIR_SOUTH) ? 40 : 24,
                     90.0f + (vId * 7 % 28), 140.0f) {
    x = startX;
    y = startY;
}

Car::Car(int vId, Direction dir, int sLane, float startX, float startY, COLORREF customColor, float speedVar)
    : RegularVehicle(vId, dir, sLane, TYPE_CAR, customColor,
                     (dir == DIR_NORTH || dir == DIR_SOUTH) ? 24 : 40,
                     (dir == DIR_NORTH || dir == DIR_SOUTH) ? 40 : 24,
                     88.0f + speedVar, 140.0f) {
    x = startX;
    y = startY;
}

Car::~Car() {
}

void Car::draw(HDC hdc) {
    RECT box = getBoundingBox();

    HBRUSH shadowBrush = CreateSolidBrush(RGB(20, 20, 20));
    HPEN nullPen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
    HGDIOBJ oldPen = SelectObject(hdc, nullPen);
    HGDIOBJ oldBrush = SelectObject(hdc, shadowBrush);

    RoundRect(hdc, box.left + 2, box.top + 2, box.right + 2, box.bottom + 2, 6, 6);

    HBRUSH bodyBrush = CreateSolidBrush(bodyColor);
    SelectObject(hdc, bodyBrush);
    HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(20, 20, 25));
    SelectObject(hdc, borderPen);

    RoundRect(hdc, box.left, box.top, box.right, box.bottom, 6, 6);

    HBRUSH glassBrush = CreateSolidBrush(RGB(180, 215, 240));
    SelectObject(hdc, glassBrush);

    if (direction == DIR_NORTH || direction == DIR_SOUTH) {
        int midY = (box.top + box.bottom) / 2;
        if (direction == DIR_NORTH) {
            Rectangle(hdc, box.left + 3, box.top + 7, box.right - 3, box.top + 13);
            Rectangle(hdc, box.left + 4, box.bottom - 11, box.right - 4, box.bottom - 6);
        } else {
            Rectangle(hdc, box.left + 3, box.bottom - 13, box.right - 3, box.bottom - 7);
            Rectangle(hdc, box.left + 4, box.top + 6, box.right - 4, box.top + 11);
        }
        HBRUSH roofBrush = CreateSolidBrush(RGB(35, 45, 55));
        SelectObject(hdc, roofBrush);
        Rectangle(hdc, box.left + 5, midY - 3, box.right - 5, midY + 3);
        DeleteObject(roofBrush);
    } else {
        int midX = (box.left + box.right) / 2;
        if (direction == DIR_WEST) {
            Rectangle(hdc, box.left + 7, box.top + 3, box.left + 13, box.bottom - 3);
            Rectangle(hdc, box.right - 11, box.top + 4, box.right - 6, box.bottom - 4);
        } else {
            Rectangle(hdc, box.right - 13, box.top + 3, box.right - 7, box.bottom - 3);
            Rectangle(hdc, box.left + 6, box.top + 4, box.left + 11, box.bottom - 4);
        }
        HBRUSH roofBrush = CreateSolidBrush(RGB(35, 45, 55));
        SelectObject(hdc, roofBrush);
        Rectangle(hdc, midX - 3, box.top + 5, midX + 3, box.bottom - 5);
        DeleteObject(roofBrush);
    }

    if (changingLane || yielding) {
        HBRUSH amberB = CreateSolidBrush(RGB(255, 180, 20));
        SelectObject(hdc, amberB);
        if (direction == DIR_SOUTH) {
            Ellipse(hdc, box.right - 5, box.bottom - 5, box.right, box.bottom);
        } else if (direction == DIR_NORTH) {
            Ellipse(hdc, box.left, box.top, box.left + 5, box.top + 5);
        } else if (direction == DIR_EAST) {
            Ellipse(hdc, box.right - 5, box.bottom - 5, box.right, box.bottom);
        } else {
            Ellipse(hdc, box.left, box.top, box.left + 5, box.top + 5);
        }
        DeleteObject(amberB);
    }

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(shadowBrush);
    DeleteObject(bodyBrush);
    DeleteObject(borderPen);
    DeleteObject(glassBrush);
    DeleteObject(nullPen);
}

const char* Car::getTypeName() const {
    return "Car";
}

Bus::Bus(int vId, Direction dir, int sLane, float startX, float startY)
    : RegularVehicle(vId, dir, sLane, TYPE_BUS, RGB(225, 145, 20),
                     (dir == DIR_NORTH || dir == DIR_SOUTH) ? 26 : 58,
                     (dir == DIR_NORTH || dir == DIR_SOUTH) ? 58 : 26,
                     72.0f + (vId * 5 % 16), 110.0f) {
    x = startX;
    y = startY;
}

Bus::Bus(int vId, Direction dir, int sLane, float startX, float startY, float speedVar)
    : RegularVehicle(vId, dir, sLane, TYPE_BUS, RGB(225, 145, 20),
                     (dir == DIR_NORTH || dir == DIR_SOUTH) ? 26 : 58,
                     (dir == DIR_NORTH || dir == DIR_SOUTH) ? 58 : 26,
                     70.0f + speedVar, 110.0f) {
    x = startX;
    y = startY;
}

Bus::~Bus() {
}

void Bus::draw(HDC hdc) {
    RECT box = getBoundingBox();

    HBRUSH bodyBrush = CreateSolidBrush(bodyColor);
    HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(40, 30, 10));
    HGDIOBJ oldBrush = SelectObject(hdc, bodyBrush);
    HGDIOBJ oldPen = SelectObject(hdc, borderPen);

    RoundRect(hdc, box.left, box.top, box.right, box.bottom, 6, 6);

    HBRUSH glassBrush = CreateSolidBrush(RGB(190, 225, 245));
    SelectObject(hdc, glassBrush);

    if (direction == DIR_NORTH || direction == DIR_SOUTH) {
        if (direction == DIR_NORTH) {
            Rectangle(hdc, box.left + 3, box.top + 4, box.right - 3, box.top + 9);
        } else {
            Rectangle(hdc, box.left + 3, box.bottom - 9, box.right - 3, box.bottom - 4);
        }
        for (int i = 0; i < 3; ++i) {
            int yPos = box.top + 14 + i * 12;
            Rectangle(hdc, box.left + 2, yPos, box.left + 5, yPos + 7);
            Rectangle(hdc, box.right - 5, yPos, box.right - 2, yPos + 7);
        }
    } else {
        if (direction == DIR_WEST) {
            Rectangle(hdc, box.left + 4, box.top + 3, box.left + 9, box.bottom - 3);
        } else {
            Rectangle(hdc, box.right - 9, box.top + 3, box.right - 4, box.bottom - 3);
        }
        for (int i = 0; i < 3; ++i) {
            int xPos = box.left + 14 + i * 12;
            Rectangle(hdc, xPos, box.top + 2, xPos + 7, box.top + 5);
            Rectangle(hdc, xPos, box.bottom - 5, xPos + 7, box.bottom - 2);
        }
    }

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(bodyBrush);
    DeleteObject(borderPen);
    DeleteObject(glassBrush);
}

const char* Bus::getTypeName() const {
    return "Bus";
}

Ambulance::Ambulance(int vId, Direction dir, int sLane, float startX, float startY)
    : EmergencyVehicle(vId, dir, sLane, TYPE_AMBULANCE, 3, RGB(250, 250, 250),
                       (dir == DIR_NORTH || dir == DIR_SOUTH) ? 26 : 48,
                       (dir == DIR_NORTH || dir == DIR_SOUTH) ? 48 : 26,
                       170.0f, 260.0f) {
    x = startX;
    y = startY;
}

Ambulance::~Ambulance() {
}

void Ambulance::draw(HDC hdc) {
    RECT box = getBoundingBox();

    HBRUSH bodyBrush = CreateSolidBrush(bodyColor);
    HPEN borderPen = CreatePen(PS_SOLID, 2, RGB(220, 20, 20));
    HGDIOBJ oldBrush = SelectObject(hdc, bodyBrush);
    HGDIOBJ oldPen = SelectObject(hdc, borderPen);

    RoundRect(hdc, box.left, box.top, box.right, box.bottom, 6, 6);

    HBRUSH redBrush = CreateSolidBrush(RGB(220, 20, 20));
    HPEN nullPen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
    SelectObject(hdc, redBrush);
    SelectObject(hdc, nullPen);

    int cx = (int)getDrawX();
    int cy = (int)getDrawY();
    Rectangle(hdc, cx - 6, cy - 2, cx + 7, cy + 3);
    Rectangle(hdc, cx - 2, cy - 6, cx + 3, cy + 7);

    int sx = cx;
    int sy = cy;
    if (direction == DIR_NORTH) {
        sy = box.top + 7;
    } else if (direction == DIR_SOUTH) {
        sy = box.bottom - 7;
    } else if (direction == DIR_WEST) {
        sx = box.left + 7;
    } else if (direction == DIR_EAST) {
        sx = box.right - 7;
    }

    drawSiren(hdc, sx, sy, 12, 5);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(bodyBrush);
    DeleteObject(borderPen);
    DeleteObject(redBrush);
    DeleteObject(nullPen);
}

const char* Ambulance::getTypeName() const {
    return "Ambulance";
}

FireTruck::FireTruck(int vId, Direction dir, int sLane, float startX, float startY)
    : EmergencyVehicle(vId, dir, sLane, TYPE_FIRE_TRUCK, 2, RGB(215, 25, 25),
                       (dir == DIR_NORTH || dir == DIR_SOUTH) ? 28 : 56,
                       (dir == DIR_NORTH || dir == DIR_SOUTH) ? 56 : 28,
                       155.0f, 240.0f) {
    x = startX;
    y = startY;
}

FireTruck::~FireTruck() {
}

void FireTruck::draw(HDC hdc) {
    RECT box = getBoundingBox();

    HBRUSH bodyBrush = CreateSolidBrush(bodyColor);
    HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(50, 10, 10));
    HGDIOBJ oldBrush = SelectObject(hdc, bodyBrush);
    HGDIOBJ oldPen = SelectObject(hdc, borderPen);

    RoundRect(hdc, box.left, box.top, box.right, box.bottom, 6, 6);

    HBRUSH ladderBrush = CreateSolidBrush(RGB(210, 215, 220));
    HPEN ladderPen = CreatePen(PS_SOLID, 1, RGB(140, 145, 150));
    SelectObject(hdc, ladderBrush);
    SelectObject(hdc, ladderPen);

    int cx = (int)getDrawX();
    int cy = (int)getDrawY();
    if (direction == DIR_NORTH || direction == DIR_SOUTH) {
        Rectangle(hdc, cx - 4, cy - 10, cx + 4, cy + 10);
        for (int i = -8; i <= 8; i += 4) {
            MoveToEx(hdc, cx - 4, cy + i, NULL);
            LineTo(hdc, cx + 4, cy + i);
        }
    } else {
        Rectangle(hdc, cx - 10, cy - 4, cx + 10, cy + 4);
        for (int i = -8; i <= 8; i += 4) {
            MoveToEx(hdc, cx + i, cy - 4, NULL);
            LineTo(hdc, cx + i, cy + 4);
        }
    }

    int sx = cx;
    int sy = cy;
    if (direction == DIR_NORTH) {
        sy = box.top + 6;
    } else if (direction == DIR_SOUTH) {
        sy = box.bottom - 6;
    } else if (direction == DIR_WEST) {
        sx = box.left + 6;
    } else if (direction == DIR_EAST) {
        sx = box.right - 6;
    }

    drawSiren(hdc, sx, sy, 14, 5);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(bodyBrush);
    DeleteObject(borderPen);
    DeleteObject(ladderBrush);
    DeleteObject(ladderPen);
}

const char* FireTruck::getTypeName() const {
    return "Fire Truck";
}

PoliceCar::PoliceCar(int vId, Direction dir, int sLane, float startX, float startY)
    : EmergencyVehicle(vId, dir, sLane, TYPE_POLICE, 1, RGB(25, 25, 30),
                       (dir == DIR_NORTH || dir == DIR_SOUTH) ? 24 : 42,
                       (dir == DIR_NORTH || dir == DIR_SOUTH) ? 42 : 24,
                       165.0f, 250.0f) {
    x = startX;
    y = startY;
}

PoliceCar::~PoliceCar() {
}

void PoliceCar::draw(HDC hdc) {
    RECT box = getBoundingBox();

    HBRUSH bodyBrush = CreateSolidBrush(bodyColor);
    HPEN borderPen = CreatePen(PS_SOLID, 1, RGB(10, 10, 15));
    HGDIOBJ oldBrush = SelectObject(hdc, bodyBrush);
    HGDIOBJ oldPen = SelectObject(hdc, borderPen);

    RoundRect(hdc, box.left, box.top, box.right, box.bottom, 6, 6);

    HBRUSH whiteBrush = CreateSolidBrush(RGB(240, 240, 245));
    HPEN nullPen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
    SelectObject(hdc, whiteBrush);
    SelectObject(hdc, nullPen);

    int cx = (int)getDrawX();
    int cy = (int)getDrawY();
    if (direction == DIR_NORTH || direction == DIR_SOUTH) {
        Rectangle(hdc, box.left + 2, cy - 7, box.right - 2, cy + 7);
    } else {
        Rectangle(hdc, cx - 7, box.top + 2, cx + 7, box.bottom - 2);
    }

    drawSiren(hdc, cx, cy, 12, 5);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(bodyBrush);
    DeleteObject(borderPen);
    DeleteObject(whiteBrush);
    DeleteObject(nullPen);
}

const char* PoliceCar::getTypeName() const {
    return "Police";
}
