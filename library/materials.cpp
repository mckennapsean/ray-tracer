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
      diffuse.setColor(0.5, 0.5, 0.5);
      specular.setColor(0.7, 0.7, 0.7);
      shininess = 20.0;
      reflection.setColor(0.0, 0.0, 0.0);
      refraction.setColor(0.0, 0.0, 0.0);
      absorption.Set(0.0, 0.0, 0.0);
      index = 1.0;
      reflectionGlossiness = 0.0;
      refractionGlossiness = 0.0;
    }
    
    // shading function (blinn-phong)
    Color shade(Cone &r, HitInfo &h, LightList &lights, int bounceCount = 1){
      
      // initialize color at pixel
      Color c;
      c.Set(0.0, 0.0, 0.0);
      
      // update texture colors from texture
      Color diff = diffuse.sample(h.uvw, h.duvw);
      Color spec = specular.sample(h.uvw, h.duvw);
      Color refl = reflection.sample(h.uvw, h.duvw);
      Color refr = refraction.sample(h.uvw, h.duvw);
      
      // add shading from each light (front hit only)
      if(h.front){
        int numLights = lights.size();
        for(int i = 0; i < numLights; i++){
          
          // grab light
          Light *light = lights[i];
          
          // ambient light check
          if(light->isAmbient()){
            
            // add ambient lighting term
            c += diff * light->illuminate(h.p);
          
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
            if(geom > 0)
              c += light->illuminate(h.p) * geom * (diff + s * spec);
          }
        }
      }
      
      // for smooth objects, set normal
      Point normRefl, normRefr;
      if(reflectionGlossiness == 0.0 && refractionGlossiness == 0.0){
        normRefl = h.n;
        normRefr = h.n;
      
      // otherwise, jitter the normal
      }else{
        
        // get two vectors for spanning our normal
        Point v0 = Point(0.0, 1.0, 0.0);
        if(v0 % h.n < -0.9 || v0 % h.n > 0.9)
          v0 = Point(0.0, 0.0, 1.0);
        Point v1 = (v0 ^ h.n).GetNormalized();
        
        // compute randomization about the normal
        float rad = sqrt(dist(rnd));
        float radRefl = rad * reflectionGlossiness;
        float radRefr = rad * refractionGlossiness;
        float rot = dist(rnd) * 2.0 * M_PI;
        
        // compute new normal
        Point norm1 = (h.n + (v0 * radRefl * cos(rot)) + (v1 * radRefl * sin(rot))).GetNormalized();
        Point norm2 = (h.n + (v0 * radRefr * cos(rot)) + (v1 * radRefr * sin(rot))).GetNormalized();
        
        // determine which gets the new normal
        if(reflectionGlossiness == 0.0)
          normRefl = h.n;
        else
          normRefl = norm1;
        if(refractionGlossiness == 0.0)
          normRefr = h.n;
        else
          normRefr = norm2;
      }
      
      // calculate and add reflection color (till out of bounces)
      Color reflectionShade;
      reflectionShade.Set(0.0, 0.0, 0.0);
      
      if(bounceCount > 0 && (refl.Grey() != 0.0 || refr.Grey() != 0.0)){
        
        // create reflected vector
        Cone *reflect = new Cone();
        reflect->pos = h.p;
        reflect->dir = 2 * (normRefl % -r.dir) * normRefl + r.dir;
        
        // update cones for texture filtering
        reflect->radius = r.radiusAt(h.z);
        reflect->tan = r.tan;
        
        // create and store reflected hit info
        HitInfo reflectHI = HitInfo();
        bool reflectHit = traceRay(*reflect, reflectHI);
        
        // grab the node material hit
        if(reflectHit){
          Node *n = reflectHI.node;
          Material *m;
          if(n)
            m = n->getMaterial();
          
          // for the material, recursively add reflections, within bounce count
          if(m)
            reflectionShade = m->shade(*reflect, reflectHI, lights, bounceCount - 1);
          
          // for no material, show the hit
          else
            reflectionShade = Color(0.929, 0.929, 0.929);
          
          // only add reflection color for front hits
          if(h.front)
            c += refl * reflectionShade;
        
        // ray hits environment texture
        }else{
          Color env = environment.sampleEnvironment(reflect->dir);
          c += refl * env;
        }
      }
      
      // add refraction color (front and back face hits)
      if(bounceCount > 0 && refraction.getColor().Grey() != 0.0){
        
        // create refracted vector
        Cone *refract = new Cone();
        refract->pos = h.p;
        
        // update cones for texture filtering
        refract->radius = r.radiusAt(h.z);
        refract->tan = r.tan;
        
        // variables for refraction calculation
        Point v = -r.dir;
        Point n;
        float n1;
        float n2;
        
        // handle front-face and back-face hits accordingly
        if(h.front){
          n1 = 1.0;
          n2 = index;
          n = normRefr;
        }else{
          n1 = index;
          n2 = 1.0;
          n = -normRefr;
        }
        
        // calculate refraction ray direction
        float c1 = n % v;
        float s1 = sqrt(1.0 - c1 * c1);
        float s2 = n1 / n2 * s1;
        float c2 = sqrt(1.0 - s2 * s2);
        Point p = (v - c1 * n).GetNormalized();
        Point pt = s2 * -p;
        Point nt = c2 * -n;
        
        // store ray direction
        refract->dir = pt + nt;
        
        // only cast rays if not total internal reflection
        if(s2 * s2 <= 1.0){
            
          // create and store refracted hit info
          HitInfo refractHI = HitInfo();
          bool refractHit = traceRay(*refract, refractHI);
          
          // grab the node material hit
          if(refractHit){
            Node *n = refractHI.node;
            Material *m;
            if(n)
              m = n->getMaterial();
            
            // for the material, recursively add refractions, within bounce count
            Color refractionShade = Color(0.0, 0.0, 0.0);
            if(m)
              refractionShade = m->shade(*refract, refractHI, lights, bounceCount - 1);
            
            // for no material, show the hit
            else
              refractionShade = Color(0.929, 0.929, 0.929);
            
            // Schlick's approximation for transmittance vs. reflectance
            float r0 = (n1 - n2) / (n1 + n2);
            r0 *= r0;
            float r;
            if(n1 <= n2)
              r = r0 + (1.0 - r0) * (1 - c1) * (1 - c1) * (1 - c1) * (1 - c1) * (1 - c1);
            else
              r = r0 + (1.0 - r0) * (1 - c2) * (1 - c2) * (1 - c2) * (1 - c2) * (1 - c2);
            float t = 1.0 - r;
            
            // compute total refraction color
            Color refractionColor = refr * (t * refractionShade + r * reflectionShade);
            
            // attenuate refraction color by absorption
            if(!refractHI.front){
              refractionColor.r *= exp(-absorption.r * refractHI.z);
              refractionColor.g *= exp(-absorption.g * refractHI.z);
              refractionColor.b *= exp(-absorption.b * refractHI.z);
            }
            
            // add refraction color
            c += refractionColor;
          
          // ray hits environment texture
          }else{
            Color env = environment.sampleEnvironment(refract->dir);
            c += env;
          }
        
        // for total internal reflection
        }else
          c += refr * reflectionShade;
      }
      
      // return final shaded color
      return c;
    }
    
    // set the diffuse color of the material
    void setDiffuse(Color c){
      diffuse.setColor(c);
    }
    
    // set the specular color of the material
    void setSpecular(Color c){
      specular.setColor(c);
    }
    
    // set the shininess factor of the material
    void setShininess(float s){
      shininess = s;
    }
    
    // set the reflection color of the material
    void setReflection(Color c){
      reflection.setColor(c);
    }
    
    // set the refraction color of the material
    void setRefraction(Color c){
      refraction.setColor(c);
    }
    
    // set the absorption color of the material
    void setAbsorption(Color c){
      absorption = c;
    }
    
    // set the index of refraction of the material
    void setRefractionIndex(float f){
      index = f;
    }
    
    // set the diffuse texture for the material
    void setDiffuseTexture(TextureMap *map){
      diffuse.setTexture(map);
    }
    
    // set the specular texture for the material
    void setSpecularTexture(TextureMap *map){
      specular.setTexture(map);
    }
    
    // set the reflection texture for the material
    void setReflectionTexture(TextureMap *map){
      reflection.setTexture(map);
    }
    
    // set the refraction texture for the material
    void setRefractionTexture(TextureMap *map){
      refraction.setTexture(map);
    }
    
    // set the environment texture
    void setEnvironmentTexture(TexturedColor c){
      environment = c;
    }
    
    // set the reflection glossiness
    void setReflectionGlossiness(float gloss){
      reflectionGlossiness = gloss;
    }
    
    // set the refraction glossiness
    void setRefractionGlossiness(float gloss){
      refractionGlossiness = gloss;
    }
    
  private:
    
    // colors for shading
    TexturedColor diffuse, specular;
    
    // shininess factor for shading
    float shininess;
    
    // colors for reflection, refraction
    TexturedColor reflection, refraction;
    
    // index of refraction
    float index;
    
    // color for absorption
    Color absorption;
    
    // environment texture color
    TexturedColor environment;
    
    // glossiness for reflections & refractions
    float reflectionGlossiness, refractionGlossiness;
    
    // random number generation for jittering the normal
    mt19937 rnd;
    uniform_real_distribution<float> dist{0.0, 1.0};
};


