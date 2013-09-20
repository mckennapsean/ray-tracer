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


// blinn-phong material definition (shading)
class BlinnMaterial: public Material{
  public:
  
    // constructor
    BlinnMaterial(){
      diffuse.Set(0.5, 0.5, 0.5);
      specular.Set(0.7, 0.7, 0.7);
      shininess = 20.0;
      reflection.Set(0.0, 0.0, 0.0);
      refraction.Set(0.0, 0.0, 0.0);
      absorption.Set(0.0, 0.0, 0.0);
      index = 1.0;
    }
    
    // shading function (blinn-phong)
    Color shade(Ray &r, HitInfo &h, LightList &lights, int bounceCount = 1){
      
      // initialize color at pixel
      Color c;
      c.Set(0.0, 0.0, 0.0);
      
      // add shading from each light
      int numLights = lights.size();
      for(int i = 0; i < numLights; i++){
        
        // grab light
        Light *light = lights[i];
        
        // ambient light check
        if(light->isAmbient()){
          
          // add ambient lighting term
          c += diffuse * light->illuminate(h.p);
        
        // otherwise, add diffuse and specular components from light
        }else{
          
          // grab vector to light
          Point l = -light->direction(h.p);
          l.Normalize();
          
          // grab vector to camera
          Point v = -r.dir;
          v.Normalize();
          
          // grab normal
          Point n = h.n;
          n.Normalize();
          
          // calculate geometry term
          float geom = n % l;
          
          // calculate half-way vector
          Point half = v + l;
          half.Normalize();
          
          // calculate total specular factor
          float s = pow(half % n, shininess);
          
          // add specular and diffuse lighting terms (only if positive)
          if(geom > 0){
            if(specularGeometry)
              c += light->illuminate(h.p) * geom * (diffuse + s * specular);
            else
              c += light->illuminate(h.p) * (geom * diffuse + s * specular);
          }
        }
      }
      
      // add reflection color (till out of bounces)
      if(bounceCount > 0 && reflection.Grey() != 0.0){
        
        // create reflected vector
        Ray *reflect = new Ray();
        reflect->pos = h.p;
        reflect->dir = 2 * (h.n % -r.dir) * h.n + r.dir;
        
        // create and store reflected hit info
        HitInfo reflectHI = HitInfo();
        bool reflectHit = traceRay(*reflect, reflectHI);
        
        // grab the node material hit
        if(reflectHit){
          Node *n = reflectHI.node;
          Material *m;
          if(n)
            m = n->getMaterial();
          
          // if hit, recursively add reflections, within bounce count
          if(m)
            c += reflection * m->shade(*reflect, reflectHI, lights, bounceCount - 1);
          
          // for no material objects, show the hit
          else
            c += Color(0.929, 0.929, 0.929);
        }
      }
      
      // reset color if we have a back hit
      if(!h.front)
        c.Set(0.0, 0.0, 0.0);
      
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
    
    // set the reflection color of the material
    void setReflection(Color c){
      reflection = c;
    }
    
    // set the refraction color of the material
    void setRefraction(Color c){
      refraction = c;
    }
    
    // set the absorption color of the material
    void setAbsorption(Color c){
      absorption = c;
    }
    
    // set the index of refraction of the material
    void setRefractionIndex(float f){
      index = f;
    }
    
  private:
    
    // colors for shading
    Color diffuse, specular;
    
    // shininess factor for shading
    float shininess;
    
    // colors for reflection, refraction
    Color reflection, refraction;
    
    // index of refraction
    float index;
    
    // color for absorption
    Color absorption;
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
      reflection.Set(0.0, 0.0, 0.0);
      refraction.Set(0.0, 0.0, 0.0);
      absorption.Set(0.0, 0.0, 0.0);
      index = 1.0;
    }
    
    // shading function (phong)
    Color shade(Ray &r, HitInfo &h, LightList &lights, int bounceCount = 1){
      
      // initialize color at pixel
      Color c;
      c.Set(0.0, 0.0, 0.0);
      
      // add shading from each light
      int numLights = lights.size();
      for(int i = 0; i < numLights; i++){
        
        // grab light
        Light *light = lights[i];
        
        // ambient light check
        if(light->isAmbient()){
          
          // add ambient lighting term
          c += diffuse * light->illuminate(h.p);
        
        // otherwise, add diffuse and specular components from light
        }else{
          
          // grab vector to light
          Point l = -light->direction(h.p);
          l.Normalize();
          
          // grab vector to camera
          Point v = -r.dir;
          v.Normalize();
          
          // grab normal
          Point n = h.n;
          n.Normalize();
          
          // calculate geometry term
          float geom = n % l;
          
          // calculate reflection vector
          Point refl = l - 2.0 * (l % n) * n;
          
          // calculate total specular factor
          // (adjusted shininess to match blinn-phong values)
          float s = pow(refl % v, shininess);
          
          // add specular and diffuse lighting terms (only if positive)
          if(geom > 0){
            if(specularGeometry)
              c += light->illuminate(h.p) * geom * (diffuse + s * specular);
            else
              c += light->illuminate(h.p) * (geom * diffuse + s * specular);
          }
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
    
    // set the reflection color of the material
    void setReflection(Color c){
      reflection = c;
    }
    
    // set the refraction color of the material
    void setRefraction(Color c){
      refraction = c;
    }
    
    // set the absorption color of the material
    void setAbsorption(Color c){
      absorption = c;
    }
    
    // set the index of refraction of the material
    void setRefractionIndex(float f){
      index = f;
    }
    
  private:
    
    // colors for shading
    Color diffuse, specular;
    
    // shininess factor for shading
    float shininess;
    
    // colors for reflection, refraction
    Color reflection, refraction;
    
    // index of refraction
    float index;
    
    // color for absorption
    Color absorption;
};
