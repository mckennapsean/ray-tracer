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

// materials sub-classes (e.g. blinn-phong, phong shading)
// originally adapted from code provided by Cem Yuksel


// namespace
using namespace scene;
namespace scene{


// blinn-phong material definition (shading)
class BlinnMaterial: public Material{
  public:
  
    // constructor
    BlinnMaterial(){
      diffuse.Set(0.5, 0.5, 0.5);
      specular.Set(0.7, 0.7, 0.7);
      shininess = 20.0;
    }
    
    // shading function (blinn-phong)
    Color shade(Ray &r, HitInfo &h, LightList &lights){
      
      // initialize color at pixel
      Color c;
      c.Set(0.0, 0.0, 0.0);
      
      // add shading from each light
      int numLights = lights.size();
      for(int i = 0; i < numLights; i++){
        
        // grab light
        Light *l = lights[i];
        
        // ambient light check
        if(l->isAmbient()){
          
          // add ambient lighting term
          c += diffuse * l->illuminate(h.p);
        
        // otherwise, add diffuse and specular components from light
        }else{
          
          // calculate geometry term
          float geom = h.n % l->direction(h.p);
          
          // calculate half-way vector
          Point half = (h.n + l->direction(h.p)) / (h.n + l->direction(h.p)).Length();
          
          // calculate total specular factor
          float s = pow(half % h.n, shininess);
          
          // add specular and diffuse lighting terms
          c += l->illuminate(h.p) * geom * (diffuse + s * specular);
        }
      }
      
      // return final shaded color
      return c;
    }
    
    // set the diffuse color of the material
    void setDiffuse(Color c){
      diffuse = c;
    }
    
    // set the specular color of the material
    void setSpecular(Color c){
      specular = c;
    }
    
    // set the shininess factor of the material
    void setShininess(float s){
      shininess = s;
    }
    
  private:
    
    // colors for shading
    Color diffuse, specular;
    
    // shininess factor for shading
    float shininess;
};


// phong material definition (shading)
// blinn-phong material definition (shading)
class PhongMaterial: public Material{
  public:
  
    // constructor
    PhongMaterial(){
      diffuse.Set(0.5, 0.5, 0.5);
      specular.Set(0.7, 0.7, 0.7);
      shininess = 20.0;
    }
    
    // shading function (phong)
    Color shade(Ray &r, HitInfo &h, LightList &lights){
      // to be implemented
      return Color();
    }
    
    
    // set the diffuse color of the material
    void setDiffuse(Color c){
      diffuse = c;
    }
    
    // set the specular color of the material
    void setSpecular(Color c){
      specular = c;
    }
    
    // set the shininess factor of the material
    void setShininess(float s){
      shininess = s;
    }
    
  private:
    
    // colors for shading
    Color diffuse, specular;
    
    // shininess factor for shading
    float shininess;
};
}