// phong material definition (shading)
// blinn-phong material definition (shading)
class PhongMaterial: public Material{
  public:
  
    // constructor
    PhongMaterial(){
      diffuse.setColor(0.5, 0.5, 0.5);
      specular.setColor(0.7, 0.7, 0.7);
      shininess = 20.0;
      reflection.setColor(0.0, 0.0, 0.0);
      refraction.setColor(0.0, 0.0, 0.0);
      absorption.Set(0.0, 0.0, 0.0);
      index = 1.0;
      reflectionGlossiness = 0.0;
      refractionGlossiness = 0.0;
    }
    
    // shading function (phong)
    Color shade(Cone &r, HitInfo &h, LightList &lights, int bounceCount = 1){
      
      // initialize color at pixel
      Color c;
      c.Set(0.0, 0.0, 0.0);
      
      // update texture colors from texture
      Color diff = diffuse.sample(h.uvw, h.duvw);
      Color spec = specular.sample(h.uvw, h.duvw);
      Color refl = reflection.sample(h.uvw, h.duvw);
      Color refr = refraction.sample(h.uvw, h.duvw);
      
      // add shading from each light
      int numLights = lights.size();
      for(int i = 0; i < numLights; i++){
        
        // grab light
        Light *light = lights[i];
        
        // ambient light check
        if(light->isAmbient()){
          
          // add ambient lighting term
          c += diff * light->illuminate(h.p);
        
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
          if(geom > 0)
            c += light->illuminate(h.p) * geom * (diff + s * spec);
        }
      }
      
      // for smooth objects, set normal
      Point normRefl, normRefr;
      if(reflectionGlossiness == 0.0 && refractionGlossiness == 0.0){
        normRefl = h.n;
        normRefr = h.n;
      
      // otherwise, jitter the normal
      }else{
        
        // get two vectors for spanning our normal
        Point v0 = Point(0.0, 1.0, 0.0);
        if(v0 % h.n < -0.9 || v0 % h.n > 0.9)
          v0 = Point(0.0, 0.0, 1.0);
        Point v1 = (v0 ^ h.n).GetNormalized();
        
        // compute randomization about the normal
        float rad = sqrt(dist(rnd));
        float radRefl = rad * reflectionGlossiness;
        float radRefr = rad * refractionGlossiness;
        float rot = dist(rnd) * 2.0 * M_PI;
        
        // compute new normal
        Point norm1 = (h.n + (v0 * radRefl * cos(rot)) + (v1 * radRefl * sin(rot))).GetNormalized();
        Point norm2 = (h.n + (v0 * radRefr * cos(rot)) + (v1 * radRefr * sin(rot))).GetNormalized();
        
        // determine which gets the new normal
        if(reflectionGlossiness == 0.0)
          normRefl = h.n;
        else
          normRefl = norm1;
        if(refractionGlossiness == 0.0)
          normRefr = h.n;
        else
          normRefr = norm2;
      }
      
      // calculate and add reflection color (till out of bounces)
      Color reflectionShade;
      reflectionShade.Set(0.0, 0.0, 0.0);
      if(bounceCount > 0 && (refl.Grey() != 0.0 || refr.Grey() != 0.0)){
        
        // create reflected vector
        Cone *reflect = new Cone();
        reflect->pos = h.p;
        reflect->dir = 2 * (normRefl % -r.dir) * normRefl + r.dir;
        
        // update cones for texture filtering
        reflect->radius = r.radiusAt(h.z);
        reflect->tan = r.tan;
        
        // create and store reflected hit info
        HitInfo reflectHI = HitInfo();
        bool reflectHit = traceRay(*reflect, reflectHI);
        
        // grab the node material hit
        if(reflectHit){
          Node *n = reflectHI.node;
          Material *m;
          if(n)
            m = n->getMaterial();
          
          // for the material, recursively add reflections, within bounce count
          if(m)
            reflectionShade = m->shade(*reflect, reflectHI, lights, bounceCount - 1);
          
          // for no material, show the hit
          else
            reflectionShade = Color(0.929, 0.929, 0.929);
          
          // only add reflection color for front hits
          if(h.front)
            c += refl * reflectionShade;
        
        // ray hits environment texture
        }else{
          Color env = environment.sampleEnvironment(reflect->dir);
          c += refl * env;
        }
      }
      
      // add refraction color (front and back face hits)
      if(bounceCount > 0 && refr.Grey() != 0.0){
        
        // create refracted vector
        Cone *refract = new Cone();
        refract->pos = h.p;
        
        // update cones for texture filtering
        refract->radius = r.radiusAt(h.z);
        refract->tan = r.tan;
        
        // variables for refraction calculation
        Point v = -r.dir;
        Point n;
        float n1;
        float n2;
        
        // handle front-face and back-face hits accordingly
        if(h.front){
          n1 = 1.0;
          n2 = index;
          n = normRefr;
        }else{
          n1 = index;
          n2 = 1.0;
          n = -normRefr;
        }
        
        // calculate refraction ray direction
        float c1 = n % v;
        float s1 = sqrt(1.0 - c1 * c1);
        float s2 = n1 / n2 * s1;
        float c2 = sqrt(1.0 - s2 * s2);
        Point p = (v - c1 * n).GetNormalized();
        Point pt = s2 * -p;
        Point nt = c2 * -n;
        
        // store ray direction
        refract->dir = pt + nt;
        
        // only cast rays if not total internal reflection
        if(s2 * s2 <= 1.0){
            
          // create and store refracted hit info
          HitInfo refractHI = HitInfo();
          bool refractHit = traceRay(*refract, refractHI);
          
          // grab the node material hit
          if(refractHit){
            Node *n = refractHI.node;
            Material *m;
            if(n)
              m = n->getMaterial();
            
            // for the material, recursively add refractions, within bounce count
            Color refractionShade = Color(0.0, 0.0, 0.0);
            if(m)
              refractionShade = m->shade(*refract, refractHI, lights, bounceCount - 1);
            
            // for no material, show the hit
            else
              refractionShade = Color(0.929, 0.929, 0.929);
            
            // Schlick's approximation for transmittance vs. reflectance
            float r0 = (n1 - n2) / (n1 + n2);
            r0 *= r0;
            float r;
            if(n1 <= n2)
              r = r0 + (1.0 - r0) * (1 - c1) * (1 - c1) * (1 - c1) * (1 - c1) * (1 - c1);
            else
              r = r0 + (1.0 - r0) * (1 - c2) * (1 - c2) * (1 - c2) * (1 - c2) * (1 - c2);
            float t = 1.0 - r;
            
            // compute total refraction color
            Color refractionColor = refr * (t * refractionShade + r * reflectionShade);
            
            // attenuate refraction color by absorption
            if(!refractHI.front){
              refractionColor.r *= exp(-absorption.r * refractHI.z);
              refractionColor.g *= exp(-absorption.g * refractHI.z);
              refractionColor.b *= exp(-absorption.b * refractHI.z);
            }
            
            // add refraction color
            c += refractionColor;
          
          // ray hits environment texture
          }else{
            Color env = environment.sampleEnvironment(refract->dir);
            c += env;
          }
        
        // for total internal reflection
        }else
          c += refr * reflectionShade;
      }
      
      // return final shaded color
      return c;
    }
    
