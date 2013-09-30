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

// object sub-classes (e.g. sphere, plane, triangular mesh OBJ)


// import triangular mesh class
#include "cyCodeBase/cyTriMesh.h"


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
        
        // if hit is too close, assume it is a back-face hit
        else if(z1 <= getBias())
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
    
    // get sphere bounding box
    BoundingBox getBoundBox(){
      return BoundingBox(-1.0, -1.0, -1.0, 1.0, 1.0, 1.0);
    }
    
  private:
    
    // sphere center and its radius
    Point center;
    float radius;
};


// Plane definition (a "unit" plane)
class Plane: public Object{
  public:
    
    // intersect a ray against the "unit" plane
    bool intersectRay(Ray &r, HitInfo &h, int face = HIT_FRONT){
      
      // only compute for rays not parallel to the unit plane
      if(r.dir.z > getBias() || r.dir.z < getBias()){
        
        // compute distance along ray direction to plane
        float t = -r.pos.z / r.dir.z;
        
        // only accept hits in front of ray (with some bias)
        if(t > getBias()){
          
          // compute the hit point
          Point hit = r.pos + t * r.dir;
          
          // only allow a hit to occur if on the "unit" plane
          if(hit.x >= -1.0 && hit.y >= -1.0 && hit.x <= 1.0 && hit.y <= 1.0){
            
            // detect back face hits
            if(r.pos.z < 0.0)
              h.front = false;
            
            // distance to hit
            h.z = t;
            
            // set hit point, normal, and return hit info
            h.p = hit;
            h.n = Point(0.0, 0.0, 1.0);
            return true;
          }
        }
      }
      
      // when no ray hits the "unit" plane
      return false;
    }
    
    // get plane bounding box
    BoundingBox getBoundBox(){
      return BoundingBox(-1.0, -1.0, 0.0, 1.0, 1.0, 0.0);
    }
};


// Triangular Mesh Object definition (from an OBJ file)
class TriObj: public Object, private cyTriMesh{
  public:
    
    // intersect a ray against the triangular mesh
    bool intersectRay(Ray &r, HitInfo &h, int face = HIT_FRONT){
      
      // check each triangular face for ray intersection
      for(int i = 0; i < NF(); i++){
        bool triang = intersectTriangle(r, h, face, i);
        if(triang)
          return triang;
      }
      
      // no faces were hit
      return false;
    }
    
    // get triangular mesh bounding box
    BoundingBox getBoundBox(){
      return BoundingBox(GetBoundMin(), GetBoundMax());
    }
    
    // when loading a triangular mesh, get its bounding box
    bool load(const char *file){
      if(!LoadFromFileObj(file))
        return false;
      if(!HasNormals())
        ComputeNormals();
      ComputeBoundingBox();
      return true;
    }
    
  private:
    
    // intersect a ray with a single triangle
    bool intersectTriangle(Ray &r, HitInfo &h, int face, int faceID){
      
      // ignore rays nearly parallel to surface
      Point n = VN(faceID);
      if(r.dir % n > getBias() || r.dir % n < -getBias()){
        
        // compute distance along ray direction to plane
        Point a = V(F(faceID).v[0]);
        float t = -((r.pos - a) % n) / (r.dir % n);
        
        // only accept hits in front of ray (with some bias) & closer hits
        if(t > getBias() && t < h.z){
          
          // compute hit point
          Point hit = r.pos + t * r.dir;
          
          // compute the area of the triangular face
          Point b = V(F(faceID).v[1]);
          Point c = V(F(faceID).v[2]);
          float area = ((b - a) ^ (c - a)).Length();
          
          // compute smaller areas of the face, relative to the full area
          // ensure that we are only calculating positive areas
          // aka, computation of the barycentric coordinates
          Point alphaVector = (b - a) ^ (hit - a);
          if(alphaVector % n > -getBias()){
            float alpha = alphaVector.Length() / area;
            Point betaVector = (hit - a) ^ (c - a);
            if(betaVector % n > -getBias()){
              float beta = betaVector.Length() / area;
              if(alpha + beta < 1.0 + getBias()){
                
                // interpolate the normal based on barycentric coordinates
                Point bc = Point(1.0 - alpha - beta, beta, alpha);
                
                // distance to hit
                h.z = t;
                
                // set hit point, normal
                h.p = GetPoint(faceID, bc);
                h.n = GetNormal(faceID, bc);
                
                // detect back face hits
                if(r.dir % h.n < 0.0)
                  h.front = false;
                
                // return hit info
                return true;
              }
            }
          }
        }
      }
      
      // when no ray hits the triangular face
      return false;
    }
};
