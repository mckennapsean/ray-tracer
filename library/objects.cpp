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

// object sub-classes (e.g. sphere)


// namespace
using namespace scene;
namespace scene{


// Sphere definition
class Sphere: public Object{
  public:
    
    // constructor
    Sphere(){
      center.Set(0, 0, 0);
      radius = 1.0;
    }
    
    // intsersect a ray against the unit sphere
    // ray must be transformed into model space, first
    bool intersectRay(Ray &ray, HitInfo &hit, int face=HIT_FRONT){
      
      // pre-compute values for quadratic solution
      Point pos = ray.pos - center;
      float A = ray.dir % ray.dir;
      float B = 2.0 * pos % ray.dir;
      float C = pos % pos - radius * radius;
      float det = (B * B) - (4 * A * C);
      
      // if the ray intersects, compute the z-buffer value
      if(det >= 0){
        float z = (-B - sqrt(det)) / (2.0 * A);
        hit.z = z;
        
        // also get the surface intersection and normal
        hit.p = ray.pos + z * ray.dir;
        hit.p.Normalize();
        hit.n = hit.p;
        
        // return true, ray is hit
        return true;
      
      // otherwise, return false
      }else
        return false;
    }
    
  private:
    
    // sphere center and its radius
    Point center;
    float radius;
};
}