    // set the diffuse color of the material
    void setDiffuse(Color c){
      diffuse.setColor(c);
    }
    
    // set the specular color of the material
    void setSpecular(Color c){
      specular.setColor(c);
    }
    
    // set the shininess factor of the material
    void setShininess(float s){
      shininess = s;
    }
    
    // set the reflection color of the material
    void setReflection(Color c){
      reflection.setColor(c);
    }
    
    // set the refraction color of the material
    void setRefraction(Color c){
      refraction.setColor(c);
    }
    
    // set the absorption color of the material
    void setAbsorption(Color c){
      absorption = c;
    }
    
    // set the index of refraction of the material
    void setRefractionIndex(float f){
      index = f;
    }
    
    // set the diffuse texture for the material
    void setDiffuseTexture(TextureMap *map){
      diffuse.setTexture(map);
    }
    
    // set the specular texture for the material
    void setSpecularTexture(TextureMap *map){
      specular.setTexture(map);
    }
    
    // set the reflection texture for the material
    void setReflectionTexture(TextureMap *map){
      reflection.setTexture(map);
    }
    
    // set the refraction texture for the material
    void setRefractionTexture(TextureMap *map){
      refraction.setTexture(map);
    }
    
    // set the environment texture
    void setEnvironmentTexture(TexturedColor c){
      environment = c;
    }
    
    // set the reflection glossiness
    void setReflectionGlossiness(float gloss){
      reflectionGlossiness = gloss;
    }
    
    // set the refraction glossiness
    void setRefractionGlossiness(float gloss){
      refractionGlossiness = gloss;
    }
    
  private:
    
    // colors for shading
    TexturedColor diffuse, specular;
    
    // shininess factor for shading
    float shininess;
    
    // colors for reflection, refraction
    TexturedColor reflection, refraction;
    
    // index of refraction
    float index;
    
    // color for absorption
    Color absorption;
    
    // environment texture color
    TexturedColor environment;
    
    // glossiness for reflections & refractions
    float reflectionGlossiness, refractionGlossiness;
    
    // random number generation for light disk rotation
    mt19937 rnd;
    uniform_real_distribution<float> dist{0.0, 1.0};
};
