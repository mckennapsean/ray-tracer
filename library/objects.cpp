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
    bool intersectRay(Ray &r, HitInfo &h, int face=HIT_FRONT){
      
      // pre-compute values for quadratic solution
      Point pos = r.pos - center;
      float A = r.dir % r.dir;
      float B = 2.0 * pos % r.dir;
      float C = pos % pos - radius * radius;
      float det = (B * B) - (4 * A * C);
      
      // if the ray intersects, compute the z-buffer value
      if(det >= 0){
        float z1 = (-B - sqrt(det)) / (2.0 * A);
        float z2 = (-B + sqrt(det)) / (2.0 * A);
        
        // determine if we have a back hit
        if(z1 * z2 < 0.0)
          h.front = false;
        
        // check closest z-buffer value, if positive (ahead of our ray)
        if(z1 > getBias()){
          h.z = z1;
          
          // compute surface intersection and normal
          h.p = r.pos + z1 * r.dir;
          h.n = h.p.GetNormalized();
          
          // return true, ray is hit
          return true;
        
        // check the next ray, if necessary
        }else if(z2 > getBias()){
          h.z = z2;
          
          // compute surface intersection and normal
          h.p = r.pos + z2 * r.dir;
          h.n = h.p.GetNormalized();
          
          // return true, ray is hit
          return true;
          
        // otherwise, all z-buffer values are negative, return false
        }else
          return false;
      }
      
      // otherwise, return false (no ray hit)
      else
        return false;
    }
    
  private:
    
    // sphere center and its radius
    Point center;
    float radius;
};
