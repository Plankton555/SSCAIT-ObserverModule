#include "CameraModule.h"

using namespace BWAPI;


CameraModule::CameraModule()
{
	cameraMoveTime = 150;
	cameraMoveTimeMin = 50;
	watchScoutWorkerUntil = 7500;
	lastMoved = 0;
	lastMovedPriority = 0;
	lastMovedPosition = BWAPI::Position(0, 0);
	cameraFocusPosition = BWAPI::Position(0, 0);
	cameraFocusUnit = NULL;
	followUnit = false;
}

void CameraModule::onStart(BWAPI::Position startPos, int screenWidth, int screenHeight)
{
	myStartLocation = startPos;
	cameraFocusPosition = startPos;
	currentCameraPosition = startPos;
	scrWidth = screenWidth;
	scrHeight = screenHeight;
}

void CameraModule::onFrame()
{
	moveCameraFallingNuke();
	moveCameraIsUnderAttack();
	moveCameraIsAttacking();
	if (Broodwar->getFrameCount() <= watchScoutWorkerUntil) {
		moveCameraScoutWorker();
	}
	moveCameraArmy();
	moveCameraDrop();

	updateCameraPosition();
}

bool CameraModule::isNearEnemyBuilding(BWAPI::Unit unit, std::set<BWAPI::Unit> &enemyUnits) {
	for (std::set<BWAPI::Unit>::iterator it = enemyUnits.begin(); it != enemyUnits.end(); it++) {
		if ((*it)->getType().isBuilding() 
			&& unit->getDistance(*it) <= TILE_SIZE*20 
			&& (Broodwar->self()->isEnemy((*it)->getPlayer()) || ((*it)->getPlayer()->isNeutral() && (*it)->getType().isAddon())) 
			&& (*it)->getType() != UnitTypes::Terran_Vulture_Spider_Mine/* && !(*it)->isLifted()*/) {
			return true;
		}
	}

	return false;
}

bool CameraModule::isNearStartLocation(BWAPI::Position pos) {
	int distance = 1000;
	BWAPI::Point<int, 32>::list startLocations = Broodwar->getStartLocations();

	for (BWAPI::Point<int, 32>::list::iterator it = startLocations.begin(); it != startLocations.end(); it++) {
		Position startLocation = BWAPI::Position(*it);
		
		// if the start position is not our own home, and the start position is closer than distance
		if (!isNearOwnStartLocation(startLocation) && startLocation.getDistance(pos) <= distance) {
			return true;
		}
	}

	return false;
}

bool CameraModule::isNearOwnStartLocation(BWAPI::Position pos) {
	int distance = 10 * TILE_SIZE; // 10*32
	return (myStartLocation.getDistance(pos) <= distance);
}

bool CameraModule::isArmyUnit(BWAPI::Unit unit) {
	return !(unit->getType().isWorker()
		|| unit->getType().isBuilding()
		|| unit->getType() == BWAPI::UnitTypes::Terran_Vulture_Spider_Mine
		|| unit->getType() == BWAPI::UnitTypes::Zerg_Overlord
		|| unit->getType() == BWAPI::UnitTypes::Zerg_Larva);
}

bool CameraModule::shouldMoveCamera(int priority) {
	bool isTimeToMove = BWAPI::Broodwar->getFrameCount() - lastMoved >= cameraMoveTime;
	bool isTimeToMoveIfHigherPrio = BWAPI::Broodwar->getFrameCount() - lastMoved >= cameraMoveTimeMin;
	bool isHigherPrio = lastMovedPriority < priority;
	// camera should move IF: enough time has passed OR (minimum time has passed AND new prio is higher)
	return isTimeToMove || (isHigherPrio && isTimeToMoveIfHigherPrio);
}

void CameraModule::moveCameraIsUnderAttack()
{
	int prio = 3;
	if (!shouldMoveCamera(prio))
	{
		return;
	}

	for each (BWAPI::Unit unit in BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->isUnderAttack())
		{
			moveCamera(unit, prio);
		}
	}
}

void CameraModule::moveCameraIsAttacking()
{
	int prio = 3;
	if (!shouldMoveCamera(prio))
	{
		return;
	}

	for each (BWAPI::Unit unit in BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->isAttacking())
		{
			moveCamera(unit, prio);
		}
	}
}

void CameraModule::moveCameraFallingNuke() {
	int prio = 5;
	if (!shouldMoveCamera(prio))
	{
		return;
	}

	for each (BWAPI::Unit unit in BWAPI::Broodwar->getAllUnits())
	{
		if (unit->getType() == UnitTypes::Terran_Nuclear_Missile && unit->getVelocityY() > 0)
		{
			moveCamera(unit, prio);
			return;
		}
	}
}


