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

// scene class (nodes, objects, transformation, camera, rendering, etc.)
// originally adapted from code provided by Cem Yuksel


// libraries, namespace
#ifndef _SCENE_CPP_INCLUDED_
#define _SCENE_CPP_INCLUDED_
#include <vector>
#include "library/cyPoint.h"
typedef cyPoint3f Point3;
#include "library/cyMatrix3.h"
typedef cyMatrix3f Matrix3;
using namespace std;


// custom functions
#ifndef min
#  define min(a, b) ((a) < (b) ? (a):(b))
#endif
#ifndef max
#  define max(a, b) ((a) > (b) ? (a):(b))
#endif
#define BIGFLOAT 1.0e30f


// declare namespace
namespace scene{


// Ray definition (point & direction)
class Ray{
  public:
    Point3 p, dir;
    Ray(){}
    Ray(const Point3 &_p, const Point3 &_dir): p(_p), dir(_dir) {}
    void Normalize(){
      dir.Normalize();
    }
};


// Node definition
class Node;


// Hit Info definition (for each node)
#define HIT_NONE 0
#define HIT_FRONT 1
#define HIT_BACK 2
#define HIT_FRONT_AND_BACK (HIT_FRONT | HIT_BACK)
struct HitInfo{
  
  // distance from ray to the hit point
  float z;
  
  // object node that is hit
  const Node *node;
  
  // returns true if the object is hit on a front face, false if back face
  bool front;
  
  // initialization
  HitInfo(){
    Init();
  }
  void Init(){
    z = BIGFLOAT;
    node = NULL;
    front = true;
  }
};


// Item Base definition
class ItemBase{
  private:
    
    // name of the item
    char *name;
  
  public:
    ItemBase(): name(NULL) {}
    virtual ~ItemBase(){
      if(name)
        delete [] name;
    }
    
    const char* GetName() const{
      return name ? name: "";
    }
    
    void SetName(const char *newName){
      if(name)
        delete [] name;
      if(newName){
        int n = strlen(newName);
        name = new char[n + 1];
        for(int i = 0; i < n; i++)
          name[i] = newName[i];
        name[n] = '\0';
      }else{
        name = NULL;
      }
    }
};

// ItemList template definition
template <class T> class ItemList: public vector <T*>{
  public:
    virtual ~ItemList(){
      DeleteAll();
    }
    void DeleteAll(){
      int n = this->size();
      for(int i = 0; i < n; i++)
        if(this->at(i))
          delete this->at(i);
    }
};


// ItemFileList template definition
template <class T> class ItemFileList{
  public:
    void Clear(){
      list.DeleteAll();
    }
    void Append(T* item, const char *name){
      list.push_back(new FileInfo(item, name));
    }
    T* Find(const char *name) const{
      int n = list.size();
      for(int i = 0; i < n; i++)
        if(list[i] && strcmp(name, list[i]->GetName()) == 0)
          return list[i]->GetObj();
        return NULL;
    }
  
  private:
    class FileInfo: public ItemBase{
      private:
        T *item;
      
      public:
        FileInfo(): item(NULL) {}
        FileInfo(T *_item, const char *name): item(_item){
          SetName(name);
        }
        ~FileInfo(){
          Delete();
        }
        void Delete(){
          if(item)
            delete item;
          item = NULL;
        }
        void SetObj(T *_item){
          Delete();
          item = _item;
        }
        T* GetObj(){
          return item;
        }
    };
    
    ItemList<FileInfo> list;
};


// Transformation definition
class Transformation{
  private:
    
    // transformation matrix (to local space)
    Matrix3 tm;
    
    // translation part of transformation
    Point3 pos;
    
    // inverse of transformation matrix (cached)
    mutable Matrix3 itm;
  
  public:
    Transformation(): pos(0, 0, 0){
      tm.SetIdentity();
      itm.SetIdentity();
    }
    const Matrix3& GetTransform() const{
      return tm;
    }
    const Point3& GetPosition() const{
      return pos;
    }
    const Matrix3& GetInverseTransform() const{
      return itm;
    }
    
