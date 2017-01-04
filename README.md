# SSCAIT-ObserverModule

This is the source code for the observer module used by the Student StarCraft AI Tournament (SSCAIT).

More information can be found at
* SSCAIT website: <http://sscaitournament.com/>
* Technical report describing the module: <https://arxiv.org/abs/1505.00278>



### Usage

This observer module is utilized by a TournamentModule. The following functions should be called:

---

```c++
CameraModule::onStart(BWAPI::Position startPos, int screenWidth, int screenHeight)
```
The screen dimensions are used to center events on the screen. Should be called once at the start.

---

```c++
CameraModule::onFrame()
```
Runs the code determining what the observer should be focusing on and handles camera movement. Should be called every frame.

---

```c++
CameraModule::moveCameraUnitCreated(BWAPI::Unit* unit)
```
Should be called when a unit is created (for example in `TournamentModule::onUnitComplete()`). Is used to inform the observer about unit-creation events.

---

Many fields (such as `cameraMoveTime`, `cameraMoveTimeMin`, and `watchScoutWorkerUntil`) are declared public in the header file and can be changed while e.g. reading a config file. If not changed, the default values (set in the CameraModule constructor) will be used.

