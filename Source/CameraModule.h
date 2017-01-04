#pragma once
#include <BWAPI.h>
#include <vector>
#include <set>
#include <windows.h>
#include <Shlwapi.h>
#include <iostream>
#include <fstream>
#include "Timer.h"
#include <iostream>
#include <cstdlib>
#include <sstream>

class CameraModule
{
private:
  BWAPI::Position myStartLocation;

public:
  int scrWidth;
  int scrHeight;
  int cameraMoveTime;
  int cameraMoveTimeMin;
  int watchScoutWorkerUntil;

  int lastMoved;
  int lastMovedPriority;
  BWAPI::Position lastMovedPosition;
  BWAPI::Position currentCameraPosition;
  BWAPI::Position cameraFocusPosition;
  BWAPI::Unit* cameraFocusUnit;
  bool followUnit;

  CameraModule();
  virtual void onStart(BWAPI::Position startPos, int screenWidth, int screenHeight);
  virtual void onFrame();
  virtual bool isNearEnemyBuilding(BWAPI::Unit* unit, std::set<BWAPI::Unit*> &enemyUnits);
  virtual bool isNearStartLocation(BWAPI::Position pos);
  virtual bool isNearOwnStartLocation(BWAPI::Position pos);
  virtual bool isArmyUnit(BWAPI::Unit* unit);
  virtual bool shouldMoveCamera(int priority);
  virtual void moveCamera(BWAPI::Position pos, int priority);
  virtual void moveCamera(BWAPI::Unit* unit, int priority);
  virtual void moveCameraIsAttacking();
  virtual void moveCameraIsUnderAttack();
  virtual void moveCameraScoutWorker();
  virtual void moveCameraFallingNuke();
  virtual void moveCameraNukeDetect(BWAPI::Position target);
  virtual void moveCameraDrop();
  virtual void moveCameraArmy();
  virtual void moveCameraUnitCreated(BWAPI::Unit* unit);
  virtual void updateCameraPosition();
};
