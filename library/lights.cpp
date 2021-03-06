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
    Color illuminate(Point p, Point n){
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


// indirect light definition
class IndirectLight: public GenericLight{
  public:
    
    // constructor
    IndirectLight(){}
    
    // get color of indirect light by tracing a new ray
    Color illuminate(Point p, Point n){
      
      // color for shading
      Color indirect;
      
      // multi-sampling for indirect lighting
      for(int s = 0; s < samples; s++){
        
        // randomize ray on a hemisphere
        float phi = dist(rnd) * 2.0 * M_PI;
        float the = acos(1.0 - 2.0 * dist(rnd)) / 2.0;
        
        // calculate hemisphere vectors
        Point v0 = Point(0.0, 1.0, 0.0);
        if(v0 % n > 0.5 || v0 % n < -0.5)
          v0 = Point(0.0, 0.0, 1.0);
        Point v1 = (v0 ^ n).GetNormalized();
        v0 = (v1 ^ n).GetNormalized();
        
        // set up ray and hit info
        Cone *r = new Cone();
        r->pos = p;
        r->dir = n.GetNormalized() * cos(the) + (v0 * cos(phi) + v1 * sin(  phi)) * sin(the);
        HitInfo hi = HitInfo();
        
        // trace a new ray
        bool hit = traceRay(*r, hi);
        
        // grab the node material hit
        Material *m;
        if(hit){
          Node *n = hi.node;
          if(n)
            m = n->getMaterial();
        }
        
        // shade our material
        if(hit && m)
          indirect = (indirect * s + m->shade(*r, hi, lights)) / (float) (s + 1);
        
        // otherwise, nothing to shade
        else
          indirect = (indirect * s + environment.sampleEnvironment(r->dir)) / (float) (s + 1);
      }
      
      // return the color
      return indirect;
    }
    
    // get direction of indirect light (non-sensical)
    Point direction(Point p){
      return Point(0, 0, 0);
    }
    
    // return true, since light is indirect
    bool isAmbient(){
      return true;
    }
    
    // set the light list
    void setLightList(LightList *l){
      lights = *l;
    }
    
    // set the environment
    void setEnvironment(TexturedColor c){
      environment = c;
    }
    
    // set the number of samples
    void setSamples(int s){
      samples = s;
    }
    
  private:
    
    // light list for all other lights
    LightList lights;
    
    // environment variable
    TexturedColor environment;
    
    // number of samples for global illumination
    int samples;
    
    // setup random generator for global illumination
    mt19937 rnd;
    uniform_real_distribution<float> dist{0.0, 1.0};
};


// irradiance cache light definition
class IrradianceCacheLight: public GenericLight{
  public:
    
    // constructor
    IrradianceCacheLight(){}
    
    // get color of indirect light by tracing a new ray
    Color illuminate(Point p, Point n){
      
      // color for shading
      Color indirect;
      
      // multi-sampling for indirect lighting
      for(int s = 0; s < samples; s++){
        
        // randomize ray on a hemisphere
        float phi = Halton(s, 3) * 2.0 * M_PI;
        float the = acos(1.0 - 2.0 * Halton(s, 2)) / 2.0;
        
        // calculate hemisphere vectors
        Point v0 = Point(0.0, 1.0, 0.0);
        if(v0 % n > 0.5 || v0 % n < -0.5)
          v0 = Point(0.0, 0.0, 1.0);
        Point v1 = (v0 ^ n).GetNormalized();
        v0 = (v1 ^ n).GetNormalized();
        
        // set up ray and hit info
        Cone *r = new Cone();
        r->pos = p;
        r->dir = n.GetNormalized() * cos(the) + (v0 * cos(phi) + v1 * sin(  phi)) * sin(the);
        HitInfo hi = HitInfo();
        
        // trace a new ray
        bool hit = traceRay(*r, hi);
        
        // grab the node material hit
        Material *m;
        if(hit){
          Node *n = hi.node;
          if(n)
            m = n->getMaterial();
        }
        
        // shade our material
        if(hit && m)
          indirect = (indirect * s + m->shade(*r, hi, lights)) / (float) (s + 1);
        
        // otherwise, nothing to shade
        else
          indirect = (indirect * s + environment.sampleEnvironment(r->dir)) / (float) (s + 1);
      }
      
      // return the color
      return indirect;
    }
    
    // get direction of indirect light (non-sensical)
    Point direction(Point p){
      return Point(0, 0, 0);
    }
    
    // return true, since light is indirect
    bool isAmbient(){
      return true;
    }
    
    // set the light list
    void setLightList(LightList *l){
      lights = *l;
    }
    
    // set the environment
    void setEnvironment(TexturedColor c){
      environment = c;
    }
    
    // set the number of samples
    void setSamples(int s){
      samples = s;
    }
    
  private:
    
    // light list for all other lights
    LightList lights;
    
    // environment variable
    TexturedColor environment;
    
    // number of samples for global illumination
    int samples;
};


// irradiance map light definition (another type of indirect light)
class IrradianceMapLight: public GenericLight{
  public:
    
    // constructor
    IrradianceMapLight(){
      indirect = Color(0.0);
    }
    
    // get color of indirect light from irradiance map
    Color illuminate(Point p, Point n){
      
      // return the color
      return indirect;
    }
    
    // get direction of indirect light (non-sensical)
    Point direction(Point p){
      return Point(0, 0, 0);
    }
    
    // return true, since light is indirect
    bool isAmbient(){
      return true;
    }
    
    // set the color of the indirect light (done for each pixel of the irradiance map)
    void setColor(Color c){
      indirect = c;
    }
    
  private:
    
    // indirect color (from irradiance map)
    Color indirect;
};


// photon map light definition
class PhotonMapLight: public GenericLight{
  public:
    
    // constructor
    PhotonMapLight(){}
    
    // get color from photon map
    Color illuminate(Point p, Point n){
      
      // grab color from photon map
      float *irrad = new float[3];
      float *position = new float[3];
      p.GetValue(position);
      float *normal = new float[3];
      n.GetValue(normal);
      irradianceEstimate(pm, irrad, position, normal, photonRad, maxPhotons);
      
      // return color
      return Color(irrad);
    }
    
    // get direction of photon map light (non-sensical)
    Point direction(Point p){
      return Point(0, 0, 0);
    }
    
    // return true, since light is indirect
    bool isAmbient(){
      return true;
    }
    
    // set photon map
    void setPhotonMap(BalancedPhotonMap *map, float f, int i){
      pm = map;
      photonRad = f;
      maxPhotons = i;
    }
    
  private:
    
    // photon map
    BalancedPhotonMap *pm;
    float photonRad = 1.0;
    int maxPhotons = 10.0;
};


// Monte Carlo photon map light definition
class MonteCarloPhotonMapLight: public GenericLight{
  public:
    
    // constructor
    MonteCarloPhotonMapLight(){}
    
    // get color of indirect light by tracing a new ray
    Color illuminate(Point p, Point n){
      
      // color for shading
      Color indirect;
      
      // multi-sampling for indirect lighting
      for(int s = 0; s < samples; s++){
        
        // randomize ray on a hemisphere
        float phi = dist(rnd) * 2.0 * M_PI;
        float the = acos(1.0 - 2.0 * dist(rnd)) / 2.0;
        
        // calculate hemisphere vectors
        Point v0 = Point(0.0, 1.0, 0.0);
        if(v0 % n > 0.5 || v0 % n < -0.5)
          v0 = Point(0.0, 0.0, 1.0);
        Point v1 = (v0 ^ n).GetNormalized();
        v0 = (v1 ^ n).GetNormalized();
        
        // set up ray and hit info
        Cone *r = new Cone();
        r->pos = p;
        r->dir = n.GetNormalized() * cos(the) + (v0 * cos(phi) + v1 * sin(  phi)) * sin(the);
        HitInfo hi = HitInfo();
        
        // trace a new ray
        bool hit = traceRay(*r, hi);
        
        // grab the node material hit
        Material *m;
        if(hit){
          Node *n = hi.node;
          if(n)
            m = n->getMaterial();
        }
        
        // shade our material
        if(hit && m){
          
          // grab color from photon map
          float *irrad = new float[3];
          float *position = new float[3];
          hi.p.GetValue(position);
          float *normal = new float[3];
          hi.n.GetValue(normal);
          irradianceEstimate(pm, irrad, position, normal, photonRad, maxPhotons);
          
          // update current color
          indirect = (indirect * s + Color(irrad)) / (float) (s + 1);
        
        // otherwise, nothing to shade
        }else
          indirect = (indirect * s + environment.sampleEnvironment(r->dir)) / (float) (s + 1);
      }
      
      // return the color
      return indirect;
    }
    
    // get direction of indirect light (non-sensical)
    Point direction(Point p){
      return Point(0, 0, 0);
    }
    
    // return true, since light is indirect
    bool isAmbient(){
      return true;
    }
    
    // set photon map
    void setPhotonMap(BalancedPhotonMap *map, float f, int i){
      pm = map;
      photonRad = f;
      maxPhotons = i;
    }
    
    // set the environment
    void setEnvironment(TexturedColor c){
      environment = c;
    }
    
    // set the number of samples
    void setSamples(int s){
      samples = s;
    }
    
  private:
    
    // photon map
    BalancedPhotonMap *pm;
    float photonRad = 1.0;
    int maxPhotons = 10.0;
    
    // environment variable
    TexturedColor environment;
    
    // number of samples for global illumination
    int samples;
    
    // setup random generator for global illumination
    mt19937 rnd;
    uniform_real_distribution<float> dist{0.0, 1.0};
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
    Color illuminate(Point p, Point n){
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
    Color illuminate(Point p, Point n){
      
      // calculate the inverse square fall-off
      float scale = 1.0;
      if(invSqFO)
        scale /= (position - p).Length() * (position - p).Length();
      
      // for a point light
      if(size == 0.0){
        Cone r = Cone();
        r.pos = p;
        r.dir = position - p;
        return shadow(r, 1.0) * scale * intensity;
      
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
        return mean * scale * intensity;
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
    
    // set shadow ray samples (min & max)
    void setShadowRays(int min, int max){
      shadowMin = min;
      shadowMax = max;
    }
    
    // inverse square fall off boolean
    void inverseSquareFalloff(){
      invSqFO = true;
    }
    
    // extensions for photon mapping
    
    // check if the light source emits photons (point light only!)
    bool isPhotonSource(){
      return true;
    }
    
    // get the intensity of the color of the photon
    Color getPhotonIntensity(){
      return intensity;
    }
    
    // calculate a random photon from our point light source
    Cone randomPhoton(){
      
      // location of point light
      Point p = position;
      
      // randomize our point within light (spherical light)
      if(size > 0){
        
        // rejection sampling
        Point o = Point(size, size, size);
        while(o.LengthSquared() > size * size){
          o.x = dist2(rnd) * size;
          o.y = dist2(rnd) * size;
          o.z = dist2(rnd) * size;
        }
        
        // set position
        p += o;
      }
      
      // rejection sampling for random light direction
      Point d = Point(1.0, 1.0, 1.0);
      while(d.LengthSquared() > 1.0){
        d.x = dist2(rnd);
        d.y = dist2(rnd);
        d.z = dist2(rnd);
      }
      d.GetNormalized();
      
      // return our random photon
      return Cone(p, d);
    }
    
  private:
    
    // intensity (or color) of light
    Color intensity;
    
    // location of light
    Point position;
    
    // size of point light (sphere if not zero)
    float size;
    
    // how many shadow rays to cast
    int shadowMin;
    int shadowMax;
    
    // random number generation for light disk rotation
    mt19937 rnd;
    uniform_real_distribution<float> dist{0.0, 1.0};
    uniform_real_distribution<float> dist2{-1.0, 1.0};
    
    bool invSqFO = false;
    
    // calculate a randomized light position on a spherical light
    Cone getShadowRay(Point p, int c, float r){
      
      // get original direction
      Point dir = (position - p).GetNormalized();
      
      // get two vectors for spanning our light disk
      Point v0 = Point(0.0, 1.0, 0.0);
      if(v0 % dir > 0.5 || v0 % dir < -0.5)
        v0 = Point(0.0, 0.0, 1.0);
      Point v1 = (v0 ^ dir).GetNormalized();
      v0 = (v1 ^ dir).GetNormalized();
      
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