    // transform into local space
    Point3 TransformTo(const Point3 &p) const{
      return itm * (p - pos);
    }
    
    // transform from local space
    Point3 TransformFrom(const Point3 &p) const{
      return tm * p + pos;
    }
    
    // transform vector to local space
    Point3 VectorTransformTo(const Point3 &dir) const{
      return TransposeMult(tm, dir);
    }
    
    // transform vector from local space
    Point3 VectorTransformFrom(const Point3 &dir) const{
      return TransposeMult(itm, dir);
    }
    
    void Translate(Point3 p){
      pos += p;
    }
    void Rotate(Point3 axis, float degree){
      Matrix3 m;
      m.SetRotation(axis, degree * (float) M_PI / 180.0);
      Transform(m);
    }
    void Scale(float sx, float sy, float sz){
      Matrix3 m;
      m.Zero();
      m[0] = sx;
      m[4] = sy;
      m[8] = sz;
      Transform(m);
    }
    void Transform(const Matrix3 &m){
      tm *= m;
      pos = m * pos;
      tm.GetInverse(itm);
    }
    void InitTransform(){
      pos.Zero();
      tm.SetIdentity();
      itm.SetIdentity();
    }
  
  private:
    
    // multiples given vector with transpose of the matrix
    static Point3 TransposeMult(const Matrix3 &m, const Point3 &dir){
      Point3 d;
      d.x = m.GetColumn(0) % dir;
      d.y = m.GetColumn(1) % dir;
      d.z = m.GetColumn(2) % dir;
      return d;
    }
};


// Object definition (extended for all kinds of objects attached to a node)
class Object{
  public:
    virtual ~Object() = 0;
    virtual bool IntersectRay(const Ray &ray, HitInfo &hit, int face = HIT_FRONT) const = 0;
};
Object::~Object(){}


// ObjectFileList definition
typedef ItemFileList<Object> ObjFileList;


// Node definition (stores objects)
class Node: public ItemBase, public Transformation{
  private:
    
    // child nodes
    Node **child;
    
    // number of child nodes
    int numChild;
    
    // object reference
    Object *obj;
  
  public:
    Node(): child(NULL), numChild(0), obj(NULL) {}
    
    // initialize the node, by deleting all its children
    void Init(){
      DeleteAllChildNodes();
      obj = NULL;
      SetName(NULL);
      InitTransform();
    }
    
    // managing hierarchy of nodes
    int GetNumChild() const{
      return numChild;
    }
    void SetNumChild(int n, int keepOld = false){
      if(n < 0)
        n = 0;
      
      // create a new child pointer
      Node **nc = NULL;
      
      if(n > 0)
        nc = new Node*[n];
      for(int i = 0; i < n; i++)
        nc[i] = NULL;
      if(keepOld){
        int sn = min(n, numChild);
        for(int i = 0; i < sn; i++)
          nc[i] = child[i];
      }
      if(child)
        delete [] child;
      child = nc;
      numChild = n;
    }
    
    const Node* GetChild(int i) const{
      return child[i];
    }
    Node* GetChild(int i){
      return child[i];
    }
    void SetChild(int i, Node *node){
      child[i] = node;
    }
    void AppendChild(Node *node){
      SetNumChild(numChild + 1, true);
      SetChild(numChild - 1, node);
    }
    void RemoveChild(int i){
      for(int j = i; j < numChild - 1; j++)
        child[j] = child[j - 1];
      SetNumChild(numChild - 1);
    }
    void DeleteAllChildNodes(){
      for(int i = 0; i < numChild; i++){
        child[i]->DeleteAllChildNodes();
        delete child[i];
      }
      SetNumChild(0);
    }
    
    // managing node objects
    const Object* GetObject() const{
      return obj;
    }
    Object* GetObject(){
      return obj;
    }
    void SetObject(Object *object){
      obj = object;
    }
    
