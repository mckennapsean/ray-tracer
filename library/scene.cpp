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
typedef cyPoint2f Point2;
typedef cyMatrix3f Matrix;
typedef cyColor Color;
typedef cyColorA ColorA;
typedef cyColor24 Color24;
typedef unsigned char uchar;


// custom functions, variables
#define min(a, b) ((a) < (b) ? (a):(b))
#define max(a, b) ((a) > (b) ? (a):(b))
#define FLOAT_MAX 1.0e30f
#define TEXTURE_SAMPLE_COUNT 32


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
    Ray(Ray &r){
      pos = r.pos;
      dir = r.dir;
    }
    
    // normalize the ray (only the direction is necessary)
    void normalize(){
      dir.Normalize();
    }
};


// add a cone to a ray for casting and differential textures (anti-aliasing textures)
class Cone: public Ray{
  public:
    
    // tangent of the cone angle & cone radius at origin
    float tan, radius;
    
    // cone ray constructor
    Cone(){}
    Cone(Point p, Point &d, float t = 0.0, float r = 0.0){
      pos = p;
      dir = d;
      tan = t;
      radius = r;
    }
    Cone(Ray r, float t = 0.0, float rad = 0.0){
      pos = r.pos;
      dir = r.dir;
      tan = t;
      radius = rad;
    }
    Cone(const Cone &c){
      pos = c.pos;
      dir = c.dir;
      tan = c.tan;
      radius = c.radius;
    }
    
    // returns a radius for some paramemter, t
    float radiusAt(float t){
      return (t * tan + radius) * dir.Length();
    }
    
    // returns the major & minor axes of an ellipse for for some given value
    void ellipseAt(float t, Point &n, Point &major, Point &minor){
      float r = radiusAt(t);
      Point d = dir.GetNormalized();
      Point T = (d ^ n).GetNormalized();
      minor = r * T;
      float c = fabs(d % n);
      if(c < 0.01)
        c = 0.01;
      major = (r / c) * (T ^ n).GetNormalized();
    }
};


// Bounding Box definition (for rendering only when necessary)
class BoundingBox{
  public:
    
    // store min and max bounding box values
    Point minP, maxP;
    
