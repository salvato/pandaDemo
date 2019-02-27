#include "pandaFramework.h"
#include "pandaSystem.h"

#include "genericAsyncTask.h"
#include "asyncTaskManager.h"

#include "cIntervalManager.h"
#include "cLerpNodePathInterval.h"
#include "cMetaInterval.h"

#include "ambientLight.h"
#include "directionalLight.h"
#include "pointLight.h"
#include "spotlight.h"


//==============================================================
// Remember that in Panda3D Y is horizontal and Z is vertical !!
//==============================================================


// The global task manager
static PT(AsyncTaskManager) taskMgr = AsyncTaskManager::get_global_ptr();

// The global clock
static PT(ClockObject) globalClock = ClockObject::get_global_clock();

// Here's what we'll store the camera in.
static NodePath camera;


// This is our task - a global or static function that has to return DoneStatus.
// The task object is passed as argument, plus a void* pointer, containing custom data.
// For more advanced usage, we can subclass AsyncTask and override the do_task method.
AsyncTask::DoneStatus
spinCameraTask(GenericAsyncTask* task, void* data) {
    (void) task;
    (void) data;
// Calculate the new position and orientation (inefficient - change me!)
    double time = globalClock->get_real_time();
    double angledegrees = time * 6.0;
    double angleradians = angledegrees * (3.14 / 180.0);
    camera.set_pos(float(20.0*sin(angleradians)), float(-20.0*cos(angleradians)), 3.0f);
    camera.set_hpr(float(angledegrees), 0.0f, 0.0f);
// Tell the task manager to continue this task the next frame.
    return AsyncTask::DS_cont;
}


PT(CMetaInterval)
setPandaAnimation(NodePath pandaActor) {
// Intervals are tasks that change a property from one value to another
// over a specified period of time.
// Starting an interval effectively starts a background process
// that modifies the property over the specified period of time.

// Create the lerp intervals needed to walk back and forth
// When the pandaPosInterval1 interval is started, it will gradually adjust the position
// of the panda from (0, 10, 0) to (0, -10, 0) over a period of 13 seconds.
// Similarly, when the pandaHprInterval1 interval is started, the heading of the panda
// will rotate 180 degrees over a period of 3 seconds.

    PT(CLerpNodePathInterval) pandaPosInterval1,
                              pandaPosInterval2,
                              pandaHprInterval1,
                              pandaHprInterval2;

    pandaPosInterval1 = new CLerpNodePathInterval("pandaPosInterval1",
                                                  13.0,
                                                  CLerpInterval::BT_no_blend,
                                                  true,
                                                  false,
                                                  pandaActor,
                                                  NodePath());
    pandaPosInterval1->set_start_pos(LPoint3f(0, 10, 0));
    pandaPosInterval1->set_end_pos(LPoint3f(0, -10, 0));

    pandaPosInterval2 = new CLerpNodePathInterval("pandaPosInterval2",
                                                  13.0,
                                                  CLerpInterval::BT_no_blend,
                                                  true,
                                                  false,
                                                  pandaActor,
                                                  NodePath());
    pandaPosInterval2->set_start_pos(LPoint3f(0, -10, 0));
    pandaPosInterval2->set_end_pos(LPoint3f(0, 10, 0));

    pandaHprInterval1 = new CLerpNodePathInterval("pandaHprInterval1",
                                                  3.0,
                                                  CLerpInterval::BT_no_blend,
                                                  true,
                                                  false,
                                                  pandaActor,
                                                  NodePath());
    pandaHprInterval1->set_start_hpr(LPoint3f(0, 0, 0));
    pandaHprInterval1->set_end_hpr(LPoint3f(180, 0, 0));

    pandaHprInterval2 = new CLerpNodePathInterval("pandaHprInterval2",
                                                  3.0,
                                                  CLerpInterval::BT_no_blend,
                                                  true,
                                                  false,
                                                  pandaActor,
                                                  NodePath());
    pandaHprInterval2->set_start_hpr(LPoint3f(180, 0, 0));
    pandaHprInterval2->set_end_hpr(LPoint3f(0, 0, 0));

// Sequences, also called MetaIntervals are tasks that execute one interval after another.
// The pandaPace sequence causes the panda to move in a straight line, turn,
// move in the opposite straight line, and finally turn again.
    PT(CMetaInterval) pandaPace;
    pandaPace = new CMetaInterval("pandaPace");
    pandaPace->add_c_interval(pandaPosInterval1,
                              0,
                              CMetaInterval::RS_previous_end);
    pandaPace->add_c_interval(pandaHprInterval1,
                              0,
                              CMetaInterval::RS_previous_end);
    pandaPace->add_c_interval(pandaPosInterval2,
                              0,
                              CMetaInterval::RS_previous_end);
    pandaPace->add_c_interval(pandaHprInterval2,
                              0,
                              CMetaInterval::RS_previous_end);
// The code pandaPace->loop() causes the Sequence to be started in looping mode.
    pandaPace->loop();
    return pandaPace;
}


