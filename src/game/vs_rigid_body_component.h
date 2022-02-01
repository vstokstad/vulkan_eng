//
// Created by Vilhelm Stokstad on 2022-01-31.
//

#ifndef _VS_RIGID_BODY_COMPONENT_H_
#define _VS_RIGID_BODY_COMPONENT_H_

struct box_collider_2D {
  float w, h;
  float x, y;
};
struct box_collider_3D {
  glm::vec3 extents{1.f, 1.f, 1.f};
  glm::vec3 center{0.f, 0.f, 0.f};
  std::vector<glm::vec3> normals = {{0.0f, -1.f, 0.0f}, {0.0f, 1.f, 0.0f},
                                    {-1.0f, 0.f, 0.0f}, {1.0f, 0.f, 0.0f},
                                    {0.0f, 0.f, 1.0f}, {0.0f, 0.f, 1.0f}};

  std::vector<glm::vec3> corners = {center.x+extents.x, center.x+};

  void sat_test(const glm::vec3 &axis, const std::vector<glm::vec3> &ptSet,
                float &minAlong, float &maxAlong) {
    minAlong = HUGE, maxAlong = -HUGE;
    for (int i = 0; i < ptSet.size(); i++) {
      // just dot it to get the min/max along this axis.
      float dotVal = glm::dot(ptSet[i], axis);
      if (dotVal < minAlong)
        minAlong = dotVal;
      if (dotVal > maxAlong)
        maxAlong = dotVal;
    }
  }
};

struct rigid_body_component {
  glm::vec3 velocity;
  float mass{1.0f};
  box_collider_3D collider;
};


inline bool isBetweenOrdered(float val, float lowerBound, float upperBound) {
  return lowerBound <= val && val <= upperBound;
}
bool overlaps(float min1, float max1, float min2, float max2) {
  return isBetweenOrdered(min2, min1, max1) ||
         isBetweenOrdered(min1, min2, max2);
}

// Shape1 and Shape2 must be CONVEX HULLS
bool intersects(box_collider_3D shape1, box_collider_3D shape2) {
  // Get the normals for one of the shapes,

  for (int i = 0; i < shape1.normals.size(); i++) {
    float shape1Min, shape1Max, shape2Min, shape2Max;
    SATtest(shape1.normals[i], shape1.corners, shape1Min, shape1Max);
    SATtest(shape2.normals[i], shape2.corners, shape2Min, shape2Max);
    if (!overlaps(shape1Min, shape1Max, shape2Min, shape2Max)) {
      return 0; // NO INTERSECTION
    }

    // otherwise, go on with the next test
  }

  // TEST SHAPE2.normals as well

  // if overlap occurred in ALL AXES, then they do intersect
  return 1;
}
#endif //_VS_RIGID_BODY_COMPONENT_H_