    // constructors
    BoundingBox(){
      init();
    }
    BoundingBox(Point min, Point max){
      minP = min;
      maxP = max;
    }
    BoundingBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ){
      minP = Point(minX, minY, minZ);
      maxP = Point(maxX, maxY, maxZ);
    }
    BoundingBox(const float *dim){
      minP = dim;
      maxP = &dim[3];
    }
    BoundingBox(float *dim){
      minP = dim;
      maxP = &dim[3];
    }
    
    // initialize the bounding box
    // no points should exist in the box (aka, empty)
    void init(){
      minP.Set(FLOAT_MAX, FLOAT_MAX, FLOAT_MAX);
      maxP.Set(-FLOAT_MAX, -FLOAT_MAX, -FLOAT_MAX);
    }
    
    // return true only if the bounding box is empty
    bool isEmpty(){
      return (minP.x > maxP.x || minP.y > maxP.y || minP.z > maxP.z);
    }
    
    // returns one of the eight corners of the bounding box, in order:
    // 0: (minX, minY, minZ)   1: (maxX, minY, minZ)
    // 2: (minX, maxY, minZ)   3: (maxX, maxY, minZ)
    // 4: (minX, minY, maxZ)   5: (maxX, minY, maxZ)
    // 6: (minX, maxY, maxZ)   7: (maxX, maxY, maxZ)
    Point corner(int i){
      Point p;
      if(i % 2 == 0)
        p.x = minP.x;
      else
        p.x = maxP.x;
      if(i % 4 < 2)
        p.y = minP.y;
      else
        p.y = maxP.y;
      if(i < 4)
        p.z = minP.z;
      else
        p.z = maxP.z;
      return p;
    }
    
    // enlarge the bounding box to encompass some point p
    void operator += (Point p){
      for(int i = 0; i < 3; i++){
        if(minP[i] > p[i])
          minP[i] = p[i];
        if(maxP[i] < p[i])
          maxP[i] = p[i];
      }
    }
    
    // enlarge the bounding box by another bounding box
    void operator += (BoundingBox b){
      for(int i = 0; i < 3; i++){
        if(minP[i] > b.minP[i])
          minP[i] = b.minP[i];
        if(maxP[i] < b.maxP[i])
          maxP[i] = b.maxP[i];
      }
    }
    
    // return true only for a point in the bounding box
    bool isInside(Point p){
      for(int i = 0; i < 3; i++)
        if(minP[i] > p[i] || maxP[i] < p[i])
          return false;
      return true;
    }
    
    // returns true only for a ray intersecting the bounding box, if the parameter of the hit is less than some maximum distance away (t)
    bool intersectRay(Ray &r, float t){
      
      // no intersection if we have no bounding box
      if(isEmpty())
        return false;
      
      // intersection must occur if ray originates inside the bounding box
      if(isInside(r.pos))
        return true;
      
      // calculate min & max intersection values for x & y
      // checking for division by zero
      float minX, minY, maxX, maxY;
      if(r.dir.x == 0.0){
        minX = -FLOAT_MAX;
        maxX = FLOAT_MAX;
      }else{
        minX = (minP.x - r.pos.x) / r.dir.x;
        maxX = (maxP.x - r.pos.x) / r.dir.x;
      }if(r.dir.y == 0.0){
        minY = -FLOAT_MAX;
        maxY = FLOAT_MAX;
      }else{
        minY = (minP.y - r.pos.y) / r.dir.y;
        maxY = (maxP.y - r.pos.y) / r.dir.y;
      }
      
      // make sure proper values are set to min/max
      if(minX > maxX)
        swap(minX, maxX);
      if(minY > maxY)
        swap(minY, maxY);
      
      // make sure we have a valid intersection so far
      if(minX <= maxY && minY <= maxX){
        
        // store min/max distances for x & y
        float minT = max(minX, minY);
        float maxT = min(maxX, maxY);
        
        // calculate min & max intersection values for z
        float minZ, maxZ;
        if(r.dir.z == 0.0){
          minZ = -FLOAT_MAX;
          maxZ = FLOAT_MAX;
        }else{
          minZ = (minP.z - r.pos.z) / r.dir.z;
          maxZ = (maxP.z - r.pos.z) / r.dir.z;
        }
        
        // make sure proper values are set to min/max
        if(minZ > maxZ)
          swap(minZ, maxZ);
        
        // make sure we still have a valid intersection
        if(minT <= maxZ && minZ <= maxT){
          
          // store min/max distances (now with z)
          minT = max(minT, minZ);
          maxT = min(maxT, maxZ);
          
          // ray intersection if and only if ray enters before leaving
          if(minT <= maxT){
            
            // make sure all hits are along positive ray direction
            // and no hits can occur closer than previous hits
            if(minT > 0.0 && minT < t)
              return true;
          }
        }
      }
      
      // when no ray hits the bounding box
      return false;
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
  
  // texture coordinates & derivatives of texture coordinates
  Point uvw;
  Point duvw[2];
  
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
    uvw.Set(0.5, 0.5, 0.5);
    duvw[0].Zero();
    duvw[1].Zero();
    node = NULL;
    front = true;
  }
  
  // set node that object hits
  void setNode(Node *n){
    node = n;
  }
};


// Item Base definition (basic node info, stores a name)
class ItemBase{
  private:
    
    // name of the item
    string name;
  
  public:
    
    // item constructor
    ItemBase(){
      name = "";
    }
    
    // item deconstructor
    virtual ~ItemBase(){}
    
    // retrive the name of the item
    string getName(){
      return name;
    }
    
    // declare a name for the item
    void setName(string newName){
      name = newName;
    }
};


