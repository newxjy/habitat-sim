#pragma once

#include "esp/core/esp.h"
#include "esp/nav/PathFinder.h"
#include "esp/scene/SceneGraph.h"
#include "esp/scene/SceneNode.h"

namespace esp {
namespace nav {

class GreedyGeodesicFollowerImpl {
 public:
  /**
   * Ouputs from the greedy follower.  Used to specify which action to take next
   * or that an error occured
   */
  enum class CODES : int {
    ERROR = -2,
    STOP = -1,
    FORWARD = 0,
    LEFT = 1,
    RIGHT = 2
  };

  /**
   * Helper typedef for function pointer to a function that manipulates a scene
   * node These functions are used to get access to the python functions which
   * implement the control functions
   */
  typedef std::function<bool(scene::SceneNode*)> MoveFn;

  /**
   * Helper for a sixdof pose
   */
  struct SixDofPose {
    quatf rotation;
    vec3f translation;
  };

  /**
   * Implements a follower that greedily fits actions to follow the geodesic
   * shortest path
   *
   * Params
   * @param[in] pathfinder Instance of the pathfinder used for calculating the
   *                       geodesic shortest path
   * @param[in] moveForward Function that implements "move_forward" on a
   *                        SceneNode
   * @param[in] turnLeft Function that implements "turn_left" on a SceneNode
   * @param[in] turnRight Function that implements "turn_right" on a SceneNode
   * @param[in] goalDist How close the agent needs to get to the goal before
   *                     calling stop
   * @param[in] forwardAmount The amount "move_forward" moves the agent
   * @param[in] turnAmount The amount "turn_left"/"turn_right" turns the agent
   *                       in radians
   * @param[in] fixThrashing Whether or not to fix thrashing
   * @param[in] thrashingThreshold The length of left, right, left, right
   *                                actions needed to be considered thrashing
   */
  GreedyGeodesicFollowerImpl(PathFinder::ptr& pathfinder,
                             MoveFn& moveForward,
                             MoveFn& turnLeft,
                             MoveFn& turnRight,
                             double goalDist,
                             double forwardAmount,
                             double turnAmount,
                             bool fixThrashing = true,
                             int thrashingThreshold = 16);

  CODES nextActionAlong(const SixDofPose& start, const vec3f& end);

  /**
   * Calculates the next action to follow the path
   *
   * Params
   * @param[in] currentPos The current position
   * @param[in] currentRot The current rotation
   * @param[in] end The end location of the path
   */
  CODES nextActionAlong(const vec3f& currentPos,
                        const vec4f& currentRot,
                        const vec3f& end);

  /**
   * Finds the full path from the current agent state to the end location
   *
   * Params
   * @param[in] startPos The starting position
   * @param[in] startRot The starting rotation
   * @param[in] end The end location of the path
   */
  std::vector<CODES> findPath(const vec3f& startPos,
                              const vec4f& startRot,
                              const vec3f& end);

  std::vector<CODES> findPath(const SixDofPose& start, const vec3f& end);

  /**
   * @breif Reset the planner.
   *
   * Should be called whenever a different goal is choosen or start state
   * differs by more than action from the last start state
   */
  void reset();

 private:
  PathFinder::ptr pathfinder_;
  MoveFn moveForward_, turnLeft_, turnRight_;
  const double forwardAmount_, goalDist_, turnAmount_;
  const bool fixThrashing_;
  const int thrashingThreshold_;
  const float closeToObsThreshold_ = 0.2f;

  std::vector<CODES> actions_;
  std::vector<CODES> thrashingActions_;

  scene::SceneGraph dummyScene_;
  scene::SceneNode dummyNode_{dummyScene_.getRootNode()},
      leftDummyNode_{dummyScene_.getRootNode()},
      rightDummyNode_{dummyScene_.getRootNode()},
      tryStepDummyNode_{dummyScene_.getRootNode()};

  ShortestPath geoDistPath_;
  float geoDist(const vec3f& start, const vec3f& end);

  struct TryStepResult {
    float postGeodesicDistance, postDistanceToClosestObstacle;
    bool didCollide;
  };

  TryStepResult tryStep(const scene::SceneNode& node, const vec3f& end);

  float computeReward(const scene::SceneNode& node,
                      const nav::ShortestPath& path,
                      const size_t primLen);

  bool isThrashing();

  std::vector<nav::GreedyGeodesicFollowerImpl::CODES> nextBestPrimAlong(
      const SixDofPose& state,
      const nav::ShortestPath& path);

  ESP_SMART_POINTERS(GreedyGeodesicFollowerImpl)
};

}  // namespace nav
}  // namespace esp
