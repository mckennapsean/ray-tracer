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

// light sub-classes (e.g. generic, ambient, direct, point)
// originally adapted from code provided by Cem Yuksel


// namespace
using namespace scene;
namespace scene{


// generic light definition (from main light class)
class GenericLight: public Light{
  public:
    
    // calculate a shadow for any light (never call from ambient!)
    float shadow(Ray ray, float z = FLOAT_MAX){
      
      // add bias to position to avoid first hit
      float bias = 0.001;
      Point d = ray.dir.GetNormalized();
      ray.pos += bias * d;
      
      // set initial hit info
      HitInfo h = HitInfo();
      h.z = z;
      
      // check ray from point to light, is it occluded?
      bool occlude = traceRay(ray, h);
      
      // return 0 if in shadow, 1 if lit directly
      if(occlude)
        return 0.0;
      else
        return 1.0;
    }
};


// ambient light definition
class AmbientLight: public GenericLight{
  public:
    
    // constructor
    AmbientLight(){
      intensity.Set(0, 0, 0);
    }
    
    // get color of ambient light
    Color illuminate(Point p){
      return intensity;
    }
    
    // get direction of ambient light (non-sensical)
    Point direction(Point p){
      return Point(0, 0, 0);
    }
    
    // return true, since light is ambient
    bool isAmbient(){
      return true;
    }
    
    // set color of ambient light
    void setIntensity(Color c){
      intensity = c;
    }
    
  private:
    
    // intensity (or color) of light
    Color intensity;
};


// direct light definition
class DirectLight: public GenericLight{
  public:
    
    // constructor
    DirectLight(){
      intensity.Set(0, 0, 0);
      dir.Set(0, 0, 1);
    }
    
    // get color of direct light (check for shadows)
    Color illuminate(Point p){
      Ray r = Ray();
      r.pos = p;
      r.dir = -dir;
      return shadow(r) * intensity;
    }
    
    // get direction of direct light (constant)
    Point direction(Point p){
      return dir;
    }
    
    // set color of direct light
    void setIntensity(Color c){
      intensity = c;
    }
    
    // set direction of direct light
    void setDirection(Point d){
      dir = d.GetNormalized();
    }
    
  private:
    
    // intensity (or color) of light
    Color intensity;
    
    // direction of light
    Point dir;
};


// point light definition
class PointLight: public GenericLight{
  public:
    
    // constructor
    PointLight(){
      intensity.Set(0, 0, 0);
      position.Set(0, 0, 0);
    }
    
    // get color of point light (check for shadows)
    Color illuminate(Point p){
      Ray r = Ray();
      r.pos = p;
      r.dir = position - p;
      return shadow(r, 1.0) * intensity;
    }
    
    // get direction of point light (calculated)
    Point direction(Point p){
      return (p - position).GetNormalized();
    }
    
    // set color of point light
    void setIntensity(Color c){
      intensity = c;
    }
    
    // set the location of the point light
    void setPosition(Point pos){
      position = pos;
    }
    
  private:
    
    // intensity (or color) of light
    Color intensity;
    
    // location of light
    Point position;
};
}