// ItemList definition (for ItemFileList, and thus a list of objects)
template <class T> class ItemList: public vector <T*>{
  public:
    
    // item list destructor
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
    void append(T* item, string name){
      list.push_back(new FileInfo(item, name));
    }
    
    // search entire list for a specific object
    T* find(string name){
      int n = list.size();
      for(int i = 0; i < n; i++)
        if(list[i] && name == list[i]->getName())
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
        FileInfo(T *i, string name){
          item = i;
          setName(name);
        }
        
        // destructor
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
    
    // intersect ray function for each object (along with ray cones)
    virtual bool intersectRay(Cone &r, HitInfo &h, int face = HIT_FRONT) = 0;
    
    // bounding box function for each object
    virtual BoundingBox getBoundBox() = 0;
    
    // bias used in ray intersection hit detection
    float getBias(){
      return 0.001;
    }
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
    // uses the incoming ray or cone, hit info of rendering pixel, and all lights
    // also keeps an integer count of how many reflection bounces remaining
    virtual Color shade(Cone &r, HitInfo &h, LightList &lights, int bounceCount = 1) = 0;
};


// MaterialList definition (stores all materials, searchable)
class MaterialList: public ItemList<Material> {
  public:
    Material* find(string name){
      int n = size();
      for(int i = 0; i < n; i++)
        if(at(i) && name == at(i)->getName())
          return at(i);
      return NULL;
    }
};


// Texture definition (how to store textures)
class Texture: public ItemBase{
  public:
    
    // evaluate the color at some given uvw location
    virtual Color sample(Point &uvw) = 0;
    
    // evaluate the color using derivatives recursively
    Color sample(Point &uvw, Point duvw[2], bool elliptic = true){
      
      // begin sampling the texture
      Color c = sample(uvw);
      
      // stop re-sampling the texture when good enough
      if(duvw[0].LengthSquared() + duvw[1].LengthSquared() == 0)
        return c;
      
      // continue the re-sampling the texture recursively
      for(int i = 0; i < TEXTURE_SAMPLE_COUNT; i++){
        
        // set variables
        float x = 0;
        float y = 0;
        float fx = 0.5;
        float fy = 1.0 / 3.0;
        
        // Halton sequence, base 2 & 3
        for(int ix = i; ix > 0; ix /= 2){
          x += fx * (ix % 2);
          fx /= 2;
        }
        for(int iy = i; iy > 0; iy /= 3){
          y += fy * (iy % 3);
          fy /= 3;
        }
        
        // elliptic texture sampling (cone)
        if(elliptic){
          float r = sqrtf(x) * 0.5;
          x = r * sinf(y * (float) M_PI * 2.0);
          y = r * cosf(y * (float) M_PI * 2.0);
        
        // for non-elliptic sampling (no cones)
        }else{
          if(x > 0.5)
            x -= 1.0;
          if(y > 0.5)
            y -= 1.0;
        }
        
        // continue re-sampling the texture
        Point p = uvw + x * duvw[0] + y * duvw[1];
        c += sample(p);
      }
      
      // return the sampled texture color
      return c / float(TEXTURE_SAMPLE_COUNT);
    }
    
  protected:
    
    // clamps the uvw points for textures that tile (between 0 & 1)
    Point tileClamp(Point &uvw){
      Point u;
      u.x = uvw.x - (int) uvw.x;
      u.y = uvw.y - (int) uvw.y;
      u.z = uvw.z - (int) uvw.z;
      if(u.x < 0.0)
        u.x += 1.0;
      if(u.y < 0.0)
        u.y += 1.0;
      if(u.z < 0.0)
        u.z += 1.0;
      return u;
    }
};


// ItemFileList for textures definition (stores all textures)
typedef ItemFileList<Texture> TextureList;


// TextureMap definition (handles textures that need transformations)
// uvw sampled values are always first transformed into texture space
class TextureMap: public Transformation{
  private:
    
    // variables
    Texture *texture;
  
  public:
    
    // constructors
    TextureMap(){
      texture = NULL;
    }
    TextureMap(Texture *t){
      texture = t;
    }
    
    // set texture
    void setTexture(Texture *t){
      texture = t;
    }
    
    // sample texture (with and without derivatives)
    virtual Color sample(Point &uvw){
      Color c;
      if(texture){
        Point p = transformTo(uvw);
        Color q = texture->sample(p);
        c.Set(q.r, q.g, q.b);
      }else
        c.Set(0.0, 0.0, 0.0);
      return c;
    }
    virtual Color sample(Point &uvw, Point duvw[2], bool elliptic = true){
      if(texture == NULL)
        return Color(0.0, 0.0, 0.0);
      Point u = transformTo(uvw);
      Point d[2];
      d[0] = transformTo(duvw[0] + uvw) - u;
      d[1] = transformTo(duvw[1] + uvw) - u;
      return texture->sample(u, d, elliptic);
    }
};


// TexturedColor definition (stores a texturemap & color
// will multiply texture with color if needed, too
class TexturedColor{
  private:
    
    // variables
    Color *color;
    TextureMap *map;
  
  public:
    
    // constructors
    TexturedColor(){
      color->Set(0.0, 0.0, 0.0);
      map = NULL;
    }
    TexturedColor(float r, float g, float b){
      color->Set(r, g, b);
      map = NULL;
    }
    
    // set color
    void setColor(Color &c){
      color->Set(c.r, c.g, c.b);
    }
    
    // set texture map
    void setTexture(TextureMap *m){
      map = m;
    }
    
    // get the current color
    Color getColor(){
      return *color;
    }
    
    // get the current texture map
    TextureMap* getTexture(){
      return map;
    }
    
    // sample texture (with and without derivatives)
    Color sample(Point &uvw){
      return map ? *color * map->sample(uvw): *color;
    }
    Color sample(Point &uvw, Point duvw[2], bool elliptic = true){
      return map ? *color * map->sample(uvw, duvw, elliptic): *color;
    }
    
    // return the appropriate color of the texture for environment mapping
    Color sampleEnvironment(Point &dir){
      float z = asinf(-dir.z) / float(M_PI) + 0.5;
      float x = dir.x / (fabs(dir.x) + fabs(dir.y));
      float y = dir.y / (fabs(dir.x) + fabs(dir.y));
      Point p = Point(0.5, 0.5, 0.0) + z * (x * Point(0.5, 0.5, 0.0) + y * Point(-0.5, 0.5, 0.0));
      return sample(p);
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
    
    // bounding box for all child nodes
    // does not include this node's object!
    BoundingBox childBoundBox;
  
  public:
    
    // empty constructor
    Node(){
      child = NULL;
      numChild = 0;
      obj = NULL;
      matl = NULL;
    }
    
    // deconstructor
    ~Node(){
      deleteAllChildNodes();
    }
    
    // initialize the node, by deleting all its children
    void init(){
      deleteAllChildNodes();
      obj = NULL;
      matl = NULL;
      childBoundBox.init();
      setName("");
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
    
    // bounding box computation (for all children)
    BoundingBox& computeChildBoundBox(){
      childBoundBox.init();
      
      // grab all child bounding boxes!
      for(int i = 0; i < numChild; i++){
        BoundingBox childBox = child[i]->computeChildBoundBox();
        Object *childObj = child[i]->getObject();
        
        // add child object's bounding box
        if(childObj)
          childBox += childObj->getBoundBox();
        
        // transform the bounding box space
        if(!childBox.isEmpty())
          for(int j = 0; j < 8; j++)
            childBoundBox += child[i]->transformFrom(childBox.corner(j));
      }
      
      // return the computed child node bounding box
      return childBoundBox;
    }
    
    // get the child bounding box
    BoundingBox& getChildBoundBox(){
      return childBoundBox;
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
    Cone toModelSpace(Cone &ray){
      Cone r;
      r.pos = transformTo(ray.pos);
      r.dir = transformTo(ray.pos + ray.dir) - r.pos;
      r.tan = ray.tan;
      r.radius = ray.radius;
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
  string materialName;
};
vector<NodeMaterial> nodeMaterialList;


// abstract root node as the scene object
Node *scene;
void setScene(Node &n){
  scene = &n;
}


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
      int offset = 14;
      int contrast = -23;
      int mx = 255 + contrast - offset;
      
      // assign pixel values based on min & max z-values
      for(int i = 0; i < size; i++){
        
        // background color
        if(z[i] == FLOAT_MAX)
          zbuffer[i] = 0;
        
        // for pixels with objects, map from white (close) to dark (far)
        else{
          float f = (maxZ - z[i]) / (maxZ - minZ);
          int c = int(f * mx);
          if(c < 0)
            f = 0;
          if(c > mx)
            f = 2;
          zbuffer[i] = c + offset;
        }
      }
    }
    
    // save the rendered image to a file
    bool save(string file){
      return outputImage(file, 3);
    }
    
    // save the rendered z-buffer image to a file
    bool saveZBuffer(string file){
      return outputImage(file, 1);
    }
  
  private:
    
    // write out an image file
    bool outputImage(string file, int components){
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


// determines if geometry term is used in the specular reflection
bool specularGeometry = false;
void setSpecularGeometry(bool b){
  specularGeometry = b;
}


// recursively go through node & descendants, find the closest ray hit info
bool traceRayToNode(Cone r, HitInfo &h, Node &n){
  
  // if object gets hit and hit first
  bool objectHit;
  
  // grab node's object
  Object *obj = n.getObject();
  
  // transform ray into model space (or local space)
  Cone ray = n.toModelSpace(r);
  
  // make hit info for node object (if exists)
  HitInfo hit = HitInfo();
  if(obj){
    hit.setNode(&n);
    
    // check the object's bounding box, should we bother sending a ray?
    if(obj->getBoundBox().intersectRay(ray, h.z)){
      
      // check if object is hit
      objectHit = obj->intersectRay(ray, hit);
    }
  }
  
  // check if hit was closer than previous hits
  if(objectHit){
    if(hit.z < h.z)
      h = hit;
    
    // if hit is not closer, don't count as hit
    else
      objectHit = false; 
  }
  
  // check the child bounding boxes, should we bother sending a ray?
  if(n.getChildBoundBox().intersectRay(ray, h.z)){
    
    // loop on child nodes
    int j = 0;
    int numChild = n.getNumChild();
    while(j < numChild){
      
      // grab child node
      Node *child = n.getChild(j);
      
      
      // recursively check this child's descendants for hit info
      bool childHit = traceRayToNode(ray, h, *child);
      
      // if child is hit, make sure we pass that on
      if(childHit)
        objectHit = true;
      
      // loop through all children
      j++;
    }
  }
  
  // if object (or a descendant) was hit, transform from model space (to world space)
  if(objectHit)
    n.fromModelSpace(h);
  
  // return whether there was a hit on object or its descendants
  return objectHit;
}

// main ray tracing function, recursively traverses scene for ray hits
bool traceRay(Cone r, HitInfo &h){
  return traceRayToNode(r, h, *scene);
}


}
#endif
