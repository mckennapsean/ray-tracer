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
    float shadow(Cone ray, float z = FLOAT_MAX){
      
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
      Cone r = Cone();
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
      size = 0.0;
    }
    
    // get color of point light (check for shadows)
    Color illuminate(Point p){
      if(size == 0.0){
        Cone r = Cone();
        r.pos = p;
        r.dir = position - p;
        return shadow(r, 1.0) * intensity;
      
      // otherwise, we have a spherical light, cast multiple shadow rays
      }else{
        
        // to detect if we are in the penumbra
        bool penumbra = false;
        
        // keep track of running shadow variables
        int count;
        float mean = 0.0;
        
        // calculate random rotation for Halton sequence on our sphere of confusion
        float rotate = dist(rnd) * 2.0 * M_PI;
        
        // cast our minmum number of shadow rays
        for(count = 0; count < shadowMin; count++){
          
          // calculate (partially) randomized shadow ray
          Cone r = getShadowRay(p, count, rotate);
          
          // cast shadow ray
          int val = shadow(r, 1.0);
          
          // update our mean shadow value
          mean = ((float) (mean * count + val)) / ((float) (count + 1));
          
          // check if we are in penumbra
          if(mean != 0.0 && mean != 1.0)
            penumbra = true;
        }
        
        // continue casting more shadow rays, if in penumbra
        if(penumbra){
          
          // continue casting shadow rays
          for(count = shadowMin; count < shadowMax; count++){
            
            // calculate (partially) randomized shadow ray
            Cone r = getShadowRay(p, count, rotate);
            
            // cast shadow ray
            int val = shadow(r, 1.0);
            
            // update our mean shadow value
            mean = ((float) (mean * count + val)) / ((float) (count + 1));
          }
        }
        
        // return our final shaded intensity
        return mean * intensity;
      }
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
    
    // set the size of the point light (now a sphere)
    void setSize(float s){
      size = s;
    }
    
  private:
    
    // intensity (or color) of light
    Color intensity;
    
    // location of light
    Point position;
    
    // size of point light (sphere if not zero)
    float size;
    
    // how many shadow rays to cast
    int shadowMin = 8;
    int shadowMax = 64;
    
    // random number generation for light disk rotation
    mt19937 rnd;
    uniform_real_distribution<float> dist{0.0, 1.0};
    
    // calculate a randomized light position on a spherical light
    Cone getShadowRay(Point p, int c, float r){
      
      // get original direction
      Point dir = (position - p).GetNormalized();
      
      // get two vectors for spanning our light disk
      Point v0 = Point(0.0, 1.0, 0.0);
      if(v0 % dir < 0.1 && v0 % dir > -0.1)
        v0 = Point(0.0, 0.0, 1.0);
      Point v1 = (v0 ^ dir).GetNormalized();
      
      // grab Halton sequence to shift point along light disk
      // first four points on the perimeter of the disk
      float diskRad;
      if(c < 4)
        diskRad = 1.0 * size;
      else
        diskRad = sqrt(Halton(c - 4, 2)) * size;
      
      // grab Halton sequence to shift point around light disk
      // first four points will be distributed about the perimeter
      float diskRot;
      if(c == 0)
        diskRot = 0.0;
      else if(c == 1)
        diskRot = 1.0 * M_PI;
      else if(c == 2)
        diskRot = 0.5 * M_PI;
      else if(c == 3)
        diskRot = 1.5 * M_PI;
      else
        diskRot = Halton(c - 4, 3) * 2.0 * M_PI;
      
      // compute our semi-random position inside the disk
      Point pos = position + (v0 * diskRad * cos(diskRot + r)) + (v1 * diskRad * sin(diskRot + r));
      
      // shadow ray to return
      Cone ray = Cone();
      ray.pos = p;
      ray.dir = pos - p;
      return ray;
    }
};
}
