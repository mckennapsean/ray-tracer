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
    }
    
    // intsersect a ray against the unit sphere
    // ray must be transformed into model space, first
    bool IntersectRay( const Ray &ray, HitInfo &hit, int face=HIT_FRONT ) const{
      
      // to be implemented
      return false;
    }
    
  private:
    Point3 center;
    static const float radius = 1.0;
};

}