    // transformations
    Ray ToNodeCoords(const Ray &ray) const{
      Ray r;
      r.p = TransformTo(ray.p);
      r.dir = TransformTo(ray.p + ray.dir) - r.p;
      return r;
    }
};


// Camera definition
class Camera{
  public:
    Point3 pos, dir, up;
    float fov;
    int imgWidth, imgHeight;
    
    void Init(){
      pos.Set(0, 0, 0);
      dir.Set(0, 0, -1);
      up.Set(0, 1, 0);
      fov = 40;
      imgWidth = 200;
      imgHeight = 150;
    }
};


// Color definition
typedef unsigned char uchar;
struct Color24{
  uchar r, g, b;
};


// RenderImage definition
class RenderImage{
  private:
    Color24 *img;
    float *zbuffer;
    uchar *zbuffer8;
    int width, height;
    int numRenderedPixels;
  
  public:
    RenderImage(): img(NULL), zbuffer(NULL), zbuffer8(NULL), width(0), height(0), numRenderedPixels(0) {}
    void Init(int w, int h){
      width = w;
      height = h;
      if(img)
        delete [] img;
      img = new Color24[width * height];
      if(zbuffer)
        delete [] zbuffer;
      zbuffer = new float[width * height];
      for(int i = 0; i < width * height; i++)
        zbuffer[i] = BIGFLOAT;
      if(zbuffer8)
        delete [] zbuffer8;
      zbuffer8 = NULL;
      ResetNumRenderedPixels();
    }
    void setBackground(Color24 c){
      for(int i = 0; i < width * height; i++)
        img[i] = c;
    }
    
    int GetWidth() const{
      return width;
    }
    int GetHeight() const{
      return height;
    }
    Color24* GetPixels(){
      return img;
    }
    float* GetZBuffer(){
      return zbuffer;
    }
    uchar* GetZBufferImage(){
      return zbuffer8;
    }
    
    void ResetNumRenderedPixels(){
      numRenderedPixels = 0;
    }
    int GetNumRenderedPixels() const{
      return numRenderedPixels;
    }
    void IncrementNumRenderPixel(){
      numRenderedPixels++;
    }
    void IncrementNumRenderPixel(int n){
      numRenderedPixels += n;
    }
    bool IsRenderDone() const{
      return numRenderedPixels >= width * height;
    }
    
    void ComputeZBufferImage(){
      int size = width * height;
      if(zbuffer8)
        delete [] zbuffer8;
      zbuffer8 = new unsigned char[size];
      
      float zmin = BIGFLOAT, zmax = 0;
      for(int i = 0; i < size; i++){
        if(zbuffer[i] == BIGFLOAT)
          continue;
        if(zmin > zbuffer[i])
          zmin = zbuffer[i];
        if(zmax < zbuffer[i])
          zmax = zbuffer[i];
      }
      for(int i = 0; i < size; i++){
        if(zbuffer[i] == BIGFLOAT)
          zbuffer8[i] = 33;
        else{
          float f = (zmax - zbuffer[i]) / (zmax - zmin);
          int c = int(f * 200);
          if(c < 0)
            f = 0;
          if(c > 200)
            f = 200;
          zbuffer8[i] = c + 47;
        }
      }
    }
    
    bool SaveImage(const char *filename) const{
      return SavePPM(filename, &img[0].r, 3);
    }
    bool SaveZImage(const char *filename) const{
      return SavePPM(filename, zbuffer8, 1);
    }
  
  private:
    bool SavePPM(const char *filename, uchar *data, int compCount) const{
      FILE *fp = fopen(filename, "wb");
      if(!fp)
        return false;
      fprintf(fp, "P6\n%d %d\n255\n", width, height);
      switch(compCount){
        case 1:
          for(int i = 0; i < width * height; i++){
            uchar d[3] = {data[i], data[i], data[i]};
            fwrite(d, 1, 3, fp);
          }
          break;
        case 3:
          fwrite(data, width * height, 3, fp);
          break;
      }
      fclose(fp);
      return true;
    }    
};


}
#endif
