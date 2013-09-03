// Copyright 2013 Sean McKenna
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//

// sphere object in ray tracer


using namespace scene;

namespace scene{

class Sphere: public Object{
  
  public:
    Sphere(){
      center.Set(0, 0, 0);
      radius = 1.0;
    }
    
    // intsersect a ray against the unit sphere
    // ray must be transformed into model space, first
    bool IntersectRay( const Ray &ray, HitInfo &hit, int face=HIT_FRONT ) const{
      
      // pre-compute values for quadratic solution
      Point3 pos = ray.p - center;
      float A = ray.dir % ray.dir;
      float B = 2.0 * pos % ray.dir;
      float C = pos % pos - radius * radius;
      float det = (B * B) - (4 * A * C);
      
      // if the ray intersects, compute the z-buffer value
      if(det >= 0){
        float z = (-B - sqrt(det)) / (2.0 * A);
        hit.z = z;
        return true;
      
      // otherwise, return false
      }else
        return false;
    }
    
  private:
    Point3 center;
    float radius;
};

}
