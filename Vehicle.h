#ifndef VEHICLE_H
#define VEHICLE_H

#include <windows.h>

enum Direction {
    DIR_NORTH = 0,
    DIR_SOUTH = 1,
    DIR_EAST  = 2,
    DIR_WEST  = 3
};

enum VehicleType {
    TYPE_CAR = 0,
    TYPE_BUS = 1,
    TYPE_AMBULANCE = 2,
    TYPE_FIRE_TRUCK = 3,
    TYPE_POLICE = 4
};

class Vehicle {
protected:
    int id;
    float x;
    float y;
    float speed;
    float maxSpeed;
    float acceleration;
    int width;
    int height;
    Direction direction;
    VehicleType type;
    int priority;
    bool emergency;
    bool stopped;
    bool cleared;
    COLORREF bodyColor;
    int subLane;
    float currentLaneOffset;
    float targetLaneOffset;
    bool changingLane;
    bool yielding;
    float laneChangeCooldown;

public:
    Vehicle(int vId, Direction dir, int sLane, VehicleType vType, int prio, bool isEmerg, COLORREF color, int w, int h, float maxSpd, float accel);
    virtual ~Vehicle();

    virtual void update(float dt);
    virtual void draw(HDC hdc) = 0;
    virtual const char* getTypeName() const = 0;

    int getId() const;
    float getX() const;
    float getY() const;
    float getDrawX() const;
    float getDrawY() const;
    void setPosition(float nx, float ny);
    float getSpeed() const;
    void setSpeed(float s);
    float getMaxSpeed() const;
    int getWidth() const;
    int getHeight() const;
    Direction getDirection() const;
    VehicleType getType() const;
    int getPriority() const;
    bool isEmergency() const;
    bool isStopped() const;
    void setStopped(bool stop);
    bool isCleared() const;
    void setCleared(bool clr);
    COLORREF getColor() const;
    int getSubLane() const;
    void setSubLane(int sl);
    bool isChangingLane() const;
    void changeToSubLane(int newSubLane, float targetOffset);
    bool isYielding() const;
    void setYielding(bool yld);
    float getLaneChangeCooldown() const;
    void setLaneChangeCooldown(float cd);
    RECT getBoundingBox() const;
};

class RegularVehicle : public Vehicle {
public:
    RegularVehicle(int vId, Direction dir, int sLane, VehicleType vType, COLORREF color, int w, int h, float maxSpd, float accel);
    virtual ~RegularVehicle();
    virtual void update(float dt);
};

class EmergencyVehicle : public Vehicle {
protected:
    float sirenTimer;
    bool sirenPhase;

public:
    EmergencyVehicle(int vId, Direction dir, int sLane, VehicleType vType, int prio, COLORREF color, int w, int h, float maxSpd, float accel);
    virtual ~EmergencyVehicle();
    virtual void update(float dt);
    void drawSiren(HDC hdc, int sx, int sy, int sWidth, int sHeight);
};

class Car : public RegularVehicle {
public:
    Car(int vId, Direction dir, int sLane, float startX, float startY);
    Car(int vId, Direction dir, int sLane, float startX, float startY, COLORREF customColor, float speedVar);
    virtual ~Car();
    virtual void draw(HDC hdc);
    virtual const char* getTypeName() const;
};

class Bus : public RegularVehicle {
public:
    Bus(int vId, Direction dir, int sLane, float startX, float startY);
    Bus(int vId, Direction dir, int sLane, float startX, float startY, float speedVar);
    virtual ~Bus();
    virtual void draw(HDC hdc);
    virtual const char* getTypeName() const;
};

class Ambulance : public EmergencyVehicle {
public:
    Ambulance(int vId, Direction dir, int sLane, float startX, float startY);
    virtual ~Ambulance();
    virtual void draw(HDC hdc);
    virtual const char* getTypeName() const;
};

class FireTruck : public EmergencyVehicle {
public:
    FireTruck(int vId, Direction dir, int sLane, float startX, float startY);
    virtual ~FireTruck();
    virtual void draw(HDC hdc);
    virtual const char* getTypeName() const;
};

class PoliceCar : public EmergencyVehicle {
public:
    PoliceCar(int vId, Direction dir, int sLane, float startX, float startY);
    virtual ~PoliceCar();
    virtual void draw(HDC hdc);
    virtual const char* getTypeName() const;
};

#endif
