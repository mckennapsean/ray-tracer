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


// libraries, namespace, define types
#ifndef _SCENE_
#define _SCENE_
#include <vector>
#include "cyCodeBase/cyPoint.h"
#include "cyCodeBase/cyMatrix3.h"
#include "cyCodeBase/cyColor.h"
using namespace std;
typedef cyPoint3f Point;
typedef cyMatrix3f Matrix;
typedef cyColor Color;
typedef cyColorA ColorA;
typedef cyColor24 Color24;
typedef unsigned char uchar;


// custom functions, variables
#define min(a, b) ((a) < (b) ? (a):(b))
#define max(a, b) ((a) > (b) ? (a):(b))
#define FLOAT_MAX 1.0e30f


// declare namespace
namespace scene{


// Ray definition (position & direction)
class Ray{
  public:
    
    // position & direction of ray
    Point pos, dir;
    
    // ray constructor
    Ray(){}
    Ray(Point &p, Point &d){
      pos = p;
      dir = d;
    }
    
    // normalize the ray (only the direction is necessary)
    void normalize(){
      dir.Normalize();
    }
};


// Node declaration (definition comes later)
class Node;


// Hit Info struct definitions (set for each node)
#define HIT_NONE 0
#define HIT_FRONT 1
#define HIT_BACK 2
#define HIT_FRONT_AND_BACK (HIT_FRONT | HIT_BACK)
struct HitInfo{
  
  // distance from the ray to the hit point
  float z;
  
  // where the object gets hit
  Point p;
  
  // surface normal of the object at the hit point
  Point n;
  
  // object node that ray hits
  Node *node;
  
  // returns true if the object is hit on a front face, false if back face
  bool front;
  
  // constructor
  HitInfo(){
    init();
  }
  
  // initialize hit info
  void init(){
    z = FLOAT_MAX;
    node = NULL;
    front = true;
  }
};


// Item Base definition (basic node info, stores a name)
class ItemBase{
  private:
    
    // name of the item
    char *name;
  
  public:
    
    // item constructor
    ItemBase(){
      name = NULL;
    }
    virtual ~ItemBase(){
      if(name)
        delete[] name;
    }
    
    // retrive the name of the item
    const char* getName() const{
      return name ? name: "";
    }
    