int
main(int argc, char *argv[]) {
// Open a new window framework
    PandaFramework framework;
    framework.open_framework(argc, argv);

// Set the window title to My Panda3D Window
    framework.set_window_title("My Panda3D Window");
// Open the window
    WindowFramework *window = framework.open_window();
    if(window == nullptr) {
        framework.close_framework();
        return (EXIT_FAILURE);
    }

// Get the camera and store it in a variable.
    camera = window->get_camera_group();

/*
// Enable keyboard detection
//    window->enable_keyboard();
// Enable default camera movement: allows you to move the camera using the mouse.
// The keys to navigate are:
// Key 	                    Action
// Left Button 	            Pan left and right.
// Right Button 	            Move forwards and backwards.
// Middle Button 	        Rotate around the origin of the application.
// Right and Middle Buttons 	Roll the point of view around the view axis.
//    window->setup_trackball();
*/

// Load the environment model from a .egg file.
// A "NodePath" is a "handle" to a node.
    NodePath scene = window->load_model(framework.get_models(), "models/environment");

// In order to put an object in the world, you will need to parent it to "render"
// or to some node that is in turn parented to "render".
    scene.reparent_to(window->get_render());

// Apply scale and position transforms to the model.
    scene.set_scale(0.25f, 0.25f, 0.25f);
    scene.set_pos(-8.0f, 42.0f, 0.0f);

// Load our first panda
    NodePath pandaActor = window->load_model(framework.get_models(), "models/panda-model");
    pandaActor.reparent_to(window->get_render());
    pandaActor.set_scale(0.005f);

// Load the walk animation
    window->load_model(pandaActor, "models/panda-walk4");
    window->loop_animations(0);// Bind models and animations

    setPandaAnimation(pandaActor);

// Tells Panda3D's task manager to call the procedure spinCameraTask() every frame.
// If we specify custom data instead of nullptr, it will be passed as
// the second argument to the task function.
    taskMgr->add(new GenericAsyncTask("Spins the camera", &spinCameraTask, nullptr));

// Now create some lights to apply to everything in the scene.
// Create Ambient Light
    PT(AmbientLight) ambientLight = new AmbientLight("ambientLight");
    ambientLight->set_color(LVecBase4f(0.1f, 0.1f, 0.1f, 1.0f));
    NodePath ambientLightNP = window->get_render().attach_new_node(ambientLight);
    window->get_render().set_light(ambientLightNP);

// Directional light 01
    PT(DirectionalLight) directionalLight = new DirectionalLight("directionalLight");
    directionalLight->set_color(LVecBase4f(0.8f, 0.2f, 0.2f, 1.0f));
    NodePath directionalLightNP = window->get_render().attach_new_node(directionalLight);
    directionalLightNP.set_hpr(180, -20, 0);
    window->get_render().set_light(directionalLightNP);

// Directional light 02
    directionalLight = new DirectionalLight("directionalLight");
    directionalLight->set_color(LVecBase4f(0.2f, 0.2f, 0.8f, 1.0f));
    directionalLightNP = window->get_render().attach_new_node(directionalLight);
    directionalLightNP.set_hpr(0, -20, 0);
    window->get_render().set_light(directionalLightNP);

// Now attach a green light only to object x.
    PT(AmbientLight) ambient = new AmbientLight("ambient");
    ambient->set_color(LVecBase4f(0.5f, 1.0f, 0.5f, 1.0f));
    NodePath ambientNP = pandaActor.attach_new_node(ambient);

// If we did not call setLightOff() first, the green light would add to
// the total set of lights on this object. Since we do call
// setLightOff(), we are turning off all the other lights on this
// object first, and then turning on only the green light.
    pandaActor.set_light_off();
    pandaActor.set_light(ambientNP);

// This is a simpler way to do stuff every frame,
// if you're too lazy to create a task.
    Thread *current_thread = Thread::get_current_thread();

    while(framework.do_frame(current_thread)) {
// Step the interval manager
      CIntervalManager::get_global_ptr()->step();
    }

//    framework.main_loop();

// Close the window framework
    framework.close_framework();
    return (EXIT_SUCCESS);
}
