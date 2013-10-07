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


// import triangular mesh & BVH storage class
#include "cyCodeBase/cyTriMesh.h"
#include "cyCodeBase/cyBVH.h"


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
      
      // check our BVH for triangular faces, update hit info
      bool triang = traceBVHNode(r, h, face, bvh.GetRootNodeID());
      
      // return hit info from intersecting rays in the BVH faces
      return triang;
    }
    
    // get triangular mesh bounding box
    BoundingBox getBoundBox(){
      return BoundingBox(GetBoundMin(), GetBoundMax());
    }
    
    // when loading a triangular mesh, get its bounding box
    bool load(const char *file){
      bvh.Clear();
      if(!LoadFromFileObj(file))
        return false;
      if(!HasNormals())
        ComputeNormals();
      ComputeBoundingBox();
      bvh.SetMesh(this,4);
      return true;
    }
    
  private:
    
    // add BVH for each triangular mesh
    cyBVHTriMesh bvh;
    
    // intersect a ray with a single triangle (Moller-Trumbore algorithm)
    bool intersectTriangle(Ray &r, HitInfo &h, int face, int faceID){
      
      bool mt = false;
      
      if(mt){
      
      // grab vertex points
      Point a = V(F(faceID).v[0]);
      Point b = V(F(faceID).v[1]);
      Point c = V(F(faceID).v[2]);
      
      // compute edge vectors
      Point e1 = b - a;
      Point e2 = c - a;
      
      // calculate first vector, P
      Point P = r.dir ^ e2;
      
      // calculate the determinant of the matrix equation
      float determ = e1 % P;
      
      // only continue for valid determinant ranges
      if(abs(determ) > getBias()){
        
        // calculate second vector, T
        Point T = r.pos - a;
        
        // calculate a barycentric component (u)
        float u = T % P;
        
        // only allow valid barycentric values
        if(u > -getBias() && u < determ * (1.0 + getBias())){
          
          // calculate a normal of the ray with an edge vector
          Point Q = T ^ e1;
          
          // calculate a barycentric component (v)
          float v = r.dir % Q;
          
          // only allow valid barycentric values
          if(v > -getBias() && v + u < determ * (1.0 + getBias())){
            
            // update barycentric coordinates
            v /= determ;
            u /= determ;
            
            // compute the barycentric coordinates for interpolating values
            Point bc = Point(1.0 - u - v, u, v);
            
            // calculate the distance to hit the triangle
            float t = (e2 % Q) / determ;
            
            // only allow valid distances to hit
            if(t > getBias() && t < h.z){
              
              // distance to hit
              h.z = t;
              
              // set hit point, normal
              h.p = GetPoint(faceID, bc);
              h.n = GetNormal(faceID, bc);
              
              // detect back face hits
              if(determ < 0.0)
                h.front = false;
              
              // return hit info
              return true;
            }
          }
        }
      }
      
      // when no ray hits the triangular face
      return false;
      
      // normal ray tracing
      }else{
      
      // grab face vertices and compute face normal
      Point a = V(F(faceID).v[0]);
      Point b = V(F(faceID).v[1]);
      Point c = V(F(faceID).v[2]);
      Point n = (b - a) ^ (c - a);
      
      // ignore rays nearly parallel to surface
      if(r.dir % n > getBias() || r.dir % n < -getBias()){
        
        // compute distance along ray direction to plane
        float t = -((r.pos - a) % n) / (r.dir % n);
        
        // only accept hits in front of ray (with some bias) & closer hits
        if(t > getBias() && t < h.z){
          
          // compute hit point
          Point hit = r.pos + t * r.dir;
          
          // simplify problem into 2D by removing the greatest normal component
          Point2 a2;
          Point2 b2;
          Point2 c2;
          Point2 hit2;
          if(abs(n.x) > abs(n.y) && abs(n.x) > abs(n.z)){
            a2 = Point2(a.y, a.z);
            b2 = Point2(b.y, b.z);
            c2 = Point2(c.y, c.z);
            hit2 = Point2(hit.y, hit.z);
          }else if(abs(n.y) > abs(n.x) && abs(n.y) > abs(n.z)){
            a2 = Point2(a.x, a.z);
            b2 = Point2(b.x, b.z);
            c2 = Point2(c.x, c.z);
            hit2 = Point2(hit.x, hit.z);
          }else{
            a2 = Point2(a.x, a.y);
            b2 = Point2(b.x, b.y);
            c2 = Point2(c.x, c.y);
            hit2 = Point2(hit.x, hit.y);
          }
          
          // compute the area of the triangular face (in 2D)
          float area = (b2 - a2) ^ (c2 - a2);
          
          // compute smaller areas of the face, in 2D
          // aka, computation of the barycentric coordinates
          float alpha = ((b2 - a2) ^ (hit2 - a2)) / area;
          if(alpha > -getBias() && alpha < 1.0 + getBias()){
            float beta = ((hit2 - a2) ^ (c2 - a2)) / area;
            if(beta > -getBias() && alpha + beta < 1.0 + getBias()){
              
              // interpolate the normal based on barycentric coordinates
              Point bc = Point(1.0 - alpha - beta, beta, alpha);
              
              // distance to hit
              h.z = t;
              
              // set hit point, normal
              h.p = GetPoint(faceID, bc);
              h.n = GetNormal(faceID, bc);
              
              // detect back face hits
              if(r.dir % h.n > 0.0)
                h.front = false;
              
              // return hit info
              return true;
            }
          }
        }
      }
      
      // when no ray hits the triangular face
      return false;
      }
    }
    
    // cast a ray into a BVH node, seeing which triangular faces may get hit
    bool traceBVHNode(Ray &r, HitInfo &h, int face, int nodeID){
      
      // grab node's bounding box
      BoundingBox b = BoundingBox(bvh.GetNodeBounds(nodeID));
      
      // does ray hit the node bounding box?
      bool hit = b.intersectRay(r, h.z);
      
      // recurse through child nodes if we hit node
      if(hit){
        
        // only recursively call function if not at a leaf node
        if(!bvh.IsLeafNode(nodeID)){
          
          // keep traversing the BVH hierarchy for hits
          int c1 = bvh.GetFirstChildNode(nodeID);
          int c2 = bvh.GetSecondChildNode(nodeID);
          bool hit1 = traceBVHNode(r, h, face, c1);
          bool hit2 = traceBVHNode(r, h, face, c2);
          
          // if we get no hit
          if(!hit1 && !hit2)
            hit = false;
        
        // for leaf nodes, trace ray into each triangular face
        }else{
          
          // get triangular faces of the hit BVH node
          const unsigned int* faces = bvh.GetNodeElements(nodeID);
          int size = bvh.GetNodeElementCount(nodeID);
          
          // trace the ray into each triangular face, tracking hits
          hit = false;
          for(int i = 0; i < size; i++){
            bool hit1 = intersectTriangle(r, h, face, faces[i]);
            if(!hit && hit1)
              hit = true;
          }
        }
      }
      
      // return if we hit a face within this node
      return hit;
    }
};