    // declare a name for the item (or leave NULL)
    void setName(const char *newName){
      if(name)
        delete[] name;
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


// ItemList definition (for ItemFileList, and thus a list of objects)
template <class T> class ItemList: public vector <T*>{
  public:
    
    // empty constructor
    virtual ~ItemList(){
      deleteAll();
    }
    
    // clear all elements within this class
    void deleteAll(){
      int n = this->size();
      for(int i = 0; i < n; i++)
        if(this->at(i))
          delete this->at(i);
    }
};


// ItemFileList definition (from ItemList, feeds into a list of objects)
template <class T> class ItemFileList{
  public:
    
    // clear all elements within this class
    void clear(){
      list.deleteAll();
    }
    
    // add an element to this class
    void append(T* item, const char *name){
      list.push_back(new FileInfo(item, name));
    }
    
    // search entire list for a specific object
    T* find(const char *name){
      int n = list.size();
      for(int i = 0; i < n; i++)
        if(list[i] && strcmp(name, list[i]->getName()) == 0)
          return list[i]->getObj();
        return NULL;
    }
  
  private:
    
    // FileInfo definition (feeds from the basic item, so each file in a list can have a name)
    class FileInfo: public ItemBase{
      private:
        
        // item in the list
        T *item;
      
      public:
        
        // constructors
        FileInfo(){
          item = NULL;
        }
        FileInfo(T *i, const char *name){
          item = i;
          setName(name);
        }
        ~FileInfo(){
          clear();
        }
        
        // clear the item from the list
        void clear(){
          if(item)
            delete item;
          item = NULL;
        }
        
        // replace the current item in the list
        void setObj(T *i){
          clear();
          item = i;
        }
        
        // grab the current list item
        T* getObj(){
          return item;
        }
    };
    
    // store items in a list with info (names) attached
    ItemList<FileInfo> list;
};


// Transformation definition (how to change between spaces)
class Transformation{
  private:
    
    // transformation matrix (to some coordinate system)
    Matrix mat;
    
    // translation part of transformation
    Point pos;
    
    // inverse of transformation matrix (cached)
    mutable Matrix imat;
  
  public:
    
    // constructor (identity transformation)
    Transformation(){
      pos.Set(0, 0, 0);
      mat.SetIdentity();
      imat.SetIdentity();
    }
    
    // get the transformation matrix, position, or inverse transformation matrix
    Matrix& getTransform(){
      return mat;
    }
    Point& getPosition(){
      return pos;
    }
    Matrix& getInverseTransform(){
      return imat;
    }
    
    // transform into local coordinate system
    Point transformTo(Point p){
      return imat * (p - pos);
    }
    
    // transform from local coordinate system
    Point transformFrom(Point p){
      return mat * p + pos;
    }
    
    // transform vector to local coordinate system
    Point vecTransformTo(Point &dir){
      return multiplyTranspose(mat, dir);
    }
    
    // transform vector from local coordinate system
    Point vecTransformFrom(Point &dir){
      return multiplyTranspose(imat, dir);
    }
    
    // set the translation of the local coordinate system
    void translate(Point p){
      pos += p;
    }
    
    // set the rotation of the local coordinate system
    // (about some axis and an amount (degrees) to rotate)
    void rotate(Point axis, float degree){
      Matrix m;
      m.SetRotation(axis, degree * (float) M_PI / 180.0);
      transform(m);
    }
    
    // set the scale of the local coordinate system
    void scale(float sx, float sy, float sz){
      Matrix m;
      m.Zero();
      m[0] = sx;
      m[4] = sy;
      m[8] = sz;
      transform(m);
    }
    
    // update the local coordinate system transformation matrix
    void transform(Matrix &m){
      mat *= m;
      pos = m * pos;
      mat.GetInverse(imat);
    }
    
    // create an initial (identity) transformation matrix
    void initTransform(){
      pos.Zero();
      mat.SetIdentity();
      imat.SetIdentity();
    }
  
  private:
    
    // multiplies given vector with transpose of the matrix
    static Point multiplyTranspose(Matrix &m, Point &dir){
      Point d;
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
    virtual bool intersectRay(Ray &r, HitInfo &h, int face = HIT_FRONT) = 0;
};
Object::~Object(){}


// ObjectFileList definition (objects are all stored in an ItemFileList with their names and search features)
typedef ItemFileList<Object> ObjFileList;


// Light definition (extended to a GenericLight, and then nested to specific lights)
class Light: public ItemBase{
  public:
    virtual Color illuminate(Point p) = 0;
    virtual Point direction(Point p) = 0;
    virtual bool isAmbient(){
      return false;
    }
};


// LightLight definition (store all lights in a list)
class LightList: public ItemList<Light> {};


// Material definition (extended to specific materials for shading)
class Material: public ItemBase{
  public:
    
    // shade method which calls all lights in the list
    // uses the incoming ray, hit info of rendering pixel, and all lights
    virtual Color shade(Ray &r, HitInfo &h, LightList &lights) = 0;
};


// MaterialList definition (stores all materials, searchable)
class MaterialList: public ItemList<Material> {
  public:
    Material* find(const char *name){
      int n = size();
      for(int i = 0; i < n; i++)
        if(at(i) && strcmp(name, at(i)->getName()) == 0)
          return at(i);
      return NULL;
    }
};


// Node definition (pieces of the scene which store objects)
class Node: public ItemBase, public Transformation{
  private:
    
    // child nodes
    Node **child;
    
    // number of child nodes
    int numChild;
    
    // object reference
    Object *obj;
    
    // material used in shading an object
    Material *matl;
  
  public:
    
    // empty constructor
    Node(){
      child = NULL;
      numChild = 0;
      obj = NULL;
    }
    
    // initialize the node, by deleting all its children
    void init(){
      deleteAllChildNodes();
      obj = NULL;
      setName(NULL);
      initTransform();
    }
    
    // get / set number of children
    int getNumChild(){
      return numChild;
    }
    void setNumChild(int n, int keepOld = false){
      if(n < 0)
        n = 0;
      
      // create a new child pointer
      Node **nc = NULL;
      
      // create a new child (or shift them all)
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
        delete[] child;
      child = nc;
      numChild = n;
    }
    
    // get / set a specific child (with some object)
    Node* getChild(int i){
      return child[i];
    }
    void setChild(int i, Node *node){
      child[i] = node;
    }
    
    // add an additional child
    void appendChild(Node *node){
      setNumChild(numChild + 1, true);
      setChild(numChild - 1, node);
    }
    
    // remove a specific child
    void removeChild(int c){
      for(int i = c; i < numChild - 1; i++)
        child[i] = child[i - 1];
      setNumChild(numChild - 1);
    }
    
    // remove all child nodes
    void deleteAllChildNodes(){
      for(int i = 0; i < numChild; i++){
        child[i]->deleteAllChildNodes();
        delete child[i];
      }
      setNumChild(0);
    }
    
    // get / set objects attached to node
    Object* getObject(){
      return obj;
    }
    void setObject(Object *object){
      obj = object;
    }
    
    // get / set materials attached to node
    Material* getMaterial(){
      return matl;
    }
    void setMaterial(Material *m){
      matl = m;
    }
    
    // transformation of rays to model (local) space
    Ray toModelSpace(Ray &ray){
      Ray r;
      r.pos = transformTo(ray.pos);
      r.dir = transformTo(ray.pos + ray.dir) - r.pos;
      return r;
    }
    
    // transformation of hit information from model (local) space back to world space
    void fromModelSpace(HitInfo &hitInfo){
      hitInfo.p = transformFrom(hitInfo.p);
      hitInfo.n = vecTransformFrom(hitInfo.n).GetNormalized();
    }
};


// NodeMaterial definition (connecting a node and material together)
struct NodeMaterial{
  Node *node;
  const char *materialName;
};
vector<NodeMaterial> nodeMaterialList;


// Camera definition (stores basic render info)
class Camera{
  public:
    
    // camera specifications
    Point pos, dir, up, cross;
    float fov;
    int imgWidth, imgHeight;
    
    // initialize camera
    void init(){
      pos.Set(0, 0, 0);
      dir.Set(0, 0, -1);
      up.Set(0, 1, 0);
      fov = 40;
      imgWidth = 200;
      imgHeight = 150;
    }
    
    // calculate necessary camera values from input
    void setup(){
      dir -= pos;
      dir.Normalize();
      up.Normalize();
      cross = dir ^ up;
      cross.Normalize();
      up = (cross ^ dir).GetNormalized();
    }
};


// Render definition (image output from the ray tracer)
class Render{
  private:
    Color24 *render;
    Color24 background;
    float *z;
    uchar *zbuffer;
    int width, height;
    int size;
    int rendered;
  
  public:
    
    // empty constructor
    Render(){
      render = NULL;
      z = NULL;
      zbuffer = NULL;
      width = 0;
      height = 0;
      size = 0;
      rendered = 0;
    }
    
    // initialization of render (need screen size)
    void init(int w, int h){
      width = w;
      height = h;
      size = w * h;
      if(render)
        delete[] render;
      render = new Color24[size];
      background.Set(0, 0, 0);
      if(z)
        delete[] z;
      z = new float[size];
      for(int i = 0; i < size; i++)
        z[i] = FLOAT_MAX;
      if(zbuffer)
        delete[] zbuffer;
      zbuffer = NULL;
      reset();
    }
    
    // set background color for render
    void setBackground(Color24 c){
      background = c;
      for(int i = 0; i < size; i++)
        render[i] = c;
    }
    
    // getters: width, height, size, render, buffer, rendered
    int getWidth(){
      return width;
    }
    int getHeight(){
      return height;
    }
    int getSize(){
      return size;
    }
    Color24* getRender(){
      return render;
    }
    float* getZBuffer(){
      return z;
    }
    int getRendered(){
      return rendered;
    }
    
    // reset the total number of rendered pixels
    void reset(){
      rendered = 0;
    }
    
    // increment total rendered pixels (one or arbitrary)
    void add(){
      rendered++;
    }
    void add(int n){
      rendered += n;
    }
    
    // check if render is done
    bool finished(){
      return rendered >= size;
    }
    
    // calculate the z-buffer image
    void computeZBuffer(){
      
      // clear z-buffer image
      if(zbuffer)
        delete[] zbuffer;
      zbuffer = new unsigned char[size];
      
      // find min, max z-values
      float minZ = FLOAT_MAX;
      float maxZ = 0;
      for(int i = 0; i < size; i++){
        if(z[i] == FLOAT_MAX)
          continue;
        if(minZ > z[i])
          minZ = z[i];
        if(maxZ < z[i])
          maxZ = z[i];
      }
      
      // offset for background and object color
      int diff = 14;
      int offset = background.r + diff;
      
      // assign pixel values based on min & max z-values
      for(int i = 0; i < size; i++){
        
        // background color
        if(z[i] == FLOAT_MAX)
          zbuffer[i] = background.r;
        
        // for pixels with objects, map from white (close) to dark (far)
        else{
          float f = (maxZ - z[i]) / (maxZ - minZ);
          int c = int(f * 200);
          if(c < 0)
            f = 0;
          if(c > 200)
            f = 200;
          zbuffer[i] = c + offset;
        }
      }
    }
    
    // save the rendered image to a file
    bool save(const char *file){
      return outputImage(file, 3);
    }
    
    // save the rendered z-buffer image to a file
    bool saveZBuffer(const char *file){
      return outputImage(file, 1);
    }
  
  private:
    
    // write out an image file
    bool outputImage(const char *file, int components){
      ofstream f;
      f.open(file);
      
      // if error writing to file
      if(!f)
        return false;
      
      // otherwise, output header in PPM format
      f << "P6\n" << width << " " << height << "\n255\n";
      
      // now write out image in binary
      for(int i = 0; i < size; i++){
        
        // output the rendered color image
        if(components == 3){
          uchar d[3] = {render[i].r, render[i].g, render[i].b};
          f.write(reinterpret_cast<char*>(d), sizeof(d));
          
        // output the z-buffer image
        }else if(components == 1){
          uchar d[3] = {zbuffer[i], zbuffer[i], zbuffer[i]};
          f.write(reinterpret_cast<char*>(d), sizeof(d));
        }
      }
      
      // close file
      f.close();
      return true;
    }
};


}
#endif