void CameraModule::moveCameraScoutWorker() {
	int highPrio = 2;
	int lowPrio = 0;
	if (!shouldMoveCamera(lowPrio))
	{
		return;
	}

	for each (BWAPI::Unit unit in BWAPI::Broodwar->self()->getUnits()) {
		if (!unit->getType().isWorker()) {
			continue;
		}
		if (isNearStartLocation(unit->getPosition())) {
			moveCamera(unit, highPrio);
		} else if (!isNearOwnStartLocation(unit->getPosition())) {
			moveCamera(unit, lowPrio);
		}
	}
}

void CameraModule::moveCameraNukeDetect(BWAPI::Position target) {
	int prio = 4;
	if (!shouldMoveCamera(prio))
	{
		return;
	}
	else {
		moveCamera(target, prio);
	}
}


void CameraModule::moveCameraDrop() {
	int prio = 2;
	if (!shouldMoveCamera(prio))
	{
		return;
	}
	for each (BWAPI::Unit unit in BWAPI::Broodwar->self()->getUnits()) {
		if ((unit->getType() == UnitTypes::Zerg_Overlord || unit->getType() == UnitTypes::Terran_Dropship || unit->getType() == UnitTypes::Protoss_Shuttle)
			&& isNearStartLocation(unit->getPosition()) && unit->getLoadedUnits().size() > 0) {
			moveCamera(unit, prio);
		}
	}
}

void CameraModule::moveCameraArmy() {
	int prio = 1;
	if (!shouldMoveCamera(prio))
	{
		return;
	}
	// Double loop, check if army units are close to each other
	int radius = 50;
	
	BWAPI::Position bestPos;
	BWAPI::Unit bestPosUnit;
	int mostUnitsNearby = 0;

	for each (BWAPI::Unit unit1 in BWAPI::Broodwar->getAllUnits()) {
		if (!isArmyUnit(unit1)) {
			continue;
		}
		BWAPI::Position uPos = unit1->getPosition();

		int nrUnitsNearby = 0;
		for each (BWAPI::Unit unit2 in BWAPI::Broodwar->getUnitsInRadius(uPos, radius)) {
			if (!isArmyUnit(unit2)) {
				continue;
			}
			nrUnitsNearby++;
		}

		if (nrUnitsNearby > mostUnitsNearby) {
			mostUnitsNearby = nrUnitsNearby;
			bestPos = uPos;
			bestPosUnit = unit1;
		}
	}

	if (mostUnitsNearby > 1) {
		moveCamera(bestPosUnit, prio);
	}
}

void CameraModule::moveCameraUnitCreated(BWAPI::Unit unit)
{
	int prio = 1;
	if (!shouldMoveCamera(prio))
	{
		return;
	}
	else if (unit->getPlayer() == Broodwar->self() && !unit->getType().isWorker()) {
		moveCamera(unit, prio);
	}
}

void CameraModule::moveCamera(BWAPI::Position pos, int priority) {
	if (!shouldMoveCamera(priority)) {
		return;
	}
	if (followUnit == false && cameraFocusPosition == pos) {
		// don't register a camera move if the position is the same
		return;
	}

	cameraFocusPosition = pos;
	lastMovedPosition = cameraFocusPosition;
	lastMoved = BWAPI::Broodwar->getFrameCount();
	lastMovedPriority = priority;
	followUnit = false;
}

void CameraModule::moveCamera(BWAPI::Unit unit, int priority) {
	if (!shouldMoveCamera(priority)) {
		return;
	}
	if (followUnit == true && cameraFocusUnit == unit) {
		// don't register a camera move if we follow the same unit
		return;
	}
	
	cameraFocusUnit = unit;
	lastMovedPosition = cameraFocusUnit->getPosition();
	lastMoved = BWAPI::Broodwar->getFrameCount();
	lastMovedPriority = priority;
	followUnit = true;
}

void CameraModule::updateCameraPosition() {
	double moveFactor = 0.1;
	if (followUnit && cameraFocusUnit->getPosition().isValid()) {
		cameraFocusPosition = cameraFocusUnit->getPosition();
	}
	currentCameraPosition = currentCameraPosition + BWAPI::Position(
		(int)(moveFactor*(cameraFocusPosition.x - currentCameraPosition.x)), 
		(int)(moveFactor*(cameraFocusPosition.y - currentCameraPosition.y)));
	BWAPI::Position currentMovedPosition = 
		currentCameraPosition - BWAPI::Position(scrWidth/2, scrHeight/2 - 40); // -40 to account for HUD

	if (currentCameraPosition.isValid()) {
		BWAPI::Broodwar->setScreenPosition(currentMovedPosition);
	}
}
